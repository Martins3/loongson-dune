#include <linux/module.h>
#include <linux/errno.h>
#include <linux/percpu.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
#include <linux/debugfs.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/syscore_ops.h>
#include <linux/cpu.h>
#include <linux/sched/signal.h>
#include <linux/sched/mm.h>
#include <linux/sched/stat.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/anon_inodes.h>
#include <linux/profile.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/compat.h>
#include <linux/srcu.h>
#include <linux/hugetlb.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/bsearch.h>
#include <linux/srcu.h>

#include <linux/types.h>
#include <linux/hardirq.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/mmu_notifier.h>
#include <linux/preempt.h>
#include <linux/msi.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/rcupdate.h>
#include <linux/ratelimit.h>
#include <linux/err.h>
#include <linux/irqflags.h>
#include <linux/context_tracking.h>
#include <linux/irqbypass.h>
#include <linux/swait.h>
#include <linux/refcount.h>
#include <linux/nospec.h>
#include <asm/signal.h>


#include "vz.h"
#include "memslot.h"


bool kvm_is_reserved_pfn(kvm_pfn_t pfn)
{
	if (pfn_valid(pfn))
		return PageReserved(pfn_to_page(pfn));

	return true;
}


// ---------------------------------- kvm_host.c -----------------------------

static void mark_page_dirty_in_slot(struct kvm_memory_slot *memslot, gfn_t gfn);

struct kvm_memslots *kvm_vcpu_memslots(struct vz_vcpu *vcpu)
{
	int as_id = kvm_arch_vcpu_memslots_id(vcpu);

	return __kvm_memslots(vcpu->kvm, as_id);
}


// ------------------------- kvm_main.c ---------------------------------------------------------------

struct kvm_memory_slot *gfn_to_memslot(struct vz_vm *kvm, gfn_t gfn)
{
	return __gfn_to_memslot(kvm_memslots(kvm), gfn);
}
EXPORT_SYMBOL_GPL(gfn_to_memslot);

struct kvm_memory_slot *kvm_vcpu_gfn_to_memslot(struct vz_vcpu *vcpu,
						gfn_t gfn)
{
	return __gfn_to_memslot(kvm_vcpu_memslots(vcpu), gfn);
}

bool kvm_is_visible_gfn(struct vz_vm *kvm, gfn_t gfn)
{
	struct kvm_memory_slot *memslot = gfn_to_memslot(kvm, gfn);

	if (!memslot || memslot->id >= KVM_USER_MEM_SLOTS ||
	    memslot->flags & KVM_MEMSLOT_INVALID)
		return false;

	return true;
}
EXPORT_SYMBOL_GPL(kvm_is_visible_gfn);

unsigned long kvm_host_page_size(struct vz_vm *kvm, gfn_t gfn)
{
	struct vm_area_struct *vma;
	unsigned long addr, size;

	size = PAGE_SIZE;

	addr = gfn_to_hva(kvm, gfn);
	if (kvm_is_error_hva(addr))
		return PAGE_SIZE;

	down_read(&current->mm->mmap_sem);
	vma = find_vma(current->mm, addr);
	if (!vma)
		goto out;

	size = vma_kernel_pagesize(vma);

out:
	up_read(&current->mm->mmap_sem);

	return size;
}

static bool memslot_is_readonly(struct kvm_memory_slot *slot)
{
	return slot->flags & KVM_MEM_READONLY;
}

static unsigned long __gfn_to_hva_many(struct kvm_memory_slot *slot, gfn_t gfn,
				       gfn_t *nr_pages, bool write)
{
	if (!slot || slot->flags & KVM_MEMSLOT_INVALID)
		return KVM_HVA_ERR_BAD;

	if (memslot_is_readonly(slot) && write)
		return KVM_HVA_ERR_RO_BAD;

	if (nr_pages)
		*nr_pages = slot->npages - (gfn - slot->base_gfn);

	return __gfn_to_hva_memslot(slot, gfn);
}

static unsigned long gfn_to_hva_many(struct kvm_memory_slot *slot, gfn_t gfn,
				     gfn_t *nr_pages)
{
	return __gfn_to_hva_many(slot, gfn, nr_pages, true);
}

unsigned long gfn_to_hva_memslot(struct kvm_memory_slot *slot, gfn_t gfn)
{
	return gfn_to_hva_many(slot, gfn, NULL);
}
EXPORT_SYMBOL_GPL(gfn_to_hva_memslot);

unsigned long gfn_to_hva(struct vz_vm *kvm, gfn_t gfn)
{
	return gfn_to_hva_many(gfn_to_memslot(kvm, gfn), gfn, NULL);
}
EXPORT_SYMBOL_GPL(gfn_to_hva);

unsigned long kvm_vcpu_gfn_to_hva(struct vz_vcpu *vcpu, gfn_t gfn)
{
	return gfn_to_hva_many(kvm_vcpu_gfn_to_memslot(vcpu, gfn), gfn, NULL);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_gfn_to_hva);

/*
 * If writable is set to false, the hva returned by this function is only
 * allowed to be read.
 */
unsigned long gfn_to_hva_memslot_prot(struct kvm_memory_slot *slot, gfn_t gfn,
				      bool *writable)
{
	unsigned long hva = __gfn_to_hva_many(slot, gfn, NULL, false);

	if (!kvm_is_error_hva(hva) && writable)
		*writable = !memslot_is_readonly(slot);

	return hva;
}

unsigned long gfn_to_hva_prot(struct vz_vm *kvm, gfn_t gfn, bool *writable)
{
	struct kvm_memory_slot *slot = gfn_to_memslot(kvm, gfn);

	return gfn_to_hva_memslot_prot(slot, gfn, writable);
}

unsigned long kvm_vcpu_gfn_to_hva_prot(struct vz_vcpu *vcpu, gfn_t gfn,
				       bool *writable)
{
	struct kvm_memory_slot *slot = kvm_vcpu_gfn_to_memslot(vcpu, gfn);

	return gfn_to_hva_memslot_prot(slot, gfn, writable);
}


/*
 * The fast path to get the writable pfn which will be stored in @pfn,
 * true indicates success, otherwise false is returned.  It's also the
 * only part that runs if we can are in atomic context.
 */
static bool hva_to_pfn_fast(unsigned long addr, bool write_fault,
			    bool *writable, kvm_pfn_t *pfn)
{
	struct page *page[1];
	int npages;

	/*
	 * Fast pin a writable pfn only if it is a write fault request
	 * or the caller allows to map a writable pfn for a read fault
	 * request.
	 */
	if (!(write_fault || writable))
		return false;

	npages = __get_user_pages_fast(addr, 1, 1, page);
	if (npages == 1) {
		*pfn = page_to_pfn(page[0]);

		if (writable)
			*writable = true;
		return true;
	}

	return false;
}

/*
 * The slow path to get the pfn of the specified host virtual address,
 * 1 indicates success, -errno is returned if error is detected.
 */
static int hva_to_pfn_slow(unsigned long addr, bool *async, bool write_fault,
			   bool *writable, kvm_pfn_t *pfn)
{
	unsigned int flags = FOLL_HWPOISON;
	struct page *page;
	int npages = 0;

	might_sleep();

	if (writable)
		*writable = write_fault;

	if (write_fault)
		flags |= FOLL_WRITE;
	if (async)
		flags |= FOLL_NOWAIT;

	npages = get_user_pages_unlocked(addr, 1, &page, flags);
	if (npages != 1)
		return npages;

	/* map read fault as writable if possible */
	if (unlikely(!write_fault) && writable) {
		struct page *wpage;

		if (__get_user_pages_fast(addr, 1, 1, &wpage) == 1) {
			*writable = true;
			put_page(page);
			page = wpage;
		}
	}
	*pfn = page_to_pfn(page);
	return npages;
}

static bool vma_is_valid(struct vm_area_struct *vma, bool write_fault)
{
	if (unlikely(!(vma->vm_flags & VM_READ)))
		return false;

	if (write_fault && (unlikely(!(vma->vm_flags & VM_WRITE))))
		return false;

	return true;
}

static int hva_to_pfn_remapped(struct vm_area_struct *vma, unsigned long addr,
			       bool *async, bool write_fault, bool *writable,
			       kvm_pfn_t *p_pfn)
{
	unsigned long pfn;
	int r;

	r = follow_pfn(vma, addr, &pfn);
	if (r) {
		/*
		 * get_user_pages fails for VM_IO and VM_PFNMAP vmas and does
		 * not call the fault handler, so do it here.
		 */
		bool unlocked = false;
		r = fixup_user_fault(current, current->mm, addr,
				     (write_fault ? FAULT_FLAG_WRITE : 0),
				     &unlocked);
		if (unlocked)
			return -EAGAIN;
		if (r)
			return r;

		r = follow_pfn(vma, addr, &pfn);
		if (r)
			return r;
	}

	if (writable)
		*writable = true;

	/*
	 * Get a reference here because callers of *hva_to_pfn* and
	 * *gfn_to_pfn* ultimately call kvm_release_pfn_clean on the
	 * returned pfn.  This is only needed if the VMA has VM_MIXEDMAP
	 * set, but the kvm_get_pfn/kvm_release_pfn_clean pair will
	 * simply do nothing for reserved pfns.
	 *
	 * Whoever called remap_pfn_range is also going to call e.g.
	 * unmap_mapping_range before the underlying pages are freed,
	 * causing a call to our MMU notifier.
	 */
	kvm_get_pfn(pfn);

	*p_pfn = pfn;
	return 0;
}

/*
 * Pin guest page in memory and return its pfn.
 * @addr: host virtual address which maps memory to the guest
 * @atomic: whether this function can sleep
 * @async: whether this function need to wait IO complete if the
 *         host page is not in the memory
 * @write_fault: whether we should get a writable host page
 * @writable: whether it allows to map a writable host page for !@write_fault
 *
 * The function will map a writable host page for these two cases:
 * 1): @write_fault = true
 * 2): @write_fault = false && @writable, @writable will tell the caller
 *     whether the mapping is writable.
 */
static kvm_pfn_t hva_to_pfn(unsigned long addr, bool atomic, bool *async,
			    bool write_fault, bool *writable)
{
	struct vm_area_struct *vma;
	kvm_pfn_t pfn = 0;
	int npages, r;

	/* we can do it either atomically or asynchronously, not both */
	BUG_ON(atomic && async);

	if (hva_to_pfn_fast(addr, write_fault, writable, &pfn))
		return pfn;

	if (atomic)
		return KVM_PFN_ERR_FAULT;

	npages = hva_to_pfn_slow(addr, async, write_fault, writable, &pfn);
	if (npages == 1)
		return pfn;

	down_read(&current->mm->mmap_sem);
	if (npages == -EHWPOISON ||
	    (!async && check_user_page_hwpoison(addr))) {
		pfn = KVM_PFN_ERR_HWPOISON;
		goto exit;
	}

retry:
	vma = find_vma_intersection(current->mm, addr, addr + 1);

	if (vma == NULL)
		pfn = KVM_PFN_ERR_FAULT;
	else if (vma->vm_flags & (VM_IO | VM_PFNMAP)) {
		r = hva_to_pfn_remapped(vma, addr, async, write_fault, writable,
					&pfn);
		if (r == -EAGAIN)
			goto retry;
		if (r < 0)
			pfn = KVM_PFN_ERR_FAULT;
	} else {
		if (async && vma_is_valid(vma, write_fault))
			*async = true;
		pfn = KVM_PFN_ERR_FAULT;
	}
exit:
	up_read(&current->mm->mmap_sem);
	return pfn;
}

kvm_pfn_t __gfn_to_pfn_memslot(struct kvm_memory_slot *slot, gfn_t gfn,
			       bool atomic, bool *async, bool write_fault,
			       bool *writable)
{
	unsigned long addr = __gfn_to_hva_many(slot, gfn, NULL, write_fault);

	if (addr == KVM_HVA_ERR_RO_BAD) {
		if (writable)
			*writable = false;
		return KVM_PFN_ERR_RO_FAULT;
	}

	if (kvm_is_error_hva(addr)) {
		if (writable)
			*writable = false;
		return KVM_PFN_NOSLOT;
	}

	/* Do not map writable pfn in the readonly memslot. */
	if (writable && memslot_is_readonly(slot)) {
		*writable = false;
		writable = NULL;
	}

	return hva_to_pfn(addr, atomic, async, write_fault, writable);
}
EXPORT_SYMBOL_GPL(__gfn_to_pfn_memslot);

