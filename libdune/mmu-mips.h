#ifndef JOS_MACHINE_MMU_MIPS_H
#define JOS_MACHINE_MMU_MIPS_H

#include "pgtable.h"
#include "pgtable-64.h"
#include "types.h"
#include <err.h>

#define UINT64(x) ((uint64_t) x)
#define CAST64(x) ((uint64_t) x)
#define ONE UINT64 (1)

typedef uint64_t ptent_t;

// TODO this implemention is ineffcient, do some clean up and examination on the copy
static inline int PDX(uint64_t va, int level){
  switch(level) {
    case 0:
      return __pte_offset(va);
    case 1:
      return pmd_index(va);
    case 2:
      return pgd_index(va);
    default:
      err(1, "invliad parameter `level` in %s \n", __FUNCTION__);
  }
}

#define NPTBITS	    11		/* log2(NPTENTRIES) */
#define NPTLVLS	    2		/* page table depth -1 */

#define NPTENTRIES	(1 << NPTBITS)

/* index into:         
 *   n = 0 => page table 
 *   n = 1 => page middle directory table
 *   n = 2 => page global directory table
 */
#define PDXMASK		((1 << NPTBITS) - 1)
#define PDSHIFT(n)	(PAGE_SHIFT + NPTBITS * (n))
#define PDX(n, la)	((((uintptr_t) (la)) >> PDSHIFT(n)) & PDXMASK)

#define PDADDR(n, i)	(((unsigned long) (i)) << PDSHIFT(n))
#define PTE_DEF_FLAGS	(_PAGE_PRESENT | _PAGE_WRITE)

/* page number field of address */
#define PPN(la)		((la) >> PAGE_SHIFT)

/* address in page table entry */   
#define PTE_ADDR(pte) ((physaddr_t) (pte) & UINT64(0x1ffffffffc000))
#define PTE_FLAGS(pte) ((physaddr_t) (pte) & UINT64(0x3fff))

/* offset in page */
#define PGOFF(la)	(((uintptr_t) (la)) & PGMASK)
#define PGADDR(la)	(((uintptr_t) (la)) & ~CAST64(PGMASK))

/* big page size */
#define BIG_PGSHIFT	PMD_SHIFT
#define BIG_PGSIZE	(1 << BIG_PGSHIFT)
#define BIG_PGMASK	(BIG_PGSIZE - 1)

/* offset in big page */
#define BIG_PGOFF(la)	(((uintptr_t) (la)) & BIG_PGMASK)
#define BIG_PGADDR(la)	(((uintptr_t) (la)) & ~CAST64(BIG_PGMASK))

#endif
