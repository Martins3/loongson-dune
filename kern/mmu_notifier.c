#include <linux/mmu_notifier.h>
#include "vz.h"
#include <linux/gfp.h>
#include <linux/nospec.h>

// TODO maybe change a better name, or just vz ?
static inline struct vz_vcpu *kvm_get_vcpu(struct vz_vm *kvm, int i)
{
	int num_vcpus = atomic_read(&kvm->online_vcpus);
	i = array_index_nospec(i, num_vcpus);

	/* Pairs with smp_wmb() in kvm_vm_ioctl_create_vcpu.  */
	smp_rmb();
	return kvm->vcpus[i];
}

#define kvm_for_each_vcpu(idx, vcpup, kvm)                                     \
	for (idx = 0; idx < atomic_read(&kvm->online_vcpus) &&                 \
		      (vcpup = kvm_get_vcpu(kvm, idx)) != NULL;                \
	     idx++)

static inline void kvm_make_request(int req, struct vz_vcpu *vcpu)
{
	/*
	 * Ensure the rest of the request is published to kvm_check_request's
	 * caller.  Paired with the smp_mb__after_atomic in kvm_check_request.
	 */
	smp_wmb();
	set_bit(req & KVM_REQUEST_MASK, (void *)&vcpu->requests);
}

static inline int kvm_vcpu_exiting_guest_mode(struct vz_vcpu *vcpu)
{
	/*
	 * The memory barrier ensures a previous write to vcpu->requests cannot
	 * be reordered with the read of vcpu->mode.  It pairs with the general
	 * memory barrier following the write of vcpu->mode in VCPU RUN.
	 */
	smp_mb__before_atomic();
	return cmpxchg(&vcpu->mode, IN_GUEST_MODE, EXITING_GUEST_MODE);
}

static bool kvm_request_needs_ipi(struct vz_vcpu *vcpu, unsigned req)
{
	int mode = kvm_vcpu_exiting_guest_mode(vcpu);

	/*
	 * We need to wait for the VCPU to reenable interrupts and get out of
	 * READING_SHADOW_PAGE_TABLES mode.
	 */
	if (req & KVM_REQUEST_WAIT)
		return mode != OUTSIDE_GUEST_MODE;

	/*
	 * Need to kick a running VCPU, but otherwise there is nothing to do.
	 */
	return mode == IN_GUEST_MODE;
}

static void ack_flush(void *_completed)
{
}

static inline bool kvm_kick_many_cpus(const struct cpumask *cpus, bool wait)
{
	if (unlikely(!cpus))
		cpus = cpu_online_mask;

	if (cpumask_empty(cpus))
		return false;

	smp_call_function_many(cpus, ack_flush, NULL, wait);
	return true;
}

bool kvm_make_vcpus_request_mask(struct vz_vm *kvm, unsigned int req,
				 unsigned long *vcpu_bitmap, cpumask_var_t tmp)
{
	int i, cpu, me;
	struct vz_vcpu *vcpu;
	bool called;

	me = get_cpu();

	kvm_for_each_vcpu (i, vcpu, kvm) {
		if (!test_bit(i, vcpu_bitmap))
			continue;

		kvm_make_request(req, vcpu);
		cpu = vcpu->cpu;

		BUG_ON((req &= KVM_REQUEST_NO_WAKEUP) == 0);

		// if (!(req & KVM_REQUEST_NO_WAKEUP) && kvm_vcpu_wake_up(vcpu))
		// continue;

		if (tmp != NULL && cpu != -1 && cpu != me &&
		    kvm_request_needs_ipi(vcpu, req))
			__cpumask_set_cpu(cpu, tmp);
	}

	called = kvm_kick_many_cpus(tmp, !!(req & KVM_REQUEST_WAIT));
	put_cpu();

	return called;
}

bool kvm_make_all_cpus_request(struct vz_vm *kvm, unsigned int req)
{
	cpumask_var_t cpus;
	bool called;
	static unsigned long vcpu_bitmap[BITS_TO_LONGS(KVM_MAX_VCPUS)] = {
		[0 ... BITS_TO_LONGS(KVM_MAX_VCPUS) - 1] = ULONG_MAX
	};

	zalloc_cpumask_var(&cpus, GFP_ATOMIC);

	called = kvm_make_vcpus_request_mask(kvm, req, vcpu_bitmap, cpus);

	free_cpumask_var(cpus);
	return called;
}

void kvm_flush_remote_tlbs(struct vz_vm *kvm)
{
	kvm_make_all_cpus_request(kvm, KVM_REQ_TLB_FLUSH);
}

static inline struct vz_vm *mmu_notifier_to_vz_vm(struct mmu_notifier *mn)
{
	return container_of(mn, struct vz_vm, mmu_notifier);
}

static void kvm_mmu_notifier_change_pte(struct mmu_notifier *mn,
					struct mm_struct *mm,
					unsigned long address, pte_t pte)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int idx;

	idx = srcu_read_lock(&kvm->srcu);
	spin_lock(&kvm->mmu_lock);
	kvm->mmu_notifier_seq++;
	kvm_set_spte_hva(kvm, address, pte);
	spin_unlock(&kvm->mmu_lock);
	srcu_read_unlock(&kvm->srcu, idx);
}