kvm_pfn_t gfn_to_pfn_prot(struct vz_vm *kvm, gfn_t gfn, bool write_fault,
			  bool *writable)
{
	return __gfn_to_pfn_memslot(gfn_to_memslot(kvm, gfn), gfn, false, NULL,
				    write_fault, writable);
}
EXPORT_SYMBOL_GPL(gfn_to_pfn_prot);

kvm_pfn_t gfn_to_pfn_memslot(struct kvm_memory_slot *slot, gfn_t gfn)
{
	return __gfn_to_pfn_memslot(slot, gfn, false, NULL, true, NULL);
}
EXPORT_SYMBOL_GPL(gfn_to_pfn_memslot);

kvm_pfn_t gfn_to_pfn_memslot_atomic(struct kvm_memory_slot *slot, gfn_t gfn)
{
	return __gfn_to_pfn_memslot(slot, gfn, true, NULL, true, NULL);
}
EXPORT_SYMBOL_GPL(gfn_to_pfn_memslot_atomic);

kvm_pfn_t gfn_to_pfn_atomic(struct vz_vm *kvm, gfn_t gfn)
{
	return gfn_to_pfn_memslot_atomic(gfn_to_memslot(kvm, gfn), gfn);
}
EXPORT_SYMBOL_GPL(gfn_to_pfn_atomic);

kvm_pfn_t kvm_vcpu_gfn_to_pfn_atomic(struct vz_vcpu *vcpu, gfn_t gfn)
{
	return gfn_to_pfn_memslot_atomic(kvm_vcpu_gfn_to_memslot(vcpu, gfn),
					 gfn);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_gfn_to_pfn_atomic);

kvm_pfn_t gfn_to_pfn(struct vz_vm *kvm, gfn_t gfn)
{
	return gfn_to_pfn_memslot(gfn_to_memslot(kvm, gfn), gfn);
}
EXPORT_SYMBOL_GPL(gfn_to_pfn);

kvm_pfn_t kvm_vcpu_gfn_to_pfn(struct vz_vcpu *vcpu, gfn_t gfn)
{
	return gfn_to_pfn_memslot(kvm_vcpu_gfn_to_memslot(vcpu, gfn), gfn);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_gfn_to_pfn);

int gfn_to_page_many_atomic(struct kvm_memory_slot *slot, gfn_t gfn,
			    struct page **pages, int nr_pages)
{
	unsigned long addr;
	gfn_t entry = 0;

	addr = gfn_to_hva_many(slot, gfn, &entry);
	if (kvm_is_error_hva(addr))
		return -1;

	if (entry < nr_pages)
		return 0;

	return __get_user_pages_fast(addr, nr_pages, 1, pages);
}
EXPORT_SYMBOL_GPL(gfn_to_page_many_atomic);

static struct page *kvm_pfn_to_page(kvm_pfn_t pfn)
{
	if (is_error_noslot_pfn(pfn))
		return KVM_ERR_PTR_BAD_PAGE;

	if (kvm_is_reserved_pfn(pfn)) {
		WARN_ON(1);
		return KVM_ERR_PTR_BAD_PAGE;
	}

	return pfn_to_page(pfn);
}

struct page *gfn_to_page(struct vz_vm *kvm, gfn_t gfn)
{
	kvm_pfn_t pfn;

	pfn = gfn_to_pfn(kvm, gfn);

	return kvm_pfn_to_page(pfn);
}
EXPORT_SYMBOL_GPL(gfn_to_page);

struct page *kvm_vcpu_gfn_to_page(struct vz_vcpu *vcpu, gfn_t gfn)
{
	kvm_pfn_t pfn;

	pfn = kvm_vcpu_gfn_to_pfn(vcpu, gfn);

	return kvm_pfn_to_page(pfn);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_gfn_to_page);

