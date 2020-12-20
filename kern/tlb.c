#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/srcu.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>
#include <asm/tlb.h>
#include <asm/tlbdebug.h>
#include "vz.h"

unsigned long GUESTID_MASK;
unsigned long GUESTID_FIRST_VERSION;
unsigned long GUESTID_VERSION_MASK;

/**
 * clear_root_gid() - Set GuestCtl1.RID for normal root operation.
 */
static inline void clear_root_gid(void)
{
	if (cpu_has_guestid) {
		clear_c0_guestctl1(MIPS_GCTL1_RID);
		mtc0_tlbw_hazard();
	}else{
    dune_err("cpu doesn't have guestid\n");
  }
}

/**
 * set_root_gid_to_guest_gid() - Set GuestCtl1.RID to match GuestCtl1.ID.
 *
 * Sets the root GuestID to match the current guest GuestID, for TLB operation
 * on the GPA->RPA mappings in the root TLB.
 *
 * The caller must be sure to disable HTW while the root GID is set, and
 * possibly longer if TLB registers are modified.
 */
static inline void set_root_gid_to_guest_gid(void)
{
	unsigned int guestctl1;

	if (cpu_has_guestid) {
		back_to_back_c0_hazard();
		guestctl1 = read_c0_guestctl1();
		guestctl1 = (guestctl1 & ~MIPS_GCTL1_RID) |
			((guestctl1 & MIPS_GCTL1_ID) >> MIPS_GCTL1_ID_SHIFT)
						     << MIPS_GCTL1_RID_SHIFT;
		write_c0_guestctl1(guestctl1);
		mtc0_tlbw_hazard();
	}else{
    dune_err("cpu doesn't have guestid\n");
  }
}

/**
 * kvm_vz_save_guesttlb() - Save a range of guest TLB entries.
 * @buf:	Buffer to write TLB entries into.
 * @index:	Start index.
 * @count:	Number of entries to save.
 *
 * Save a range of guest TLB entries. The caller must ensure interrupts are
 * disabled.
 */
void dune_vz_save_guesttlb(struct dune_mips_tlb *buf, unsigned int index,
			  unsigned int count)
{
	unsigned int end = index + count;
	unsigned long old_entryhi, old_entrylo0, old_entrylo1, old_pagemask;
	unsigned int guestctl1 = 0;
	int old_index, i;

	/* Save registers we're about to clobber */
	old_index = read_gc0_index();
	old_entryhi = read_gc0_entryhi();
	old_entrylo0 = read_gc0_entrylo0();
	old_entrylo1 = read_gc0_entrylo1();
	old_pagemask = read_gc0_pagemask();

	/* Set root GuestID for root probe */
	htw_stop();
	set_root_gid_to_guest_gid();
	if (cpu_has_guestid)
		guestctl1 = read_c0_guestctl1();

	/* Read each entry from guest TLB */
	for (i = index; i < end; ++i, ++buf) {
		write_gc0_index(i);

		mtc0_tlbr_hazard();
		guest_tlb_read();
		tlb_read_hazard();

		if (cpu_has_guestid &&
		    (read_c0_guestctl1() ^ guestctl1) & MIPS_GCTL1_RID) {
			/* Entry invalid or belongs to another guest */
			buf->tlb_hi = UNIQUE_GUEST_ENTRYHI(i);
			buf->tlb_lo[0] = 0;
			buf->tlb_lo[1] = 0;
			buf->tlb_mask = 0;
		} else {
			/* Entry belongs to the right guest */
			buf->tlb_hi = read_gc0_entryhi();
			buf->tlb_lo[0] = read_gc0_entrylo0();
			buf->tlb_lo[1] = read_gc0_entrylo1();
			buf->tlb_mask = read_gc0_pagemask();
		}
	}

	/* Clear root GuestID again */
	clear_root_gid();
	htw_start();

	/* Restore clobbered registers */
	write_gc0_index(old_index);
	write_gc0_entryhi(old_entryhi);
	write_gc0_entrylo0(old_entrylo0);
	write_gc0_entrylo1(old_entrylo1);
	write_gc0_pagemask(old_pagemask);

	tlbw_use_hazard();
}

/**
 * kvm_vz_load_guesttlb() - Save a range of guest TLB entries.
 * @buf:	Buffer to read TLB entries from.
 * @index:	Start index.
 * @count:	Number of entries to load.
 *
 * Load a range of guest TLB entries. The caller must ensure interrupts are
 * disabled.
 */
void dune_vz_load_guesttlb(const struct dune_mips_tlb *buf, unsigned int index,
			  unsigned int count)
{
	unsigned int end = index + count;
	unsigned long old_entryhi, old_entrylo0, old_entrylo1, old_pagemask;
	int old_index, i;

	/* Save registers we're about to clobber */
	old_index = read_gc0_index();
	old_entryhi = read_gc0_entryhi();
	old_entrylo0 = read_gc0_entrylo0();
	old_entrylo1 = read_gc0_entrylo1();
	old_pagemask = read_gc0_pagemask();

	/* Set root GuestID for root probe */
	htw_stop();
	set_root_gid_to_guest_gid();

	/* Write each entry to guest TLB */
	for (i = index; i < end; ++i, ++buf) {
		write_gc0_index(i);
		write_gc0_entryhi(buf->tlb_hi);
		write_gc0_entrylo0(buf->tlb_lo[0]);
		write_gc0_entrylo1(buf->tlb_lo[1]);
		write_gc0_pagemask(buf->tlb_mask);

		mtc0_tlbw_hazard();
		guest_tlb_write_indexed();
	}

	/* Clear root GuestID again */
	clear_root_gid();
	htw_start();

	/* Restore clobbered registers */
	write_gc0_index(old_index);
	write_gc0_entryhi(old_entryhi);
	write_gc0_entrylo0(old_entrylo0);
	write_gc0_entrylo1(old_entrylo1);
	write_gc0_pagemask(old_pagemask);

	tlbw_use_hazard();
}