static int kvm_mmu_notifier_invalidate_range_start(struct mmu_notifier *mn,
						   struct mm_struct *mm,
						   unsigned long start,
						   unsigned long end,
						   bool blockable)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int idx;

	idx = srcu_read_lock(&kvm->srcu);
	spin_lock(&kvm->mmu_lock);
	/*
	 * The count increase must become visible at unlock time as no
	 * spte can be established without taking the mmu_lock and
	 * count is also read inside the mmu_lock critical section.
	 */
	kvm->mmu_notifier_count++;
	kvm_unmap_hva_range(kvm, start, end);

	spin_unlock(&kvm->mmu_lock);

	srcu_read_unlock(&kvm->srcu, idx);

	return 0;
}

static void kvm_mmu_notifier_invalidate_range_end(struct mmu_notifier *mn,
						  struct mm_struct *mm,
						  unsigned long start,
						  unsigned long end)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);

	spin_lock(&kvm->mmu_lock);
	/*
	 * This sequence increase will notify the kvm page fault that
	 * the page that is going to be mapped in the spte could have
	 * been freed.
	 */
	kvm->mmu_notifier_seq++;
	smp_wmb();
	/*
	 * The above sequence increase must be visible before the
	 * below count decrease, which is ensured by the smp_wmb above
	 * in conjunction with the smp_rmb in mmu_notifier_retry().
	 */
	kvm->mmu_notifier_count--;
	spin_unlock(&kvm->mmu_lock);

	BUG_ON(kvm->mmu_notifier_count < 0);
}

static int kvm_mmu_notifier_clear_flush_young(struct mmu_notifier *mn,
					      struct mm_struct *mm,
					      unsigned long start,
					      unsigned long end)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int young, idx;

	idx = srcu_read_lock(&kvm->srcu);
	spin_lock(&kvm->mmu_lock);

	young = kvm_age_hva(kvm, start, end);
	if (young)
		kvm_flush_remote_tlbs(kvm);

	spin_unlock(&kvm->mmu_lock);
	srcu_read_unlock(&kvm->srcu, idx);

	return young;
}

static int kvm_mmu_notifier_clear_young(struct mmu_notifier *mn,
					struct mm_struct *mm,
					unsigned long start, unsigned long end)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int young, idx;

	idx = srcu_read_lock(&kvm->srcu);
	spin_lock(&kvm->mmu_lock);
	/*
	 * Even though we do not flush TLB, this will still adversely
	 * affect performance on pre-Haswell Intel EPT, where there is
	 * no EPT Access Bit to clear so that we have to tear down EPT
	 * tables instead. If we find this unacceptable, we can always
	 * add a parameter to kvm_age_hva so that it effectively doesn't
	 * do anything on clear_young.
	 *
	 * Also note that currently we never issue secondary TLB flushes
	 * from clear_young, leaving this job up to the regular system
	 * cadence. If we find this inaccurate, we might come up with a
	 * more sophisticated heuristic later.
	 */
	young = kvm_age_hva(kvm, start, end);
	spin_unlock(&kvm->mmu_lock);
	srcu_read_unlock(&kvm->srcu, idx);

	return young;
}

static int kvm_mmu_notifier_test_young(struct mmu_notifier *mn,
				       struct mm_struct *mm,
				       unsigned long address)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int young, idx;

	idx = srcu_read_lock(&kvm->srcu);
	spin_lock(&kvm->mmu_lock);
	young = kvm_test_age_hva(kvm, address);
	spin_unlock(&kvm->mmu_lock);
	srcu_read_unlock(&kvm->srcu, idx);

	return young;
}

static void kvm_mmu_notifier_release(struct mmu_notifier *mn,
				     struct mm_struct *mm)
{
	struct vz_vm *kvm = mmu_notifier_to_vz_vm(mn);
	int idx;

	idx = srcu_read_lock(&kvm->srcu);
	srcu_read_unlock(&kvm->srcu, idx);
}

static const struct mmu_notifier_ops kvm_mmu_notifier_ops = {
	.flags = MMU_INVALIDATE_DOES_NOT_BLOCK,
	.invalidate_range_start = kvm_mmu_notifier_invalidate_range_start,
	.invalidate_range_end = kvm_mmu_notifier_invalidate_range_end,
	.clear_flush_young = kvm_mmu_notifier_clear_flush_young,
	.clear_young = kvm_mmu_notifier_clear_young,
	.test_young = kvm_mmu_notifier_test_young,
	.change_pte = kvm_mmu_notifier_change_pte,
	.release = kvm_mmu_notifier_release,
};

// TODO init with kvm_pgd_alloc ?
static int kvm_init_mmu_notifier(struct vz_vm *kvm)
{
  kvm->mmu_notifier.ops = &kvm_mmu_notifier_ops;
  return mmu_notifier_register(&kvm->mmu_notifier, current->mm);
}