void kvm_release_page_clean(struct page *page)
{
	WARN_ON(is_error_page(page));

	kvm_release_pfn_clean(page_to_pfn(page));
}
EXPORT_SYMBOL_GPL(kvm_release_page_clean);

void kvm_release_pfn_clean(kvm_pfn_t pfn)
{
	if (!is_error_noslot_pfn(pfn) && !kvm_is_reserved_pfn(pfn))
		put_page(pfn_to_page(pfn));
}
EXPORT_SYMBOL_GPL(kvm_release_pfn_clean);

void kvm_release_page_dirty(struct page *page)
{
	WARN_ON(is_error_page(page));

	kvm_release_pfn_dirty(page_to_pfn(page));
}
EXPORT_SYMBOL_GPL(kvm_release_page_dirty);

void kvm_release_pfn_dirty(kvm_pfn_t pfn)
{
	kvm_set_pfn_dirty(pfn);
	kvm_release_pfn_clean(pfn);
}
EXPORT_SYMBOL_GPL(kvm_release_pfn_dirty);

void kvm_set_pfn_dirty(kvm_pfn_t pfn)
{
	if (!kvm_is_reserved_pfn(pfn)) {
		struct page *page = pfn_to_page(pfn);

		if (!PageReserved(page))
			SetPageDirty(page);
	}
}
EXPORT_SYMBOL_GPL(kvm_set_pfn_dirty);

void kvm_set_pfn_accessed(kvm_pfn_t pfn)
{
	if (!kvm_is_reserved_pfn(pfn))
		mark_page_accessed(pfn_to_page(pfn));
}
EXPORT_SYMBOL_GPL(kvm_set_pfn_accessed);

void kvm_get_pfn(kvm_pfn_t pfn)
{
	if (!kvm_is_reserved_pfn(pfn))
		get_page(pfn_to_page(pfn));
}
EXPORT_SYMBOL_GPL(kvm_get_pfn);

static int next_segment(unsigned long len, int offset)
{
	if (len > PAGE_SIZE - offset)
		return PAGE_SIZE - offset;
	else
		return len;
}

static int __kvm_read_guest_page(struct kvm_memory_slot *slot, gfn_t gfn,
				 void *data, int offset, int len)
{
	int r;
	unsigned long addr;

	addr = gfn_to_hva_memslot_prot(slot, gfn, NULL);
	if (kvm_is_error_hva(addr))
		return -EFAULT;
	r = __copy_from_user(data, (void __user *)addr + offset, len);
	if (r)
		return -EFAULT;
	return 0;
}

