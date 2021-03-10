#include "cp0.h"
.global ebase_error_entry_begin
.global ebase_error_entry_end
ebase_error_entry_begin:
INVALID_CODEFLOW_1
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
mfc0 k1, C0_CAUSE
dsrl k1, k1, 2
andi k1, k1, 0x1f
xor k1, k1, EXCCODE_SYS 
beq k1, zero, syscall
nop


mfc0 k1, C0_CAUSE
dsrl k1, k1, 2
andi k1, k1, 0x1f
# 标记 k0, 让 k1 在 VCPU Registers Dump 中间容易查找
li k0, 0xff3
INVALID_CODEFLOW_2

# beq k1, k0, ebase_tlb_entry_begin

syscall:
dmfc0 k0, C0_KSCRATCH1
sd $2, 0(k0)
sd $4, 8(k0)
sd $5, 16(k0)
sd $6, 24(k0)
sd $7, 32(k0)
sd $8, 40(k0)
sd $9, 48(k0)
HYPERCALL
INVALID_CODEFLOW_3
ld $2, 0(k0)
ld $7, 32(k0)
dmfc0 k0, C0_EPC
daddiu k0, k0, 4
dmtc0 k0, C0_EPC
eret
ebase_general_entry_end:

.global host_loop
.global switch_stack
switch_stack:
	move sp, a1
	dla t9, host_loop
	bal host_loop 


// TODO @function , maybe add this to other functions too
.set	noreorder
.global	dune_clone
.type	dune_clone,@function
dune_clone:
	li	$2, 5055
	syscall
	beq	$7, $0, 1f
	nop
	// parent failed
	jr	ra
	dsubu	$2, $0, $2
1:	beq	$2, $0, 1f
	nop
	// parent success
	jr	ra
	nop
// child gotos a new function
1:	ld	$25, 0(sp)	# function pointer
	ld	$4, 8(sp)	# argument pointer
	jalr	$25		# call the user's function
	nop
	move 	$4, $2
	li	$2, 5058
	syscall

// lasx 一个寄存器的大小是 # define FPU_REG_WIDTH	256
LEAF(__kvm_restore_lasx)
	xvld_b	0,  16 * 0,  a0
	xvld_b	1,  16 * 1,  a0
	xvld_b	2,  16 * 2,  a0
	xvld_b	3,  16 * 3,  a0
	xvld_b	4,  16 * 4,  a0
	xvld_b	5,  16 * 5,  a0
	xvld_b	6,  16 * 6,  a0
	xvld_b	7,  16 * 7,  a0
	xvld_b	8,  16 * 8,  a0
	xvld_b	9,  16 * 9,  a0
	xvld_b	10, 16 * 10, a0
	xvld_b	11, 16 * 11, a0
	xvld_b	12, 16 * 12, a0
	xvld_b	13, 16 * 13, a0
	xvld_b	14, 16 * 14, a0
	xvld_b	15, 16 * 15, a0
	xvld_b	16, 16 * 16, a0
	xvld_b	17, 16 * 17, a0
	xvld_b	18, 16 * 18, a0
	xvld_b	19, 16 * 19, a0
	xvld_b	20, 16 * 20, a0
	xvld_b	21, 16 * 21, a0
	xvld_b	22, 16 * 22, a0
	xvld_b	23, 16 * 23, a0
	xvld_b	24, 16 * 24, a0
	xvld_b	25, 16 * 25, a0
	xvld_b	26, 16 * 26, a0
	xvld_b	27, 16 * 27, a0
	xvld_b	28, 16 * 28, a0
	xvld_b	29, 16 * 29, a0
	xvld_b	30, 16 * 30, a0
	xvld_b	31, 16 * 31, a0
	jr	ra
	 nop
	END(__kvm_restore_lasx)
