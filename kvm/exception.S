#include "cp0.h"
.global ebase_error_entry_begin
.global ebase_error_entry_end
ebase_error_entry_begin:
INVALID_CODEFLOW
ebase_error_entry_end:

.global ebase_tlb_entry_begin
.global ebase_tlb_entry_end
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


.global ebase_general_entry_begin
.global ebase_general_entry_end
ebase_general_entry_begin:
.set push
.set noat
mfc0 k1, C0_CAUSE
andi k1, k1, 0x7c
dsrl k1, k1, 2
xor k1, k1, EXCCODE_SYS 
beq k1, zero, syscall
nop
INVALID_CODEFLOW
syscall:
dla k0, HYP_PARA
sd $2, 0(k0)
sd $4, 8(k0)
sd $5, 16(k0)
sd $6, 24(k0)
sd $7, 32(k0)
sd $8, 40(k0)
sd $9, 48(k0)
HYPERCALL
ld $2, 0(k0)
dmfc0 k0, C0_EPC
daddiu k0, k0, 4
dmtc0 k0, C0_EPC
eret
.set pop
ebase_general_entry_end:

.global host_loop
.global switch_stack
switch_stack:
	move sp, a3
	dla t9, host_loop
	bal host_loop 

.global  HYP_PARA       # _num is a global symbol, when it is defined
.data               # switch to read-write data section
HYP_PARA:
.align 8
.dword  0
.dword  0
.dword  0
.dword  0
.dword  0
.dword  0
.dword  0