int kvm_read_guest_page(struct vz_vm *kvm, gfn_t gfn, void *data, int offset,
			int len)
{
	struct kvm_memory_slot *slot = gfn_to_memslot(kvm, gfn);

	return __kvm_read_guest_page(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_read_guest_page);

int kvm_vcpu_read_guest_page(struct vz_vcpu *vcpu, gfn_t gfn, void *data,
			     int offset, int len)
{
	struct kvm_memory_slot *slot = kvm_vcpu_gfn_to_memslot(vcpu, gfn);

	return __kvm_read_guest_page(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_read_guest_page);

int kvm_read_guest(struct vz_vm *kvm, gpa_t gpa, void *data, unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	int seg;
	int offset = offset_in_page(gpa);
	int ret;

	while ((seg = next_segment(len, offset)) != 0) {
		ret = kvm_read_guest_page(kvm, gfn, data, offset, seg);
		if (ret < 0)
			return ret;
		offset = 0;
		len -= seg;
		data += seg;
		++gfn;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(kvm_read_guest);

int kvm_vcpu_read_guest(struct vz_vcpu *vcpu, gpa_t gpa, void *data,
			unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	int seg;
	int offset = offset_in_page(gpa);
	int ret;

	while ((seg = next_segment(len, offset)) != 0) {
		ret = kvm_vcpu_read_guest_page(vcpu, gfn, data, offset, seg);
		if (ret < 0)
			return ret;
		offset = 0;
		len -= seg;
		data += seg;
		++gfn;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(kvm_vcpu_read_guest);

static int __kvm_read_guest_atomic(struct kvm_memory_slot *slot, gfn_t gfn,
				   void *data, int offset, unsigned long len)
{
	int r;
	unsigned long addr;

	addr = gfn_to_hva_memslot_prot(slot, gfn, NULL);
	if (kvm_is_error_hva(addr))
		return -EFAULT;
	pagefault_disable();
	r = __copy_from_user_inatomic(data, (void __user *)addr + offset, len);
	pagefault_enable();
	if (r)
		return -EFAULT;
	return 0;
}

int kvm_read_guest_atomic(struct vz_vm *kvm, gpa_t gpa, void *data,
			  unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	struct kvm_memory_slot *slot = gfn_to_memslot(kvm, gfn);
	int offset = offset_in_page(gpa);

	return __kvm_read_guest_atomic(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_read_guest_atomic);

int kvm_vcpu_read_guest_atomic(struct vz_vcpu *vcpu, gpa_t gpa, void *data,
			       unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	struct kvm_memory_slot *slot = kvm_vcpu_gfn_to_memslot(vcpu, gfn);
	int offset = offset_in_page(gpa);

	return __kvm_read_guest_atomic(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_read_guest_atomic);

static int __kvm_write_guest_page(struct kvm_memory_slot *memslot, gfn_t gfn,
				  const void *data, int offset, int len)
{
	int r;
	unsigned long addr;

	addr = gfn_to_hva_memslot(memslot, gfn);
	if (kvm_is_error_hva(addr))
		return -EFAULT;
	r = __copy_to_user((void __user *)addr + offset, data, len);
	if (r)
		return -EFAULT;
	mark_page_dirty_in_slot(memslot, gfn);
	return 0;
}

int kvm_write_guest_page(struct vz_vm *kvm, gfn_t gfn, const void *data,
			 int offset, int len)
{
	struct kvm_memory_slot *slot = gfn_to_memslot(kvm, gfn);

	return __kvm_write_guest_page(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_write_guest_page);

int kvm_vcpu_write_guest_page(struct vz_vcpu *vcpu, gfn_t gfn,
			      const void *data, int offset, int len)
{
	struct kvm_memory_slot *slot = kvm_vcpu_gfn_to_memslot(vcpu, gfn);

	return __kvm_write_guest_page(slot, gfn, data, offset, len);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_write_guest_page);

int kvm_write_guest(struct vz_vm *kvm, gpa_t gpa, const void *data,
		    unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	int seg;
	int offset = offset_in_page(gpa);
	int ret;

	while ((seg = next_segment(len, offset)) != 0) {
		ret = kvm_write_guest_page(kvm, gfn, data, offset, seg);
		if (ret < 0)
			return ret;
		offset = 0;
		len -= seg;
		data += seg;
		++gfn;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(kvm_write_guest);

int kvm_vcpu_write_guest(struct vz_vcpu *vcpu, gpa_t gpa, const void *data,
			 unsigned long len)
{
	gfn_t gfn = gpa >> PAGE_SHIFT;
	int seg;
	int offset = offset_in_page(gpa);
	int ret;

	while ((seg = next_segment(len, offset)) != 0) {
		ret = kvm_vcpu_write_guest_page(vcpu, gfn, data, offset, seg);
		if (ret < 0)
			return ret;
		offset = 0;
		len -= seg;
		data += seg;
		++gfn;
	}
	return 0;
}

static void mark_page_dirty_in_slot(struct kvm_memory_slot *memslot,
				    gfn_t gfn)
{
	if (memslot && memslot->dirty_bitmap) {
		unsigned long rel_gfn = gfn - memslot->base_gfn;

		set_bit_le(rel_gfn, memslot->dirty_bitmap);
	}
}

void mark_page_dirty(struct vz_vm *kvm, gfn_t gfn)
{
	struct kvm_memory_slot *memslot;

	memslot = gfn_to_memslot(kvm, gfn);
	mark_page_dirty_in_slot(memslot, gfn);
}

void kvm_vcpu_mark_page_dirty(struct vz_vcpu *vcpu, gfn_t gfn)
{
	struct kvm_memory_slot *memslot;

	memslot = kvm_vcpu_gfn_to_memslot(vcpu, gfn);
	mark_page_dirty_in_slot(memslot, gfn);
}
