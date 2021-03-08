/* MIPS KVM register ids */
#define MIPS_CP0_32(_R, _S)                                                    \
	(KVM_REG_MIPS_CP0 | KVM_REG_SIZE_U32 | (8 * (_R) + (_S)))

#define MIPS_CP0_64(_R, _S)                                                    \
	(KVM_REG_MIPS_CP0 | KVM_REG_SIZE_U64 | (8 * (_R) + (_S)))

#define KVM_REG_MIPS_CP0_INDEX MIPS_CP0_32(0, 0)
#define KVM_REG_MIPS_CP0_RANDOM MIPS_CP0_32(1, 0)
#define KVM_REG_MIPS_CP0_ENTRYLO0 MIPS_CP0_64(2, 0)
#define KVM_REG_MIPS_CP0_ENTRYLO1 MIPS_CP0_64(3, 0)
#define KVM_REG_MIPS_CP0_CONTEXT MIPS_CP0_64(4, 0)
#define KVM_REG_MIPS_CP0_CONTEXTCONFIG MIPS_CP0_32(4, 1)
#define KVM_REG_MIPS_CP0_USERLOCAL MIPS_CP0_64(4, 2)
#define KVM_REG_MIPS_CP0_XCONTEXTCONFIG MIPS_CP0_64(4, 3)
#define KVM_REG_MIPS_CP0_PAGEMASK MIPS_CP0_32(5, 0)
#define KVM_REG_MIPS_CP0_PAGEGRAIN MIPS_CP0_32(5, 1)
#define KVM_REG_MIPS_CP0_SEGCTL0 MIPS_CP0_64(5, 2)
#define KVM_REG_MIPS_CP0_SEGCTL1 MIPS_CP0_64(5, 3)
#define KVM_REG_MIPS_CP0_SEGCTL2 MIPS_CP0_64(5, 4)
#define KVM_REG_MIPS_CP0_PWBASE MIPS_CP0_64(5, 5)
#define KVM_REG_MIPS_CP0_PWFIELD MIPS_CP0_64(5, 6)
#define KVM_REG_MIPS_CP0_PWSIZE MIPS_CP0_64(5, 7)
#define KVM_REG_MIPS_CP0_WIRED MIPS_CP0_32(6, 0)
#define KVM_REG_MIPS_CP0_PWCTL MIPS_CP0_32(6, 6)
#define KVM_REG_MIPS_CP0_HWRENA MIPS_CP0_32(7, 0)
#define KVM_REG_MIPS_CP0_BADVADDR MIPS_CP0_64(8, 0)
#define KVM_REG_MIPS_CP0_BADINSTR MIPS_CP0_32(8, 1)
#define KVM_REG_MIPS_CP0_BADINSTRP MIPS_CP0_32(8, 2)
#define KVM_REG_MIPS_CP0_COUNT MIPS_CP0_32(9, 0)
#define KVM_REG_MIPS_CP0_ENTRYHI MIPS_CP0_64(10, 0)
#define KVM_REG_MIPS_CP0_COMPARE MIPS_CP0_32(11, 0)
#define KVM_REG_MIPS_CP0_STATUS MIPS_CP0_32(12, 0)
#define KVM_REG_MIPS_CP0_INTCTL MIPS_CP0_32(12, 1)
#define KVM_REG_MIPS_CP0_CAUSE MIPS_CP0_32(13, 0)
#define KVM_REG_MIPS_CP0_EPC MIPS_CP0_64(14, 0)
#define KVM_REG_MIPS_CP0_PRID MIPS_CP0_32(15, 0)
#define KVM_REG_MIPS_CP0_EBASE MIPS_CP0_64(15, 1)
#define KVM_REG_MIPS_CP0_CONFIG MIPS_CP0_32(16, 0)
#define KVM_REG_MIPS_CP0_CONFIG1 MIPS_CP0_32(16, 1)
#define KVM_REG_MIPS_CP0_CONFIG2 MIPS_CP0_32(16, 2)
#define KVM_REG_MIPS_CP0_CONFIG3 MIPS_CP0_32(16, 3)
#define KVM_REG_MIPS_CP0_CONFIG4 MIPS_CP0_32(16, 4)
#define KVM_REG_MIPS_CP0_CONFIG5 MIPS_CP0_32(16, 5)
#define KVM_REG_MIPS_CP0_CONFIG6 MIPS_CP0_32(16, 6)
#define KVM_REG_MIPS_CP0_CONFIG7 MIPS_CP0_64(16, 7)
#define KVM_REG_MIPS_CP0_MAARI MIPS_CP0_64(17, 2)
#define KVM_REG_MIPS_CP0_XCONTEXT MIPS_CP0_64(20, 0)
#define KVM_REG_MIPS_CP0_GSCAUSE MIPS_CP0_64(22, 1)
#define KVM_REG_MIPS_CP0_ERROREPC MIPS_CP0_64(30, 0)
#define KVM_REG_MIPS_CP0_KSCRATCH1 MIPS_CP0_64(31, 2)
#define KVM_REG_MIPS_CP0_KSCRATCH2 MIPS_CP0_64(31, 3)
#define KVM_REG_MIPS_CP0_KSCRATCH3 MIPS_CP0_64(31, 4)
#define KVM_REG_MIPS_CP0_KSCRATCH4 MIPS_CP0_64(31, 5)
#define KVM_REG_MIPS_CP0_KSCRATCH5 MIPS_CP0_64(31, 6)
#define KVM_REG_MIPS_CP0_KSCRATCH6 MIPS_CP0_64(31, 7)

