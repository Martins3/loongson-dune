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
mfc0 k0, C0_CAUSE
dsrl k0, k0, 2
andi k0, k0, 0x1f

move k1, k0
xor k1, k1, EXCCODE_SYS 
beq k1, zero, syscall
nop

# 标记 k1, 让 k0 在 VCPU Registers Dump 中间容易查找
li k1, 0xff3
INVALID_CODEFLOW_2

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
ld $2, 0(k0)
ld $3, 8(k0) # syscall pipe
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
// musl 的作者为什么不采用 LEAF 和 END ?
.set	noreorder
.global	dune_clone
.type	dune_clone,@function
dune_clone:
	li	$2, 5055 # clone
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
	li	$2, 5058 # exit
	syscall

/*
 * LEAF - declare leaf routine
 */
#define LEAF(symbol)					\
		.globl	symbol;				\
		.align	2;				\
		.type	symbol, @function;		\
		.ent	symbol, 0;			\
symbol:		.frame	sp, 0, ra;			\
		.cfi_startproc;				\
		.insn

/*
 * END - mark end of function
 */
#define END(function)					\
		.cfi_endproc;				\
		.end	function;			\
		.size	function, .-function

.macro	insn_if_mips enc
.insn
.word (\enc)
.endm

/*
 * starting with binutils 2.24.51.20140729, MIPS binutils warn about mixing
 * hardfloat and softfloat object files.  The kernel build uses soft-float by
 * default, so we also need to pass -msoft-float along to GAS if it supports it.
 * But this in turn causes assembler errors in files which access hardfloat
 * registers.  We detect if GAS supports "-msoft-float" in the Makefile and
 * explicitly put ".set hardfloat" where floating point registers are touched.
 */
#ifdef GAS_HAS_SET_HARDFLOAT
#define SET_HARDFLOAT .set hardfloat
#else
#define SET_HARDFLOAT
#endif

#define PTR_ADDU	daddu

.macro	xvst_b	wd, off, base
.set	push
.set	noat
SET_HARDFLOAT
PTR_ADDU $1, \base, \off
insn_if_mips 0xe8000819 | (\wd << 6)
.set	pop
.endm

.macro	insn32_if_mm enc
.endm

#define VCPU_FPR (256 / 8)

LEAF(get_fpu_regs)
	xvst_b	0,  VCPU_FPR * 0,  a0
	xvst_b	1,  VCPU_FPR * 1,  a0
	xvst_b	2,  VCPU_FPR * 2,  a0
	xvst_b	3,  VCPU_FPR * 3,  a0
	xvst_b	4,  VCPU_FPR * 4,  a0
	xvst_b	5,  VCPU_FPR * 5,  a0
	xvst_b	6,  VCPU_FPR * 6,  a0
	xvst_b	7,  VCPU_FPR * 7,  a0
	xvst_b	8,  VCPU_FPR * 8,  a0
	xvst_b	9,  VCPU_FPR * 9,  a0
	xvst_b	10, VCPU_FPR * 10, a0
	xvst_b	11, VCPU_FPR * 11, a0
	xvst_b	12, VCPU_FPR * 12, a0
	xvst_b	13, VCPU_FPR * 13, a0
	xvst_b	14, VCPU_FPR * 14, a0
	xvst_b	15, VCPU_FPR * 15, a0
	xvst_b	16, VCPU_FPR * 16, a0
	xvst_b	17, VCPU_FPR * 17, a0
	xvst_b	18, VCPU_FPR * 18, a0
	xvst_b	19, VCPU_FPR * 19, a0
	xvst_b	20, VCPU_FPR * 20, a0
	xvst_b	21, VCPU_FPR * 21, a0
	xvst_b	22, VCPU_FPR * 22, a0
	xvst_b	23, VCPU_FPR * 23, a0
	xvst_b	24, VCPU_FPR * 24, a0
	xvst_b	25, VCPU_FPR * 25, a0
	xvst_b	26, VCPU_FPR * 26, a0
	xvst_b	27, VCPU_FPR * 27, a0
	xvst_b	28, VCPU_FPR * 28, a0
	xvst_b	29, VCPU_FPR * 29, a0
	xvst_b	30, VCPU_FPR * 30, a0
	xvst_b	31, VCPU_FPR * 31, a0
	jr	ra
	 nop
END(get_fpu_regs)

#define fcr31        $31      /* FPU status register */
LEAF(get_fcsr)
	.set	push
	cfc1	t0, fcr31
	sw	t0, 0(a0)
	jr	ra
	nop
	.set	pop
END(get_fcsr)

.macro	_ctcmsa	cd, rs
.set	push
.set	noat
SET_HARDFLOAT
move	$1, \rs
insn_if_mips 0x783e0819 | (\cd << 6)
insn32_if_mm 0x583e0816 | (\cd << 6)
.set	pop
.endm

#define MSA_CSR		1
LEAF(get_msacsr)
	_ctcmsa	MSA_CSR, t0
	sw	t0, 0(a0)
	jr	ra
	nop
END(get_msacsr)

// reference musl/src/unistd/mips64/pipe.s
.set	noreorder
.global	host_loop_pipe
.type	host_loop_pipe,@function
host_loop_pipe:
	li	$2, 5021
	syscall
	beq	$7, $0, 1f
	nop
  jr ra # syscall 返回值 $2，函数的返回值也是 $2，所以直接返回
  nop
1:	sw	$2, 0($4)
	sw	$3, 4($4)
	move	$2, $0
	jr	ra
	nop
