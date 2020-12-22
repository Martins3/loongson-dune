#ifndef _H_MEMSLOT
#define _H_MEMSLOT
#include <linux/pagemap.h>
#include "vz.h"

// ------------------------------- asm/kvm_host.h -----------------------------
// TODO in fact, these macro seperated in multiple files
/*
 * EVA has overlapping user & kernel address spaces, so user VAs may be >
 * PAGE_OFFSET. For this reason we can't use the default KVM_HVA_ERR_BAD of
 * PAGE_OFFSET.
 */

#define KVM_HVA_ERR_BAD (-1UL)
#define KVM_HVA_ERR_RO_BAD (-2UL)

static inline bool kvm_is_error_hva(unsigned long addr)
{
	return IS_ERR_VALUE(addr);
}

/*
 * The bit 16 ~ bit 31 of kvm_memory_region::flags are internally used
 * in kvm, other bits are visible for userspace which are defined in
 * include/linux/kvm_h.
 */
#define KVM_MEMSLOT_INVALID (1UL << 16)

/*
 * The bit 0 ~ bit 15 of kvm_memory_region::flags are visible for userspace,
 * other bits are reserved for kvm internal use which are defined in
 * include/linux/kvm_host.h.
 */
#define KVM_MEM_LOG_DIRTY_PAGES (1UL << 0)
#define KVM_MEM_READONLY (1UL << 1)

static inline int kvm_arch_vcpu_memslots_id(struct vz_vcpu *vcpu)
{
	return 0;
}

/*
 * For the normal pfn, the highest 12 bits should be zero,
 * so we can mask bit 62 ~ bit 52  to indicate the error pfn,
 * mask bit 63 to indicate the noslot pfn.
 */
#define KVM_PFN_ERR_MASK	(0x7ffULL << 52)
#define KVM_PFN_ERR_NOSLOT_MASK	(0xfffULL << 52)
#define KVM_PFN_NOSLOT		(0x1ULL << 63)

#define KVM_PFN_ERR_FAULT	(KVM_PFN_ERR_MASK)
#define KVM_PFN_ERR_HWPOISON	(KVM_PFN_ERR_MASK + 1)
#define KVM_PFN_ERR_RO_FAULT	(KVM_PFN_ERR_MASK + 2)

/*
 * error pfns indicate that the gfn is in slot but faild to
 * translate it to pfn on host.
 */
static inline bool is_error_pfn(kvm_pfn_t pfn)
{
	return !!(pfn & KVM_PFN_ERR_MASK);
}

/*
 * error_noslot pfns indicate that the gfn can not be
 * translated to pfn - it is not in slot or failed to
 * translate it to pfn.
 */
static inline bool is_error_noslot_pfn(kvm_pfn_t pfn)
{
	return !!(pfn & KVM_PFN_ERR_NOSLOT_MASK);
}

/* noslot pfn indicates that the gfn is not in slot. */
static inline bool is_noslot_pfn(kvm_pfn_t pfn)
{
	return pfn == KVM_PFN_NOSLOT;
}

#define KVM_ERR_PTR_BAD_PAGE	(ERR_PTR(-ENOENT))

static inline bool is_error_page(struct page *page)
{
	return IS_ERR(page);
}

/*
 * search_memslots() and __gfn_to_memslot() are here because they are
 * used in non-modular code in arch/powerpc/kvm/book3s_hv_rm_mmu.c.
 * gfn_to_memslot() itself isn't here as an inline because that would
 * bloat other code too much.
 */
static inline struct kvm_memory_slot *
search_memslots(struct kvm_memslots *slots, gfn_t gfn)
{
	int start = 0, end = slots->used_slots;
	int slot = atomic_read(&slots->lru_slot);
	struct kvm_memory_slot *memslots = slots->memslots;

	if (gfn >= memslots[slot].base_gfn &&
	    gfn < memslots[slot].base_gfn + memslots[slot].npages)
		return &memslots[slot];

	while (start < end) {
		slot = start + (end - start) / 2;

		if (gfn >= memslots[slot].base_gfn)
			end = slot;
		else
			start = slot + 1;
	}

	if (gfn >= memslots[start].base_gfn &&
	    gfn < memslots[start].base_gfn + memslots[start].npages) {
		atomic_set(&slots->lru_slot, start);
		return &memslots[start];
	}

	return NULL;
}

