#include "cp0.h"

.set noreorder

.global ebase_tlb_entry_begin
.global ebase_tlb_entry_end

.global ebase_error_entry_begin
.global ebase_error_entry_end

ebase_error_entry_begin:
	.word INVALID_CODEFLOW
ebase_error_entry_end:

ebase_tlb_entry_begin:
	mfc0 k0, C0_PAGEGRAIN
	or k0, (0x1 << 29)
	mtc0 k0, C0_PAGEGRAIN

	li k0, PAGEMASK_1G_MASK
	dmtc0 k0, C0_PAGEMASK

	ori k0, PAGEMASK_1G_MASK_LOW_BITS
	nor k0, k0, zero
	dmfc0 k1, C0_BADVADDR
	and k1, k0, k1

	dsrl k1, EntryLo_VPN_SHITF
	ori k1, EntryLo_FLAGS
	dmtc0 k1, C0_ENTRYLO0

	li k0, EntryLo1_1G_OFFSET
	or k1, k0 
  dmtc0 k1, C0_ENTRYLO1
	tlbwr
	eret
ebase_tlb_entry_end:
