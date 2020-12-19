#ifndef __PGTABLE_64_H
#define __PGTABLE_64_H
#include "pgtable-bits.h"
/* COPYRIGHT belongs to mips/include/asm/pgtable-64.h */

#define PGD_ORDER   0
#define PMD_ORDER		0
#define PTE_ORDER		0

#define PTRS_PER_PGD	((PAGE_SIZE << PGD_ORDER) / sizeof(pgd_t))
#define PTRS_PER_PMD	((PAGE_SIZE << PMD_ORDER) / sizeof(pmd_t))
#define PTRS_PER_PTE	((PAGE_SIZE << PTE_ORDER) / sizeof(pte_t))


#define PMD_SHIFT	(PAGE_SHIFT + (PAGE_SHIFT + PTE_ORDER - 3))
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))

#define PGDIR_SHIFT	(PMD_SHIFT + (PAGE_SHIFT + PMD_ORDER - 3))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))


#define pte_pfn(x)		((unsigned long)((x).pte >> _PFN_SHIFT))
#define pfn_pte(pfn, prot)	__pte(((pfn) << _PFN_SHIFT) | pgprot_val(prot))
#define pfn_pmd(pfn, prot)	__pmd(((pfn) << _PFN_SHIFT) | pgprot_val(prot))

#define pgd_index(address)	(((address) >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))
#define pmd_index(address)	(((address) >> PMD_SHIFT) & (PTRS_PER_PMD-1))

/* Find an entry in the third-level page table.. */
#define __pte_offset(address)						\
	(((address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset(dir, address)					\
	((pte_t *) pmd_page_vaddr(*(dir)) + __pte_offset(address))
#endif