#define INIT_VALUE_INDEX 0
#define INIT_VALUE_RANDOM 0
#define INIT_VALUE_ENTRYLO0 10000 
#define INIT_VALUE_ENTRYLO1 20000
#define INIT_VALUE_CONTEXT 0 

#define INIT_VALUE_PAGEGRAIN 0xe8000000

// #define INIT_VALUE_USERLOCAL 0
#define INIT_VALUE_PAGEMASK 0
#define INIT_VALUE_PWBASE 0
#define INIT_VALUE_PWFIELD 0
#define INIT_VALUE_PWSIZE 0
#define INIT_VALUE_WIRED 0
#define INIT_VALUE_PWCTL 0

// function configure_hwrena in
// /home/maritns3/core/loongson-dune/cross/arch/mips/kernel/traps.c
#define INIT_VALUE_HWRENA 0xe000000f

#define INIT_VALUE_BADVADDR 0
#define INIT_VALUE_BADINSTR 0
#define INIT_VALUE_BADINSTRP 0

#define INIT_VALUE_COUNT 0
#define INIT_VALUE_ENTRYHI 0

// TODO I want delay the clock interrupt as late as possible
#define INIT_VALUE_COMPARE 0xffffffff

#define STATUS_BIT_INT (1 << 0)
#define STATUS_BIT_EXL (1 << 1)
#define STATUS_BIT_UX (1 << 5)
#define STATUS_BIT_SX (1 << 6)
#define STATUS_BIT_KX (1 << 7)
#define STATUS_BIT_FR (1 << 26)
#define STATUS_BIT_CP0 (1 << 28) // CP0
#define STATUS_BIT_CP1 (1 << 29) // FPU
#define STATUS_BIT_MM (1 << 30) // 多媒体指令协处理器
#define INIT_VALUE_STATUS STATUS_BIT_UX | STATUS_BIT_KX | STATUS_BIT_FR |STATUS_BIT_CP0 | STATUS_BIT_CP1 |STATUS_BIT_MM
// TODO how interrupt works ?
// TODO what's meaning of EIC and VI ?
#define INIT_VALUE_INTCTL 0xfc000100
#define INIT_VALUE_CAUSE 0
#define INIT_VALUE_EPC 0

#define INIT_VALUE_PRID 0x14c004

// TODO 显然 guest 是没有对于 config 进行初始化的 ?
#define INIT_VALUE_CONFIG  0x80064603
#define INIT_VALUE_CONFIG1 0xfeab5593
#define INIT_VALUE_CONFIG2 0x825f055f
#define INIT_VALUE_CONFIG3 0xdc8030a0
#define INIT_VALUE_CONFIG4 0xd0fcc268
#define INIT_VALUE_CONFIG5 0x40001
#define INIT_VALUE_CONFIG6 0xbd226783
#define INIT_VALUE_CONFIG7 0x5

#define INIT_VALUE_XCONTEXT 0
#define INIT_VALUE_GSCAUSE 0
#define INIT_VALUE_ERROREPC 0

// #define INIT_VALUE_KSCRATCH1 0
#define INIT_VALUE_KSCRATCH2 0
#define INIT_VALUE_KSCRATCH3 0
#define INIT_VALUE_KSCRATCH4 0
#define INIT_VALUE_KSCRATCH5 0
#define INIT_VALUE_KSCRATCH6 0

#define INVALID_CODEFLOW_1 .word (0x42000028 | (1 << 11))
#define INVALID_CODEFLOW_2 .word (0x42000028 | (22 << 11))
#define INVALID_CODEFLOW_3 .word (0x42000028 | (3 << 11))
#define INVALID_EBASE_POSITION 0x20
#define UNIMP_ERROR .word (0x42000028 | (0xf << 11))
#define HYPERCALL .word 0x42000028

/* Some CP0 registers */
#define C0_INDEX	0, 0
#define C0_ENTRYLO0	$2, 0
#define C0_TCBIND	2, 2
#define C0_ENTRYLO1	$3, 0
#define C0_CONTEXT	4, 0
#define C0_PAGEMASK	$5, 0
#define C0_PAGEGRAIN $5, 1
#define C0_PWBASE	5, 5
#define C0_PWFIELD	5, 6
#define C0_PWSIZE	5, 7
#define C0_PWCTL	6, 6
#define C0_BADVADDR	$8, 0
#define C0_PGD		9, 7
#define C0_ENTRYHI	$10, 0
#define C0_CAUSE	$13, 0
#define C0_EPC		$14, 0
#define C0_XCONTEXT	20, 0
#define C0_KSCRATCH1 $31, 2
#define C0_KSCRATCH2 $31, 3

#define zero	$0	/* wired zero */
#define AT	$1	/* assembler temp  - uppercase because of ".set at" */
#define v0	$2	/* return value */
#define v1	$3
#define a0	$4	/* argument registers */
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8	/* caller saved */
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define ta0	$12
#define t5	$13
#define ta1	$13
#define t6	$14
#define ta2	$14
#define t7	$15
#define ta3	$15
#define s0	$16	/* callee saved */
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24	/* caller saved */
#define t9	$25
#define jp	$25	/* PIC jump register */
#define k0	$26	/* kernel scratch */
#define k1	$27
#define gp	$28	/* global pointer */
#define sp	$29	/* stack pointer */
#define fp	$30	/* frame pointer */
#define s8	$30	/* same like fp! */
#define ra	$31	/* return address */

#define EntryLo_C 3 << 3
#define EntryLo_D 1 << 2
#define EntryLo_V 1 << 1
#define EntryLo_G 1 << 0
#define EntryLo_FLAGS EntryLo_C | EntryLo_D | EntryLo_V | EntryLo_G
#define EntryLo_FLAGS_LEN 6

#define PFN_SHIFT 12

#define EntryLo_VPN_SHITF (PFN_SHIFT - EntryLo_FLAGS_LEN)

#define PAGEMASK_1G_MASK 0x7fffe000
#define PAGEMASK_1G_MASK_LOW_BITS 0x1fff

#define EntryLo1_1G_OFFSET 0x40000000 >> EntryLo_VPN_SHITF

#define PAGESHIFT 14
#define PAGESIZE (1 << PAGESHIFT)

/*
 * Cause.ExcCode trap codes.
 */
#define EXCCODE_INT		0	/* Interrupt pending */
#define EXCCODE_MOD		1	/* TLB modified fault */
#define EXCCODE_TLBL		2	/* TLB miss on load or ifetch */
#define EXCCODE_TLBS		3	/* TLB miss on a store */
#define EXCCODE_ADEL		4	/* Address error on a load or ifetch */
#define EXCCODE_ADES		5	/* Address error on a store */
#define EXCCODE_IBE		6	/* Bus error on an ifetch */
#define EXCCODE_DBE		7	/* Bus error on a load or store */
#define EXCCODE_SYS		8	/* System call */
#define EXCCODE_BP		9	/* Breakpoint */
#define EXCCODE_RI		10	/* Reserved instruction exception */
#define EXCCODE_CPU		11	/* Coprocessor unusable */
#define EXCCODE_OV		12	/* Arithmetic overflow */
#define EXCCODE_TR		13	/* Trap instruction */
#define EXCCODE_MSAFPE		14	/* MSA floating point exception */
#define EXCCODE_FPE		15	/* Floating point exception */
#define EXCCODE_TLBRI		19	/* TLB Read-Inhibit exception */
#define EXCCODE_TLBXI		20	/* TLB Execution-Inhibit exception */
#define EXCCODE_MSADIS		21	/* MSA disabled exception */
#define EXCCODE_MDMX		22	/* MDMX unusable exception */
#define EXCCODE_WATCH		23	/* Watch address reference */
#define EXCCODE_MCHECK		24	/* Machine check */
#define EXCCODE_THREAD		25	/* Thread exceptions (MT) */
#define EXCCODE_DSPDIS		26	/* DSP disabled exception */
#define EXCCODE_GE		27	/* Virtualized guest exception (VZ) */


/**
 * copied from https://github.com/torvalds/linux/arch/mips/kernel/syscalls/syscall_n64.tbl
 * 55	n64	clone				__sys_clone
 * 56	n64	fork				__sys_fork
 * 57	n64	execve			sys_execve
 * 270 n64	kexec_load			sys_kexec_load
 * 316 n64	execveat			sys_execveat
 * 435 n64	clone3		__sys_clone3 
 * */
#define MIPS_N64_OFFSET 5000
#define SYS_CLONE (MIPS_N64_OFFSET + 55)
#define SYS_FORK (MIPS_N64_OFFSET + 56)
#define SYS_EXECVE (MIPS_N64_OFFSET + 57)
#define SYS_EXECLOAD (MIPS_N64_OFFSET + 270)
#define SYS_EXECAT (MIPS_N64_OFFSET + 316)
#define SYS_CLONE3 (MIPS_N64_OFFSET + 435)

#if __mips_isa_rev >= 6
#define SYSCALL_CLOBBERLIST \
	"$1", "$3", "$10", "$11", "$12", "$13", \
	"$14", "$15", "$24", "$25", "memory"
#else
#define SYSCALL_CLOBBERLIST \
	"$1", "$3", "$10", "$11", "$12", "$13", \
	"$14", "$15", "$24", "$25", "hi", "lo", "memory"
#endif
