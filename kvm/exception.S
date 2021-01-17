.set mips64
.set noreorder

.global ebase_tlb_entry_begin
.global ebase_tlb_entry_end

ebase_tlb_entry_begin:
	.word 0x42000028
ebase_tlb_entry_end:
