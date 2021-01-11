#ifndef _H_VZ
#define _H_VZ
#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/kvm_types.h> // TODO remove anything about kvm ?
#include <asm/syscall.h>
#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/mmu_notifier.h>
#include <linux/nospec.h>

#include "dune.h"

extern __init int vz_init(void);
extern void vz_exit(void);
extern int vz_launch(struct dune_config *conf, int64_t *ret_code);

// TODO firstly, I don't why KMV limite vcpu to 16, secondly, one vcpu for a thread, if thread beyond 16 ?
#define KVM_MAX_VCPUS 16

#define N_MIPS_COPROC_REGS 32
#define N_MIPS_COPROC_SEL 8
struct mips_coproc {
	unsigned long reg[N_MIPS_COPROC_REGS][N_MIPS_COPROC_SEL];
};

#define KVM_NR_MEM_OBJS 4
/*
 * We don't want allocation failures within the mmu code, so we preallocate
 * enough memory for a single page fault in a cache.
 */
struct kvm_mmu_memory_cache {
	int nobjs;
	void *objects[KVM_NR_MEM_OBJS];
};

struct dune_mips_tlb {
	long tlb_mask;
	long tlb_hi;
	long tlb_lo[2];
};
// TODO why guest TLB size is arch unrelated ?
#define KVM_MIPS_GUEST_TLB_SIZE 64

// TODO do we need the name of all the GPRs, why x86 dune need this ?
enum vz_reg {
	// TODO
	NR_VCPU_REGS
};

struct kvm_arch_memory_slot {
};


#define KVM_USER_MEM_SLOTS	16
/* memory slots that does not exposed to userspace */
#define KVM_PRIVATE_MEM_SLOTS	0

#define KVM_MEM_SLOTS_NUM (KVM_USER_MEM_SLOTS + KVM_PRIVATE_MEM_SLOTS)

struct kvm_memory_slot {
	gfn_t base_gfn;
	unsigned long npages;
	unsigned long *dirty_bitmap;
	struct kvm_arch_memory_slot arch;
	unsigned long userspace_addr;
	u32 flags;
	short id;
};

/*
 * Note:
 * memslots are not sorted by id anymore, please use id_to_memslot()
 * to get the memslot by its id.
 */
struct kvm_memslots {
	u64 generation;
	struct kvm_memory_slot memslots[KVM_MEM_SLOTS_NUM];
	/* The mapping table from slot id to the index in memslots[]. */
	short id_to_index[KVM_MEM_SLOTS_NUM];
	atomic_t lru_slot;
	int used_slots;
};

// copied from struct kvm_arch and kvm
// TODO shared by all vcpus which belong to one kvm
struct vz_vm {
	// TODO how to initlize the vz_vm ?
	spinlock_t mmu_lock;
	struct mmu_notifier mmu_notifier;
	unsigned long mmu_notifier_seq;
	long mmu_notifier_count;
	struct srcu_struct srcu;
	long tlbs_dirty;

	struct vz_vcpu *vcpus[KVM_MAX_VCPUS];
	// TODO please trace how online_vcpus setup
	/*
	 * created_vcpus is protected by kvm->lock, and is incremented
	 * at the beginning of KVM_CREATE_VCPU.  online_vcpus is only
	 * incremented after storing the kvm_vcpu pointer in vcpus,
	 * and is accessed atomically.
	 */
	atomic_t online_vcpus;

	// TODO
	// when should we assign it
	// process running in dune's mm
	//
	// I don't know should protect it or not ?
	struct mm_struct *current_mm;

#define KVM_ADDRESS_SPACE_NUM 1
	struct kvm_memslots __rcu *memslots[KVM_ADDRESS_SPACE_NUM];

	/* Guest physical mm */
	struct mm_struct gpa_mm;
};

struct vz_vcpu {
	struct list_head list;
	int cpu;
	int launched; // TODO launched and conf != NULL

	int shutdown;
	// TODO seems two ret_code ?
	int ret_code;

	// TODO
	// sys_call_ptr_t *syscall_tbl;
	struct dune_config *conf;

	void *guest_ebase;

	// TODO this is what to do next !
	int (*vcpu_run)(struct vz_vcpu *vcpu);

	/* Host KSEG0 address of the EI/DI offset */
	void *kseg0_commpage;

	/* COP0 State */
	struct mips_coproc *cop0;

	/* GPRS */
	unsigned long gprs[NR_VCPU_REGS];
	unsigned long hi;
	unsigned long lo;
	// TODO pc
	unsigned long pc;

	/* S/W Based TLB for guest */
	struct dune_mips_tlb guest_tlb[KVM_MIPS_GUEST_TLB_SIZE];

	/* FPU State */
	// TODO what's fcr31 ?
	struct mips_fpu_struct fpu;

	/* vcpu's vzguestid is different on each host cpu in an smp system */
	u64 vzguestid[NR_CPUS];

	// TODO this is originally in kvm_run::exit_reason
	/* out */
	u32 exit_reason;

	// TODO why I need this ?
	/* Last CPU the VCPU state was loaded on */
	int last_sched_cpu;
	/* Last CPU the VCPU actually executed guest code on */
	int last_exec_cpu;

	/* wired guest TLB entries */
	struct dune_mips_tlb *wired_tlb;
	unsigned int wired_tlb_limit;
	unsigned int wired_tlb_used;

	struct vz_vm * kvm;

	/* Host CP0 registers used when handling exits from guest */
	unsigned long host_cp0_badvaddr;
	unsigned long host_cp0_epc;
	u32 host_cp0_cause;
	u32 host_cp0_guestctl0;
	u32 host_cp0_badinstr;
	u32 host_cp0_badinstrp;
	u32 host_cp0_gscause;

	// TODO we remove it by accident, will restore it later, I mean any function related with it
	int mode;
  // TODO remote tlb flush need it
	u64 requests;

	/* Cache some mmu pages needed inside spinlock regions */
	struct kvm_mmu_memory_cache mmu_page_cache;

  // TODO in dune_arch_vcpu_load, we load it, but never assign to it
	int vcpu_id;
};

/* entry.c */
int dune_mips_entry_setup(void);
void *dune_mips_build_vcpu_run(void *addr);
void *dune_mips_build_tlb_refill_exception(void *addr, void *handler);
void *dune_mips_build_exception(void *addr, void *handler);
void *dune_mips_build_exit(void *addr);

/* mips.c */
void dump_handler(const char *symbol, void *start, void *end);

/* commpage.h */
struct kvm_mips_commpage {
	/* COP0 state is mapped into Guest kernel via commpage */
	struct mips_coproc cop0;
};

#define KVM_MIPS_COMM_EIDI_OFFSET 0x0

void dune_mips_commpage_init(struct vz_vcpu *vcpu);

/* tlb.h */

extern unsigned long GUESTID_MASK;
extern unsigned long GUESTID_FIRST_VERSION;
extern unsigned long GUESTID_VERSION_MASK;

void dune_vz_load_guesttlb(const struct dune_mips_tlb *buf, unsigned int index,
			   unsigned int count);
void dune_vz_save_guesttlb(struct dune_mips_tlb *buf, unsigned int index,
			   unsigned int count);
void dune_vz_local_flush_guesttlb_all(void);
void dune_vz_local_flush_roottlb_all_guests(void);