static inline struct kvm_memory_slot *id_to_memslot(struct kvm_memslots *slots,
						    int id)
{
	int index = slots->id_to_index[id];
	struct kvm_memory_slot *slot;

	slot = &slots->memslots[index];

	WARN_ON(slot->id != id);
	return slot;
}

static inline int check_user_page_hwpoison(unsigned long addr)
{
	int rc, flags = FOLL_HWPOISON | FOLL_WRITE;

	rc = get_user_pages(addr, 1, flags, NULL, NULL);
	return rc == -EHWPOISON;
}

// ---------------------------------- kvm_host.h -----------------------------

int gfn_to_page_many_atomic(struct kvm_memory_slot *slot, gfn_t gfn,
			    struct page **pages, int nr_pages);

struct page *gfn_to_page(struct vz_vm *kvm, gfn_t gfn);
unsigned long gfn_to_hva(struct vz_vm *kvm, gfn_t gfn);
unsigned long gfn_to_hva_prot(struct vz_vm *kvm, gfn_t gfn, bool *writable);
unsigned long gfn_to_hva_memslot(struct kvm_memory_slot *slot, gfn_t gfn);
unsigned long gfn_to_hva_memslot_prot(struct kvm_memory_slot *slot, gfn_t gfn,
				      bool *writable);
void kvm_release_page_clean(struct page *page);
void kvm_release_page_dirty(struct page *page);
void kvm_set_page_accessed(struct page *page);

kvm_pfn_t gfn_to_pfn_atomic(struct vz_vm *kvm, gfn_t gfn);
kvm_pfn_t gfn_to_pfn(struct vz_vm *kvm, gfn_t gfn);
kvm_pfn_t gfn_to_pfn_prot(struct vz_vm *kvm, gfn_t gfn, bool write_fault,
			  bool *writable);
kvm_pfn_t gfn_to_pfn_memslot(struct kvm_memory_slot *slot, gfn_t gfn);
kvm_pfn_t gfn_to_pfn_memslot_atomic(struct kvm_memory_slot *slot, gfn_t gfn);
kvm_pfn_t __gfn_to_pfn_memslot(struct kvm_memory_slot *slot, gfn_t gfn,
			       bool atomic, bool *async, bool write_fault,
			       bool *writable);

void kvm_release_pfn_clean(kvm_pfn_t pfn);
void kvm_release_pfn_dirty(kvm_pfn_t pfn);
void kvm_set_pfn_dirty(kvm_pfn_t pfn);
void kvm_set_pfn_accessed(kvm_pfn_t pfn);
void kvm_get_pfn(kvm_pfn_t pfn);

int kvm_read_guest_page(struct vz_vm *kvm, gfn_t gfn, void *data, int offset,
			int len);
int kvm_read_guest_atomic(struct vz_vm *kvm, gpa_t gpa, void *data,
			  unsigned long len);
int kvm_read_guest(struct vz_vm *kvm, gpa_t gpa, void *data, unsigned long len);
int kvm_read_guest_cached(struct vz_vm *kvm, struct gfn_to_hva_cache *ghc,
			  void *data, unsigned long len);
int kvm_write_guest_page(struct vz_vm *kvm, gfn_t gfn, const void *data,
			 int offset, int len);
int kvm_write_guest(struct vz_vm *kvm, gpa_t gpa, const void *data,
		    unsigned long len);
int kvm_write_guest_cached(struct vz_vm *kvm, struct gfn_to_hva_cache *ghc,
			   void *data, unsigned long len);
int kvm_write_guest_offset_cached(struct vz_vm *kvm, struct gfn_to_hva_cache *ghc,
				  void *data, unsigned int offset,
				  unsigned long len);
int kvm_gfn_to_hva_cache_init(struct vz_vm *kvm, struct gfn_to_hva_cache *ghc,
			      gpa_t gpa, unsigned long len);
int kvm_clear_guest_page(struct vz_vm *kvm, gfn_t gfn, int offset, int len);
int kvm_clear_guest(struct vz_vm *kvm, gpa_t gpa, unsigned long len);
struct kvm_memory_slot *gfn_to_memslot(struct vz_vm *kvm, gfn_t gfn);
bool kvm_is_visible_gfn(struct vz_vm *kvm, gfn_t gfn);
unsigned long kvm_host_page_size(struct vz_vm *kvm, gfn_t gfn);
void mark_page_dirty(struct vz_vm *kvm, gfn_t gfn);

