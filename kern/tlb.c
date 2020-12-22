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

/**
 * kvm_vz_local_flush_roottlb_all_guests() - Flush all root TLB entries for
 * guests.
 *
 * Invalidate all entries in root tlb which are GPA mappings.
 */
void dune_vz_local_flush_roottlb_all_guests(void)
{
	unsigned long flags;
	unsigned long old_entryhi, old_pagemask, old_guestctl1;
	int entry, diag;

	if (WARN_ON(!cpu_has_guestid))
		return;

	local_irq_save(flags);
	htw_stop();

	/* TLBR may clobber EntryHi.ASID, PageMask, and GuestCtl1.RID */
	old_entryhi = read_c0_entryhi();
	old_pagemask = read_c0_pagemask();
	old_guestctl1 = read_c0_guestctl1();

	/*
	 * Invalidate guest entries in root TLB while leaving root entries
	 * intact when possible.
	 */
	for (entry = 0; entry < current_cpu_data.tlbsize; entry++) {
		write_c0_index(entry);
		mtc0_tlbw_hazard();
		tlb_read();
		tlb_read_hazard();

#ifdef CONFIG_CPU_LOONGSON3
		diag = read_c0_diag();
		if (diag & 0xc0000) {
			change_c0_diag(0xc0000, 0x0);
			continue;
		}
#endif
		/* Don't invalidate non-guest (RVA) mappings in the root TLB */
		if (!(read_c0_guestctl1() & MIPS_GCTL1_RID))
			continue;

		/* Make sure all entries differ. */
		write_c0_entryhi(UNIQUE_ENTRYHI(entry));
		write_c0_entrylo0(0);
		write_c0_entrylo1(0);
		write_c0_guestctl1(0);
		mtc0_tlbw_hazard();
		tlb_write_indexed();
	}
#ifdef CONFIG_CPU_LOONGSON3
	//Should clear DIAG.MID after tlbr
	change_c0_diag(0xc0000, 0x0);
#endif

	write_c0_entryhi(old_entryhi);
	write_c0_pagemask(old_pagemask);
	write_c0_guestctl1(old_guestctl1);
	tlbw_use_hazard();

	htw_start();
	local_irq_restore(flags);
}

/**
 * kvm_vz_local_flush_guesttlb_all() - Flush all guest TLB entries.
 *
 * Invalidate all entries in guest tlb irrespective of guestid.
 */
void dune_vz_local_flush_guesttlb_all(void)
{
	unsigned long flags;
	unsigned long old_index;
	unsigned long old_entryhi;
	unsigned long old_entrylo[2];
	unsigned long old_pagemask;
	int entry;
	u64 cvmmemctl2 = 0;

	local_irq_save(flags);

	/* Preserve all clobbered guest registers */
	old_index = read_gc0_index();
	old_entryhi = read_gc0_entryhi();
	old_entrylo[0] = read_gc0_entrylo0();
	old_entrylo[1] = read_gc0_entrylo1();
	old_pagemask = read_gc0_pagemask();

	htw_stop();
	set_root_gid_to_guest_gid();

	switch (current_cpu_type()) {
	case CPU_CAVIUM_OCTEON3:
		/* Inhibit machine check due to multiple matching TLB entries */
		cvmmemctl2 = read_c0_cvmmemctl2();
		cvmmemctl2 |= CVMMEMCTL2_INHIBITTS;
		write_c0_cvmmemctl2(cvmmemctl2);
		break;
	};

	/* Invalidate guest entries in guest TLB */
	write_gc0_entrylo0(0);
	write_gc0_entrylo1(0);
	write_gc0_pagemask(0);
	for (entry = 0; entry < current_cpu_data.guest.tlbsize; entry++) {
		/* Make sure all entries differ. */
		write_gc0_index(entry);
		write_gc0_entryhi(UNIQUE_GUEST_ENTRYHI(entry));
		mtc0_tlbw_hazard();
		guest_tlb_write_indexed();
	}

	if (cvmmemctl2) {
		cvmmemctl2 &= ~CVMMEMCTL2_INHIBITTS;
		write_c0_cvmmemctl2(cvmmemctl2);
	};

	/* Clear root GuestID again */
	clear_root_gid();

	write_gc0_index(old_index);
	write_gc0_entryhi(old_entryhi);
	write_gc0_entrylo0(old_entrylo[0]);
	write_gc0_entrylo1(old_entrylo[1]);
	write_gc0_pagemask(old_pagemask);
	tlbw_use_hazard();

	htw_start();
	local_irq_restore(flags);

}

// TODO if cpu_has_guestid, we should remove this function
static u32 kvm_mips_get_root_asid(struct vz_vcpu *vcpu)
{
	if (cpu_has_guestid)
		return 0;
	else
    BUG();
}

static int _kvm_mips_host_tlb_inv(unsigned long entryhi)
{
	int idx;

	write_c0_entryhi(entryhi);
	mtc0_tlbw_hazard();

	tlb_probe();
	tlb_probe_hazard();
	idx = read_c0_index();

	if (idx >= current_cpu_data.tlbsize)
		BUG();

	if (idx >= 0) {
		write_c0_entryhi(UNIQUE_ENTRYHI(idx));
		write_c0_entrylo0(0);
		write_c0_entrylo1(0);
		mtc0_tlbw_hazard();

		tlb_write_indexed();
		tlbw_use_hazard();
	}

	return idx;
}

int kvm_vz_host_tlb_inv(struct vz_vcpu *vcpu, unsigned long va)
{
	int idx;
	unsigned long flags, old_entryhi;

	local_irq_save(flags);
	htw_stop();

	/* Set root GuestID for root probe and write of guest TLB entry */
	set_root_gid_to_guest_gid();

	old_entryhi = read_c0_entryhi();

	idx = _kvm_mips_host_tlb_inv((va & VPN2_MASK) |
				     kvm_mips_get_root_asid(vcpu));

	write_c0_entryhi(old_entryhi);
	clear_root_gid();
	mtc0_tlbw_hazard();

	htw_start();
	local_irq_restore(flags);

	/*
	 * We don't want to get reserved instruction exceptions for missing tlb
	 * entries.
	 */
	if (cpu_has_vtag_icache)
		flush_icache_all();

	if (idx > 0)
		dune_debug("%s: Invalidated root entryhi %#lx @ idx %d\n",
			  __func__, (va & VPN2_MASK) |
				    kvm_mips_get_root_asid(vcpu), idx);

	return 0;
}