#define dune_err(fmt, ...)                                                     \
	pr_err("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)
#define dune_info(fmt, ...)                                                    \
	pr_info("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)
#define dune_debug(fmt, ...)                                                   \
	pr_debug("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)

// TODO only part of following macro is useful, remove useless entries
#define KVM_EXIT_UNKNOWN 0
#define KVM_EXIT_EXCEPTION 1
#define KVM_EXIT_IO 2
#define KVM_EXIT_HYPERCALL 3
#define KVM_EXIT_DEBUG 4
#define KVM_EXIT_HLT 5
#define KVM_EXIT_MMIO 6
#define KVM_EXIT_IRQ_WINDOW_OPEN 7
#define KVM_EXIT_SHUTDOWN 8
#define KVM_EXIT_FAIL_ENTRY 9
#define KVM_EXIT_INTR 10
#define KVM_EXIT_SET_TPR 11
#define KVM_EXIT_TPR_ACCESS 12
#define KVM_EXIT_S390_SIEIC 13
#define KVM_EXIT_S390_RESET 14
#define KVM_EXIT_DCR 15 /* deprecated */
#define KVM_EXIT_NMI 16
#define KVM_EXIT_INTERNAL_ERROR 17
#define KVM_EXIT_OSI 18
#define KVM_EXIT_PAPR_HCALL 19
#define KVM_EXIT_S390_UCONTROL 20
#define KVM_EXIT_WATCHDOG 21
#define KVM_EXIT_S390_TSCH 22
#define KVM_EXIT_EPR 23
#define KVM_EXIT_SYSTEM_EVENT 24
#define KVM_EXIT_S390_STSI 25
#define KVM_EXIT_IOAPIC_EOI 26
#define KVM_EXIT_HYPERV 27

// TODO only part of following macro is useful, remove useless entries
enum emulation_result {
	EMULATE_DONE, /* no further processing */
	EMULATE_DO_MMIO, /* kvm_run filled with MMIO request */
	EMULATE_FAIL, /* can't emulate this instruction */
	EMULATE_WAIT, /* WAIT instruction */
	EMULATE_PRIV_FAIL,
	EMULATE_EXCEPT, /* A guest exception has been generated */
	EMULATE_HYPERCALL, /* HYPCALL instruction */
};

// TODO simplified it
/* Resume Flags */
#define RESUME_FLAG_DR (1 << 0) /* Reload guest nonvolatile state? */
#define RESUME_FLAG_HOST (1 << 1) /* Resume host? */

#define RESUME_GUEST 0
#define RESUME_GUEST_DR RESUME_FLAG_DR
#define RESUME_HOST RESUME_FLAG_HOST

int dune_trap_vz_no_handler(struct vz_vcpu *vcpu);
int dune_arch_vcpu_dump_regs(struct vz_vcpu *vcpu);

// TODO if kvm_host.h is include, maybe conflited
enum { OUTSIDE_GUEST_MODE,
       IN_GUEST_MODE,
       EXITING_GUEST_MODE,
       READING_SHADOW_PAGE_TABLES,
};

void dune_vz_vcpu_reenter(struct vz_vcpu *vcpu);
int dune_vz_vcpu_run(struct vz_vcpu *vcpu);

int kvm_unmap_hva_range(struct vz_vm *kvm, unsigned long start,
			unsigned long end);
void kvm_set_spte_hva(struct vz_vm *kvm, unsigned long hva, pte_t pte);
int kvm_age_hva(struct vz_vm *kvm, unsigned long start, unsigned long end);
int kvm_test_age_hva(struct vz_vm *kvm, unsigned long hva);

#define KVM_REQUEST_MASK GENMASK(7, 0)
#define KVM_REQUEST_NO_WAKEUP BIT(8)
#define KVM_REQUEST_WAIT BIT(9)
/*
 * Architecture-independent vcpu->requests bit members
 * Bits 4-7 are reserved for more arch-independent bits.
 */
#define KVM_REQ_TLB_FLUSH (0 | KVM_REQUEST_WAIT | KVM_REQUEST_NO_WAKEUP)

void kvm_flush_remote_tlbs(struct vz_vm *kvm);
int kvm_vz_host_tlb_inv(struct vz_vcpu *vcpu, unsigned long va);

// TODO why I need it ?
static inline int mmu_notifier_retry(struct vz_vm *kvm, unsigned long mmu_seq)
{
	if (unlikely(kvm->mmu_notifier_count))
		return 1;
	/*
	 * Ensure the read of mmu_notifier_count happens before the read
	 * of mmu_notifier_seq.  This interacts with the smp_wmb() in
	 * mmu_notifier_invalidate_range_end to make sure that the caller
	 * either sees the old (non-zero) value of mmu_notifier_count or
	 * the new (incremented) value of mmu_notifier_seq.
	 * PowerPC Book3s HV KVM calls this under a per-page lock
	 * rather than under kvm->mmu_lock, for scalability, so
	 * can't rely on kvm->mmu_lock to keep things ordered.
	 */
	smp_rmb();
	if (kvm->mmu_notifier_seq != mmu_seq)
		return 1;
	return 0;
}

#endif
