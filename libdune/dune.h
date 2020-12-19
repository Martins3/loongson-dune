#include "types.h"
#include "pgtable.h"
#include "mmu-mips.h"
#include "../kern/dune.h"

#include <sys/queue.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

// page allocation

SLIST_HEAD(page_head, page);
typedef SLIST_ENTRY(page) page_entry_t;

struct page {
	page_entry_t link;
	uint64_t ref;
};

extern struct page *pages;
extern int num_pages;

#define PAGEBASE	0x200000000
#define MAX_PAGES	(1ul << 20) /* 4 GB of memory */


extern struct page * dune_page_alloc(void);
extern void dune_page_free(struct page *pg);
extern void dune_page_stats(void);

static inline struct page * dune_pa2page(physaddr_t pa)
{
	return &pages[PPN(pa - PAGEBASE)];
}

static inline physaddr_t dune_page2pa(struct page *pg)
{
	return PAGEBASE + ((pg - pages) << PAGE_SHIFT);
}

extern bool dune_page_isfrompool(physaddr_t pa);

static inline struct page * dune_page_get(struct page *pg)
{
	assert(pg >= pages);
	assert(pg < (pages + num_pages));

	pg->ref++;

	return pg;
}

static inline void dune_page_put(struct page *pg)
{
	assert(pg >= pages);
	assert(pg < (pages + num_pages));

	pg->ref--;

	if (!pg->ref)
		dune_page_free(pg);
}


// virtual memory

extern ptent_t *pgroot;
extern uintptr_t phys_limit;
extern uintptr_t mmap_base;
extern uintptr_t stack_base;

// physical addresss is limit to 64GB,
// so do a compact mapping is necessary.
static inline uintptr_t dune_mmap_addr_to_pa(void *ptr)
{
  assert(((uintptr_t) ptr) >= mmap_base);
	return ((uintptr_t) ptr) - mmap_base +
					phys_limit - GPA_STACK_SIZE - GPA_MAP_SIZE;
}

static inline uintptr_t dune_stack_addr_to_pa(void *ptr)
{

	return ((uintptr_t) ptr) - stack_base +
					phys_limit - GPA_STACK_SIZE;
}

static inline uintptr_t dune_va_to_pa(void *ptr)
{
	if ((uintptr_t) ptr >= stack_base)
		return dune_stack_addr_to_pa(ptr);
	else if ((uintptr_t) ptr >= mmap_base)
		return dune_mmap_addr_to_pa(ptr);
	else
		return (uintptr_t) ptr;
}

static inline void dune_flush_tlb(void)
{
  err(1, "TODO : need much more attention how tlb works\n");
}

#define PERM_NONE  	0	/* no access */
#define PERM_R		0x0001	/* read permission */
#define PERM_W		0x0002	/* write permission */
#define PERM_X		0x0004	/* execute permission */
#define PERM_BIG	0x0100	/* Use large pages */

// Maybe we need 
// TODO 

/* Define beginning and end of VA space */
#define VA_START		((void *)0)
#define VA_END			((void *)-1)

enum {
	CREATE_NONE = 0,
	CREATE_NORMAL = 1,
	CREATE_BIG = 2,
  CREATE_DEBUG = 3,
};

extern int dune_vm_mprotect(ptent_t *root, void *va, size_t len, int perm);
extern int dune_vm_map_phys(ptent_t *root, void *va, size_t len, void *pa, int perm);
extern int dune_vm_map_pages(ptent_t *root, void *va, size_t len, int perm);
extern void dune_vm_unmap(ptent_t *root, void *va, size_t len);
extern int dune_vm_lookup(ptent_t *root, void *va, int create, ptent_t **pte_out);

extern int debug_dune_get_vm_map_phys(ptent_t *root, void *va, size_t len, void *pa);

extern int dune_vm_insert_page(ptent_t *root, void *va, struct page *pg, int perm);
extern struct page * dune_vm_lookup_page(ptent_t *root, void *va);

extern ptent_t * dune_vm_clone(ptent_t *root);
extern void dune_vm_free(ptent_t *root);
extern void dune_vm_default_pgflt_handler(uintptr_t addr, uint64_t fec);

typedef int (*page_walk_cb)(const void *arg, ptent_t *ptep, void *va);
extern int dune_vm_page_walk(ptent_t *root, void *start_va, void *end_va,
			    page_walk_cb cb, const void *arg);

extern int dune_vm_page_walk_debug(ptent_t *root, void *start_va, void *end_va,
			    page_walk_cb cb, const void *arg);

// entry routines

extern int dune_init(bool map_full);
extern int dune_enter();

/**
 * dune_init_and_enter - initializes libdune and enters "Dune mode"
 * 
 * This is a simple initialization routine that handles everything
 * in one go. Note that you still need to call dune_enter() in
 * each new forked child or thread.
 * 
 * Returns 0 on success, otherwise failure.
 */
static inline int dune_init_and_enter(void)
{
	int ret;
	
	if ((ret = dune_init(1)))
		return ret;
	
	return dune_enter();
}