struct kvm_memslots *kvm_vcpu_memslots(struct vz_vcpu *vcpu);
struct kvm_memory_slot *kvm_vcpu_gfn_to_memslot(struct vz_vcpu *vcpu,
						gfn_t gfn);
kvm_pfn_t kvm_vcpu_gfn_to_pfn_atomic(struct vz_vcpu *vcpu, gfn_t gfn);
kvm_pfn_t kvm_vcpu_gfn_to_pfn(struct vz_vcpu *vcpu, gfn_t gfn);
struct page *kvm_vcpu_gfn_to_page(struct vz_vcpu *vcpu, gfn_t gfn);
unsigned long kvm_vcpu_gfn_to_hva(struct vz_vcpu *vcpu, gfn_t gfn);
unsigned long kvm_vcpu_gfn_to_hva_prot(struct vz_vcpu *vcpu, gfn_t gfn,
				       bool *writable);
int kvm_vcpu_read_guest_page(struct vz_vcpu *vcpu, gfn_t gfn, void *data,
			     int offset, int len);
int kvm_vcpu_read_guest_atomic(struct vz_vcpu *vcpu, gpa_t gpa, void *data,
			       unsigned long len);
int kvm_vcpu_read_guest(struct vz_vcpu *vcpu, gpa_t gpa, void *data,
			unsigned long len);
int kvm_vcpu_write_guest_page(struct vz_vcpu *vcpu, gfn_t gfn,
			      const void *data, int offset, int len);
int kvm_vcpu_write_guest(struct vz_vcpu *vcpu, gpa_t gpa, const void *data,
			 unsigned long len);
void kvm_vcpu_mark_page_dirty(struct vz_vcpu *vcpu, gfn_t gfn);

static inline struct kvm_memslots *__kvm_memslots(struct vz_vm *kvm, int as_id)
{
	as_id = array_index_nospec(as_id, KVM_ADDRESS_SPACE_NUM);
	return srcu_dereference_check(
		kvm->memslots[as_id], &kvm->srcu,
		lockdep_is_held(&kvm->slots_lock) ||
			!refcount_read(&kvm->users_count));
}

static inline struct kvm_memslots *kvm_memslots(struct vz_vm *kvm)
{
	return __kvm_memslots(kvm, 0);
}

#define kvm_for_each_memslot(memslot, slots)	\
	for (memslot = &slots->memslots[0];	\
	      memslot < slots->memslots + KVM_MEM_SLOTS_NUM && memslot->npages;\
		memslot++)

static inline struct kvm_memory_slot *
__gfn_to_memslot(struct kvm_memslots *slots, gfn_t gfn)
{
	return search_memslots(slots, gfn);
}

static inline unsigned long __gfn_to_hva_memslot(struct kvm_memory_slot *slot,
						 gfn_t gfn)
{
	return slot->userspace_addr + (gfn - slot->base_gfn) * PAGE_SIZE;
}

static inline int memslot_id(struct vz_vm *kvm, gfn_t gfn)
{
	return gfn_to_memslot(kvm, gfn)->id;
}

static inline gfn_t hva_to_gfn_memslot(unsigned long hva,
				       struct kvm_memory_slot *slot)
{
	gfn_t gfn_offset = (hva - slot->userspace_addr) >> PAGE_SHIFT;

	return slot->base_gfn + gfn_offset;
}

static inline gpa_t gfn_to_gpa(gfn_t gfn)
{
	return (gpa_t)gfn << PAGE_SHIFT;
}

static inline gfn_t gpa_to_gfn(gpa_t gpa)
{
	return (gfn_t)(gpa >> PAGE_SHIFT);
}

static inline hpa_t pfn_to_hpa(kvm_pfn_t pfn)
{
	return (hpa_t)pfn << PAGE_SHIFT;
}

static inline struct page *kvm_vcpu_gpa_to_page(struct vz_vcpu *vcpu,
						gpa_t gpa)
{
	return kvm_vcpu_gfn_to_page(vcpu, gpa_to_gfn(gpa));
}

static inline bool kvm_is_error_gpa(struct vz_vm *kvm, gpa_t gpa)
{
	unsigned long hva = gfn_to_hva(kvm, gpa_to_gfn(gpa));

	return kvm_is_error_hva(hva);
}

#endif
