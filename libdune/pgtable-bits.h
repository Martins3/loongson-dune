#ifndef __PAGETABLE_BITS_H
#define __PAGETABLE_BITS_H

// arch/mips/include/asm/pgtable-bits.h
#include <stdint.h>
/* Page table bits used for r4k systems */
enum pgtable_bits {
	/* Used only by software (masked out before writing EntryLo*) */
	_PAGE_PRESENT_SHIFT,
	_PAGE_WRITE_SHIFT,
	_PAGE_ACCESSED_SHIFT,
	_PAGE_MODIFIED_SHIFT,
	_PAGE_HUGE_SHIFT,
	_PAGE_PROTNONE_SHIFT,
	_PAGE_SPECIAL_SHIFT,
	/* Used by TLB hardware (placed in EntryLo*) */
	_PAGE_NO_EXEC_SHIFT,
	_PAGE_NO_READ_SHIFT,
	_PAGE_GLOBAL_SHIFT,
	_PAGE_VALID_SHIFT,
	_PAGE_DIRTY_SHIFT,
	_CACHE_SHIFT,
};

/* Used only by software */
#define _PAGE_PRESENT		(1 << _PAGE_PRESENT_SHIFT)
#define _PAGE_WRITE		(1 << _PAGE_WRITE_SHIFT)
#define _PAGE_ACCESSED		(1 << _PAGE_ACCESSED_SHIFT)
#define _PAGE_MODIFIED		(1 << _PAGE_MODIFIED_SHIFT)
# define _PAGE_HUGE		(1 << _PAGE_HUGE_SHIFT)

#define _PAGE_PROTNONE		(1 << _PAGE_PROTNONE_SHIFT)
#define _PAGE_SPECIAL		(1 << _PAGE_SPECIAL_SHIFT)

# define _PAGE_NO_EXEC		1 << _PAGE_NO_EXEC_SHIFT

#define _PAGE_NO_READ		(1 << _PAGE_NO_READ_SHIFT)
#define _PAGE_GLOBAL		(1 << _PAGE_GLOBAL_SHIFT)
#define _PAGE_VALID		(1 << _PAGE_VALID_SHIFT)
#define _PAGE_DIRTY		(1 << _PAGE_DIRTY_SHIFT)

# define _CACHE_MASK		(7 << _CACHE_SHIFT)
# define _PFN_SHIFT		(PAGE_SHIFT - 12 + _CACHE_SHIFT + 3)

#define _PAGE_SILENT_READ	_PAGE_VALID
#define _PAGE_SILENT_WRITE	_PAGE_DIRTY

#define _PFN_MASK		(~((1 << (_PFN_SHIFT)) - 1))

/*
 * pte_to_entrylo converts a page table entry (PTE) into a Mips
 * entrylo0/1 value.
 */
static inline uint64_t pte_to_entrylo(unsigned long pte_val)
{
		int sa;
		sa = 63 - _PAGE_NO_READ_SHIFT;
		/*
		 * C has no way to express that this is a DSRL
		 * _PAGE_NO_EXEC_SHIFT followed by a ROTR 2.  Luckily
		 * in the fast path this is done in assembly
		 */
		return (pte_val >> _PAGE_GLOBAL_SHIFT) |
			((pte_val & (_PAGE_NO_EXEC | _PAGE_NO_READ)) << sa);

	return pte_val >> _PAGE_GLOBAL_SHIFT;
}

#define _CACHE_CACHABLE_NONCOHERENT (3<<_CACHE_SHIFT)  /* LOONGSON       */
#define _CACHE_CACHABLE_COHERENT    (3<<_CACHE_SHIFT)  /* LOONGSON-3     */

#define __READABLE	(_PAGE_SILENT_READ | _PAGE_ACCESSED)
#define __WRITEABLE	(_PAGE_SILENT_WRITE | _PAGE_WRITE | _PAGE_MODIFIED)

#define _PAGE_CHG_MASK	(_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_SPECIAL | \
			 _PFN_MASK | _CACHE_MASK)
#endif
