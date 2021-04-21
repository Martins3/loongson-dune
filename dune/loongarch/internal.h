#ifndef INTERNAL_H_6IUWCEFP
#define INTERNAL_H_6IUWCEFP

/* Loongarch KVM register ids */
#define LOONGARCH_CSR_32(_R, _S)                                               \
	(KVM_REG_LOONGARCH_CSR | KVM_REG_SIZE_U32 | (8 * (_R) + (_S)))

#define LOONGARCH_CSR_64(_R, _S)                                               \
	(KVM_REG_LOONGARCH_CSR | KVM_REG_SIZE_U64 | (8 * (_R) + (_S)))

#define KVM_LOONGARCH_CSR_CRMD 0
#define KVM_LOONGARCH_CSR_PRMD 1
#define KVM_LOONGARCH_CSR_EUEN 2
#define KVM_LOONGARCH_CSR_MISC 3
#define KVM_LOONGARCH_CSR_ECFG 4
#define KVM_LOONGARCH_CSR_ESTAT 5
#define KVM_LOONGARCH_CSR_EPC 6
#define KVM_LOONGARCH_CSR_BADV 7
#define KVM_LOONGARCH_CSR_BADI 8
#define KVM_LOONGARCH_CSR_EBASE 0xc
#define KVM_LOONGARCH_CSR_TLBIDX 0x10
#define KVM_LOONGARCH_CSR_TLBHI 0x11
#define KVM_LOONGARCH_CSR_TLBLO0 0x12
#define KVM_LOONGARCH_CSR_TLBLO1 0x13
#define KVM_LOONGARCH_CSR_GTLBC 0x15
#define KVM_LOONGARCH_CSR_TRGP 0x16
#define KVM_LOONGARCH_CSR_ASID 0x18
#define KVM_LOONGARCH_CSR_PGDL 0x19
#define KVM_LOONGARCH_CSR_PGDH 0x1a
#define KVM_LOONGARCH_CSR_PGD 0x1b
#define KVM_LOONGARCH_CSR_PWCTL0 0x1c
#define KVM_LOONGARCH_CSR_PWCTL1 0x1d
#define KVM_LOONGARCH_CSR_STLBPS 0x1e
#define KVM_LOONGARCH_CSR_RVACFG 0x1f
#define KVM_LOONGARCH_CSR_CPUNUM 0x20
#define KVM_LOONGARCH_CSR_PRCFG1 0x21
#define KVM_LOONGARCH_CSR_PRCFG2 0x22
#define KVM_LOONGARCH_CSR_PRCFG3 0x23
#define KVM_LOONGARCH_CSR_KSCRATCH0 0x30
#define KVM_LOONGARCH_CSR_KSCRATCH1 0x31
#define KVM_LOONGARCH_CSR_KSCRATCH2 0x32
#define KVM_LOONGARCH_CSR_KSCRATCH3 0x33
#define KVM_LOONGARCH_CSR_KSCRATCH4 0x34
#define KVM_LOONGARCH_CSR_KSCRATCH5 0x35
#define KVM_LOONGARCH_CSR_KSCRATCH6 0x36
#define KVM_LOONGARCH_CSR_KSCRATCH7 0x37
#define KVM_LOONGARCH_CSR_KSCRATCH8 0x38
#define KVM_LOONGARCH_CSR_TIMERID 0x40
#define KVM_LOONGARCH_CSR_TIMERCFG 0x41
#define KVM_LOONGARCH_CSR_TIMERTICK 0x42
#define KVM_LOONGARCH_CSR_TIMEROFFSET 0x43
#define KVM_LOONGARCH_CSR_GSTAT 0x50
#define KVM_LOONGARCH_CSR_GCFG 0x51
#define KVM_LOONGARCH_CSR_GINTC 0x52
#define KVM_LOONGARCH_CSR_GCNTC 0x53
#define KVM_LOONGARCH_CSR_LLBCTL 0x60
#define KVM_LOONGARCH_CSR_IMPCTL1 0x80
#define KVM_LOONGARCH_CSR_IMPCTL2 0x81
#define KVM_LOONGARCH_CSR_GNMI 0x82
#define KVM_LOONGARCH_CSR_TLBREBASE 0x88
#define KVM_LOONGARCH_CSR_TLBRBADV 0x89
#define KVM_LOONGARCH_CSR_TLBREPC 0x8a
#define KVM_LOONGARCH_CSR_TLBRSAVE 0x8b
#define KVM_LOONGARCH_CSR_TLBRELO0 0x8c
#define KVM_LOONGARCH_CSR_TLBRELO1 0x8d
#define KVM_LOONGARCH_CSR_TLBREHI 0x8e
#define KVM_LOONGARCH_CSR_TLBRPRMD 0x8f
#define KVM_LOONGARCH_CSR_ERRCTL 0x90
#define KVM_LOONGARCH_CSR_ERRINFO1 0x91
#define KVM_LOONGARCH_CSR_ERRINFO2 0x92
#define KVM_LOONGARCH_CSR_ERREBASE 0x93
#define KVM_LOONGARCH_CSR_ERREPC 0x94
#define KVM_LOONGARCH_CSR_ERRSAVE 0x95
#define KVM_LOONGARCH_CSR_CTAG 0x98
#define KVM_LOONGARCH_CSR_MCSR0 0xc0
#define KVM_LOONGARCH_CSR_MCSR1 0xc1
#define KVM_LOONGARCH_CSR_MCSR2 0xc2
#define KVM_LOONGARCH_CSR_MCSR3 0xc3
#define KVM_LOONGARCH_CSR_MCSR8 0xc8
#define KVM_LOONGARCH_CSR_MCSR9 0xc9
#define KVM_LOONGARCH_CSR_MCSR10 0xca
#define KVM_LOONGARCH_CSR_MCSR24 0xf0
#define KVM_LOONGARCH_CSR_UCAWIN 0x100
#define KVM_LOONGARCH_CSR_UCAWIN0_LO 0x102
#define KVM_LOONGARCH_CSR_UCAWIN0_HI 0x103
#define KVM_LOONGARCH_CSR_UCAWIN1_LO 0x104
#define KVM_LOONGARCH_CSR_UCAWIN1_HI 0x105
#define KVM_LOONGARCH_CSR_UCAWIN2_LO 0x106
#define KVM_LOONGARCH_CSR_UCAWIN2_HI 0x107
#define KVM_LOONGARCH_CSR_UCAWIN3_LO 0x108
#define KVM_LOONGARCH_CSR_UCAWIN3_HI 0x109
#define KVM_LOONGARCH_CSR_DMWIN0 0x180
#define KVM_LOONGARCH_CSR_DMWIN1 0x181
#define KVM_LOONGARCH_CSR_DMWIN2 0x182
#define KVM_LOONGARCH_CSR_DMWIN3 0x183
#define KVM_LOONGARCH_CSR_PERF0_EVENT 0x200
#define KVM_LOONGARCH_CSR_PERF0_COUNT 0x201
#define KVM_LOONGARCH_CSR_PERF1_EVENT 0x202
#define KVM_LOONGARCH_CSR_PERF1_COUNT 0x203
#define KVM_LOONGARCH_CSR_PERF2_EVENT 0x204
#define KVM_LOONGARCH_CSR_PERF2_COUNT 0x205
#define KVM_LOONGARCH_CSR_PERF3_EVENT 0x206
#define KVM_LOONGARCH_CSR_PERF3_COUNT 0x207
#define KVM_LOONGARCH_CSR_DEBUG 0x500
#define KVM_LOONGARCH_CSR_DEPC 0x501
#define KVM_LOONGARCH_CSR_DESAVE 0x502

#define KVM_CSR_CRMD LOONGARCH_CSR_64(0, 0)
#define KVM_CSR_PRMD LOONGARCH_CSR_64(1, 0)
#define KVM_CSR_EUEN LOONGARCH_CSR_64(2, 0)
#define KVM_CSR_MISC LOONGARCH_CSR_64(3, 0)
#define KVM_CSR_ECFG LOONGARCH_CSR_64(4, 0)
#define KVM_CSR_ESTAT LOONGARCH_CSR_64(5, 0)
#define KVM_CSR_EPC LOONGARCH_CSR_64(6, 0)
#define KVM_CSR_BADV LOONGARCH_CSR_64(7, 0)
#define KVM_CSR_BADI LOONGARCH_CSR_64(8, 0)
#define KVM_CSR_EBASE LOONGARCH_CSR_64(0xc, 0)
#define KVM_CSR_TLBIDX LOONGARCH_CSR_64(0x10, 0)
#define KVM_CSR_TLBHI LOONGARCH_CSR_64(0x11, 0)
#define KVM_CSR_TLBLO0 LOONGARCH_CSR_64(0x12, 0)
#define KVM_CSR_TLBLO1 LOONGARCH_CSR_64(0x13, 0)
#define KVM_CSR_GTLBC LOONGARCH_CSR_64(0x15, 0)
#define KVM_CSR_TRGP LOONGARCH_CSR_64(0x16, 0)
#define KVM_CSR_ASID LOONGARCH_CSR_64(0x18, 0)
#define KVM_CSR_PGDL LOONGARCH_CSR_64(0x19, 0)
#define KVM_CSR_PGDH LOONGARCH_CSR_64(0x1a, 0)
#define KVM_CSR_PGD LOONGARCH_CSR_64(0x1b, 0)
#define KVM_CSR_PWCTL0 LOONGARCH_CSR_64(0x1c, 0)
#define KVM_CSR_PWCTL1 LOONGARCH_CSR_64(0x1d, 0)
#define KVM_CSR_STLBPS LOONGARCH_CSR_64(0x1e, 0)
#define KVM_CSR_RVACFG LOONGARCH_CSR_64(0x1f, 0)
#define KVM_CSR_CPUNUM LOONGARCH_CSR_64(0x20, 0)
#define KVM_CSR_PRCFG1 LOONGARCH_CSR_64(0x21, 0)
#define KVM_CSR_PRCFG2 LOONGARCH_CSR_64(0x22, 0)
#define KVM_CSR_PRCFG3 LOONGARCH_CSR_64(0x23, 0)
#define KVM_CSR_KSCRATCH0 LOONGARCH_CSR_64(0x30, 0)
#define KVM_CSR_KSCRATCH1 LOONGARCH_CSR_64(0x31, 0)
#define KVM_CSR_KSCRATCH2 LOONGARCH_CSR_64(0x32, 0)
#define KVM_CSR_KSCRATCH3 LOONGARCH_CSR_64(0x33, 0)
#define KVM_CSR_KSCRATCH4 LOONGARCH_CSR_64(0x34, 0)
#define KVM_CSR_KSCRATCH5 LOONGARCH_CSR_64(0x35, 0)
#define KVM_CSR_KSCRATCH6 LOONGARCH_CSR_64(0x36, 0)
#define KVM_CSR_KSCRATCH7 LOONGARCH_CSR_64(0x37, 0)
#define KVM_CSR_KSCRATCH8 LOONGARCH_CSR_64(0x38, 0)
#define KVM_CSR_TIMERID LOONGARCH_CSR_64(0x40, 0)
#define KVM_CSR_TIMERCFG LOONGARCH_CSR_64(0x41, 0)
#define KVM_CSR_TIMERTICK LOONGARCH_CSR_64(0x42, 0)
#define KVM_CSR_TIMEROFFSET LOONGARCH_CSR_64(0x43, 0)
#define KVM_CSR_GSTAT LOONGARCH_CSR_64(0x50, 0)
#define KVM_CSR_GCFG LOONGARCH_CSR_64(0x51, 0)
#define KVM_CSR_GINTC LOONGARCH_CSR_64(0x52, 0)
#define KVM_CSR_GCNTC LOONGARCH_CSR_64(0x53, 0)
#define KVM_CSR_LLBCTL LOONGARCH_CSR_64(0x60, 0)
#define KVM_CSR_IMPCTL1 LOONGARCH_CSR_64(0x80, 0)
#define KVM_CSR_IMPCTL2 LOONGARCH_CSR_64(0x81, 0)
#define KVM_CSR_GNMI LOONGARCH_CSR_64(0x82, 0)
#define KVM_CSR_TLBREBASE LOONGARCH_CSR_64(0x88, 0)
#define KVM_CSR_TLBRBADV LOONGARCH_CSR_64(0x89, 0)
#define KVM_CSR_TLBREPC LOONGARCH_CSR_64(0x8a, 0)
#define KVM_CSR_TLBRSAVE LOONGARCH_CSR_64(0x8b, 0)
#define KVM_CSR_TLBRELO0 LOONGARCH_CSR_64(0x8c, 0)
#define KVM_CSR_TLBRELO1 LOONGARCH_CSR_64(0x8d, 0)
#define KVM_CSR_TLBREHI LOONGARCH_CSR_64(0x8e, 0)
#define KVM_CSR_TLBRPRMD LOONGARCH_CSR_64(0x8f, 0)
#define KVM_CSR_ERRCTL LOONGARCH_CSR_64(0x90, 0)
#define KVM_CSR_ERRINFO1 LOONGARCH_CSR_64(0x91, 0)
#define KVM_CSR_ERRINFO2 LOONGARCH_CSR_64(0x92, 0)
#define KVM_CSR_ERREBASE LOONGARCH_CSR_64(0x93, 0)
#define KVM_CSR_ERREPC LOONGARCH_CSR_64(0x94, 0)
#define KVM_CSR_ERRSAVE LOONGARCH_CSR_64(0x95, 0)
#define KVM_CSR_CTAG LOONGARCH_CSR_64(0x98, 0)
#define KVM_CSR_MCSR0 LOONGARCH_CSR_64(0xc0, 0)
#define KVM_CSR_MCSR1 LOONGARCH_CSR_64(0xc1, 0)
#define KVM_CSR_MCSR2 LOONGARCH_CSR_64(0xc2, 0)
#define KVM_CSR_MCSR3 LOONGARCH_CSR_64(0xc3, 0)
#define KVM_CSR_MCSR8 LOONGARCH_CSR_64(0xc8, 0)
#define KVM_CSR_MCSR9 LOONGARCH_CSR_64(0xc9, 0)
#define KVM_CSR_MCSR10 LOONGARCH_CSR_64(0xca, 0)
#define KVM_CSR_MCSR24 LOONGARCH_CSR_64(0xf0, 0)
#define KVM_CSR_UCWIN LOONGARCH_CSR_64(0x100, 0)
#define KVM_CSR_UCWIN0_LO LOONGARCH_CSR_64(0x102, 0)
#define KVM_CSR_UCWIN0_HI LOONGARCH_CSR_64(0x103, 0)
#define KVM_CSR_UCWIN1_LO LOONGARCH_CSR_64(0x104, 0)
#define KVM_CSR_UCWIN1_HI LOONGARCH_CSR_64(0x105, 0)
#define KVM_CSR_UCWIN2_LO LOONGARCH_CSR_64(0x106, 0)
#define KVM_CSR_UCWIN2_HI LOONGARCH_CSR_64(0x107, 0)
#define KVM_CSR_UCWIN3_LO LOONGARCH_CSR_64(0x108, 0)
#define KVM_CSR_UCWIN3_HI LOONGARCH_CSR_64(0x109, 0)
#define KVM_CSR_DMWIN0 LOONGARCH_CSR_64(0x180, 0)
#define KVM_CSR_DMWIN1 LOONGARCH_CSR_64(0x181, 0)
#define KVM_CSR_DMWIN2 LOONGARCH_CSR_64(0x182, 0)
#define KVM_CSR_DMWIN3 LOONGARCH_CSR_64(0x183, 0)
#define KVM_CSR_PERF0_EVENT LOONGARCH_CSR_64(0x200, 0)
#define KVM_CSR_PERF0_COUNT LOONGARCH_CSR_64(0x201, 0)
#define KVM_CSR_PERF1_EVENT LOONGARCH_CSR_64(0x202, 0)
#define KVM_CSR_PERF1_COUNT LOONGARCH_CSR_64(0x203, 0)
#define KVM_CSR_PERF2_EVENT LOONGARCH_CSR_64(0x204, 0)
#define KVM_CSR_PERF2_COUNT LOONGARCH_CSR_64(0x205, 0)
#define KVM_CSR_PERF3_EVENT LOONGARCH_CSR_64(0x206, 0)
#define KVM_CSR_PERF3_COUNT LOONGARCH_CSR_64(0x207, 0)
#define KVM_CSR_DEBUG LOONGARCH_CSR_64(0x500, 0)
#define KVM_CSR_DEPC LOONGARCH_CSR_64(0x501, 0)
#define KVM_CSR_DESAVE LOONGARCH_CSR_64(0x502, 0)

// following copied from arch/loongarch/include/asm/loongarchregs.h
/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#define _U64CAST_
#else
#define _ULCAST_ (unsigned long)
#define _U64CAST_ (u64)
#endif

#ifdef __ASSEMBLY__
#define _ATYPE_
#define _ATYPE32_
#define _ATYPE64_
#define _CONST64_(x)	x
#else
#define _ATYPE_		__PTRDIFF_TYPE__
#define _ATYPE32_	int
#define _ATYPE64_	__s64
#ifdef CONFIG_64BIT
#define _CONST64_(x)	x ## L
#else
#define _CONST64_(x)	x ## LL
#endif
#endif

/* TLB refill registers */
#define LOONGARCH_CSR_TLBREBASE 0x88 /* TLB refill exception base address */
#define LOONGARCH_CSR_TLBRBADV 0x89 /* TLB refill badvaddr */
#define LOONGARCH_CSR_TLBREPC 0x8a /* TLB refill EPC */
#define LOONGARCH_CSR_TLBRSAVE 0x8b /* KScratch for TLB refill exception */
#define LOONGARCH_CSR_TLBRELO0 0x8c /* TLB refill entrylo0 */
#define LOONGARCH_CSR_TLBRELO1 0x8d /* TLB refill entrylo1 */
#define LOONGARCH_CSR_TLBREHI 0x8e /* TLB refill entryhi */
#define LOONGARCH_CSR_TLBRPRMD 0x8f /* TLB refill mode info */

#define DMW_PABITS	48
#define CSR_DMW1_PLV0		_CONST64_(1 << 0)
#define CSR_DMW1_MAT		_CONST64_(1 << 4)
#define CSR_DMW1_VSEG		_CONST64_(0x9000)
#define CSR_DMW1_BASE		(CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT		(CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

// copied from arch/loongarch/include/asm/regdef.h
#define zero $r0 /* wired zero */
#define ra $r1 /* return address */
#define tp $r2
#define sp $r3 /* stack pointer */
#define v0 $r4 /* return value - caller saved */
#define v1 $r5
#define a0 $r4 /* argument registers */
#define a1 $r5
#define a2 $r6
#define a3 $r7
#define a4 $r8
#define a5 $r9
#define a6 $r10
#define a7 $r11
#define t0 $r12 /* caller saved */
#define t1 $r13
#define t2 $r14
#define t3 $r15
#define t4 $r16
#define t5 $r17
#define t6 $r18
#define t7 $r19
#define t8 $r20
/* $r21: Temporarily reserved */
#define fp $r22 /* frame pointer */
#define s0 $r23 /* callee saved */
#define s1 $r24
#define s2 $r25
#define s3 $r26
#define s4 $r27
#define s5 $r28
#define s6 $r29
#define s7 $r30
#define s8 $r31

// 分配四个 page 来作为 ebase，其中 reset 放在第四个页面
//
// 6.1.1 中断一共 13 个, SWI0 的中断号为 0, SWI1 的中断号为 1
// 6.2.1 中断的 ecode 是 中断号 + 64
//
// (64 + 14) * vec_size
#define CSR_ECFG_VS_SHIFT 16
#define INSTRUCTION_LEN_IN_BYTES 4
#define INT_OFFSET 64
#define VEC_SIZE                                                               \
	(1 << (INIT_VALUE_ECFG >> CSR_ECFG_VS_SHIFT)) * INSTRUCTION_LEN_IN_BYTES
#define ERREBASE_OFFSET (PAGESIZE * 3)

#define EXCCODE_SYS 11 /* System call */

// TODO 需要将清空 EntryHi 的低位吗 ?
// TODO 岂不是 EntryHi 中间记录了的地址 和 TLBRBADV 的相同的
//
// TODO 最大支持的 TLB size 是什么 ?
// TODO TLBLO 是否设置为 dirty 位如何处理 ? (最好还是设置一下吧
// TODO 存储访问类型是什么 ?
//
// TODO 内核调试的策略，当 hyerpcall 不是 syscall 的源头的时候
// 一次性产生所有的数值，当 hypercall 是，打印所有的常规寄存器

#define CSR_TLBRELO_RPLV_SHIFT 63
#define CSR_TLBRELO_RPLV (_ULCAST_(0x1) << CSR_TLBRELO_RPLV_SHIFT)
#define CSR_TLBRELO_XI_SHIFT 62
#define CSR_TLBRELO_XI (_ULCAST_(0x1) << CSR_TLBRELO_XI_SHIFT)
#define CSR_TLBRELO_RI_SHIFT 61
#define CSR_TLBRELO_RI (_ULCAST_(0x1) << CSR_TLBRELO_RI_SHIFT)
#define CSR_TLBRELO_PFN_SHIFT 12
#define CSR_TLBRELO_PFN_WIDTH 36
#define CSR_TLBRELO_PFN (_ULCAST_(0xfffffffff) << CSR_TLBRELO_PFN_SHIFT)
#define CSR_TLBRELO_GLOBAL_SHIFT 6
#define CSR_TLBRELO_GLOBAL (_ULCAST_(0x1) << CSR_TLBRELO_GLOBAL_SHIFT)
#define CSR_TLBRELO_CCA_SHIFT 4
#define CSR_TLBRELO_CCA_WIDTH 2
#define CSR_TLBRELO_CCA (_ULCAST_(0x1) << CSR_TLBRELO_CCA_SHIFT)
#define CSR_TLBRELO_PLV_SHIFT 2
#define CSR_TLBRELO_PLV_WIDTH 2
#define CSR_TLBRELO_PLV (_ULCAST_(0x3) << CSR_TLBRELO_PLV_SHIFT)
#define CSR_TLBRELO_WE_SHIFT 1
#define CSR_TLBRELO_WE (_ULCAST_(0x1) << CSR_TLBRELO_WE_SHIFT)
#define CSR_TLBRELO_V_SHIFT 0
#define CSR_TLBRELO_V (_ULCAST_(0x1) << CSR_TLBRELO_V_SHIFT)

#define TLBRELO_STANDARD_BITS                                                  \
	(CSR_TLBRELO_V | CSR_TLBRELO_WE | CSR_TLBRELO_CCA | CSR_TLBRELO_GLOBAL)

// 512M 从 INIT_VALUE_PRCFG2 中获取
#define TLB_PS 29
#define MAX_TLB_SIZE (1 << TLB_PS)
#define TLB_MASK (MAX_TLB_SIZE - 1)

// ring 0, disable interrupt, mapping
#define CRMD_PG 4
#define INIT_VALUE_CRMD (1 << CRMD_PG)
// #define INIT_VALUE_PRMD 0x0

#define INIT_VALUE_EUEN 0x7
#define INIT_VALUE_MISC 0x0

// VS 指令间距是
// 屏蔽 IPI ，时钟，性能计数器 和 硬中断
#define INIT_VALUE_ECFG 0x70000

// 参考 6.2.1，中断的 ecode = 等于中断号 + 64
#define INIT_VALUE_RVACFG 0x0

// 1.8.2 这个字段是 host 设置，guest 只读的
// 虽然没有什么意义，将其设置为 kvm_cpu->cpu_id, 而不是 0
// #define INIT_VALUE_CPUNUM 0x0

// TODO 其实我一直都是非常的怀疑到底是否可以 SET 这些 config, 可以进行一些测试
#define INIT_VALUE_PRCFG1 0x72f8
// TLB 支持的页大小 [12, 29], 从 4k 到 512M 的大小
#define INIT_VALUE_PRCFG2 0x3ffff000
// TLB 的物理参数
#define INIT_VALUE_PRCFG3 0x8073f2

// 读取的 LLBCTL 总是 0, 猜测是因为多数情况下，LLBit 都不会被其他人清零，所以总是 0
#define INIT_VALUE_LLBCTL 0x1
#define INIT_VALUE_IMPCTL1 0x343c3
// #define INIT_VALUE_IMPCTL2 0x0

// #define INIT_VALUE_CTAG 0x0

#define INIT_VALUE_MCSR0 0x3f2f2fe0014c010
#define INIT_VALUE_MCSR1 0xfcff007ccfc7
#define INIT_VALUE_MCSR2 0x1000105f5e100
#define INIT_VALUE_MCSR3 0x7f33
#define INIT_VALUE_MCSR8 0x608000300002c3d
#define INIT_VALUE_MCSR9 0x608000f06080003
#define INIT_VALUE_MCSR10 0x60e000f
#define INIT_VALUE_MCSR24 0xe

#define INIT_VALUE_ERRCTL 0x0

// - [x] 为什么需要保存 exception 之前的状态 和 返回地址
// - [x] 为什么 TLB refill exception 需要单独保存 和 返回地址
// - [x] TLBRERA / TLBRPRMD 按道理来说都是只读信息才对啊
// - [x] 那些 TLB refill 在需要处理这些，那么，请问，走普通入口的需要这些吗 ?

// section 5.3 直接映射 和 访问类型还是存在关系的 !
// 当 MMU 处于直接翻译模式的时候，所有的指令都是按照 CRMD 决定的
// 当 MMU 处于非映射模拟，如果在直接映射窗口，那么按照窗口，否则按照通过页表项。
// 所以，使用一致缓存就好了
//
// - [x] 没有页表项怎么办? 页表项中间的 TLB 的 MAT 项目从页表项中间获取
//
// section 5.2 直接翻译模式 和 映射翻译模式:
//
// - [x] 在 TLB refill 的时候，会自动进入到 直接翻译模式 吗?
//  - 从 CRMD 的说明看，还是进入到映射模式，看来直接映射模式是给机器重启使用的
// - [x] 5.4 页表映射

// - [x] 之前都是没有区分 STLB 和 MTLB 的，为什么可以正常工作的啊?
//  - 因为填写的首先指定了大小, 然后可以自动忽视 STLB
// - [x] pagemask 的实现靠什么东西啊?
// - 为什么存在两个 PS : TLBHI 和 TLNINDEX
// - 如果是 TLB refill，那么在 TLBHI 中间处理
//
// - [x] CSR.TLBRERA.isTLBR 对于 TLB 指令的影响
// - [x] TLBWR / TLBFILL / TLBSRCH 分别的作用?

// TLBIDX 的作用
// - [x] 为什么需要 TLBSRCH 指令，在 TLB refill 的时候，这不是自动填写的吗 ?
// - 可能是一些我们不知道的需求吧!
//
// - TLBIDX bit 位置 NE 和 TLBRERA 的关系。
//
// 通过判断 TLBRERA 来区分到底是不是一个 TLB refill
// TLB refill 被特殊照顾
// 出错地址信息在 TLBRBADV 上

// - [x] 为什么需要设计成为两种 TLB 啊(历史原因)
//
// 内核的疑惑:
// 1. 按道理，应该是存在 K0 和 K1 这种寄存器, check kvm entry 相关的代码
//  - 实际上不需要，因为 caller save 的原因，有些寄存器实际上是可以随便使用的
// 2. TLS 相关的寄存器在哪里 ?
//  - 似乎 TLS 的代码就是放到 reg[2] 上的
//
// 1. LOONGARCH_CSR_EBASE : 0xc
// 2. LOONGARCH_CSR_TLBREBASE : 0x88
// 3. LOONGARCH_CSR_ERREBASE : 0x93
// ERREBASE 入口现在的内核并没有注册
//
// 关于实现:
// - build_tlb_refill_handler :  将代码拷贝到 refill_ebase 中间
// - configure_exception_vector : ebase 和 refill_ebase 写入到 csr 中间
// - trap_init : refill_ebase = ebase, 调用各种 handler 设置
//
// 从内核代码 和 寄存器数值分析，vector 的距离是 512
// 需要分配的空间是:
// ( size = (64 + 14) * vec_size;)
// 因为自身的中断号 + 14 啊
// 分配空间其实按照页对齐可以了，因为只是占据 512 byte 的
//
// TLB refill 的处理办法:
// 1. 使用了 TLBRSAVE 寄存器, 首位保存，所以没有 k0 和 k1 寄存器
// 2. 完全没有在乎 TLBRPRMD 之类的操作
// - [x] 为什么最后是 TLBWR 而不是 TLBFILL (这是纰漏)
// 这个指令是从 MIPS 时代就存在的，MIPS tlbwr 就是根据 random 寄存器填写的, 但是现在 TLBWR 是根据 index 来的
//
//
// PGDL 和 PGDH : 提供给 GPD 的两个地址
//
// PWCL 和 PWCH : 描述 pagewalk 的地址，虚拟地址用于在各个级别进行所以的位宽和开始范围
//
// badvaddr 从哪里找，取决于是否是 TLB refill exception 的
// PGD 的取值取决于是否出错的地址
//
// 分析 syscall 的处理方式
// - traps.c:trap_init 中间初始化
// - genex.S:handle_sys_wrap
// - 在 arch/loongarch/kernel/scall64-64.S 定义 syscall 的处理, 和常规函数感觉其实没有什么区别
//
// NESTED(handle_sys_wrap, 0, sp)
//
// 	la.abs	t0, handle_sys
// 	jirl    zero, t0, 0
// 	END(handle_sys_wrap)
//
// 秒啊，现在这些东西都可以搞定了:
// syscall 的参数和返回值 ? 从 glibc 中间拷贝吧!
// /home/maritns3/core/loongson-dune/la-glibc/sysdeps/unix/sysv/linux/loongarch/clone.S
// 
// TODO 从 arch/loongarch/kernel/genex.S 看，根本区分不了 la 和 la.abs
// 从 arch/loongarch/kernel/head.S 看，可以使用 dli 
//
// 至少 li 还是存在的, 

// 也许将 STLB 的大小映射为 1G ?
// 应该是不可以的
#define INIT_VALUE_STLBPS 0xE
#define INIT_VALUE_PWCTL0 0x5e56e
#define INIT_VALUE_PWCTL1 0x2e4

// 可以看看内核中间 LOONGARCH_CSR_TMID 的 reference
// FIXME 感觉内核中间把 timerid 当做 CPUID 来使用了
//
// 在现在的任务中间:
// kvm_vz_vcpu_setup 将 timerid 初始化为 `vcpu->vcpu_id`
//
// ioctl(vcpu->vm->vm_fd, KVM_CREATE_VCPU, vcpu->cpu_id);
// 就是 vcpu_id 啊

#define INIT_VALUE_TIMERCFG 0x0

// CPUCFG 知道一共存在 4 个，从内核的定义来看，也是如此的
#define INIT_VALUE_PERF0_EVENT 0x0
#define INIT_VALUE_PERF1_EVENT 0x0
#define INIT_VALUE_PERF2_EVENT 0x0
#define INIT_VALUE_PERF3_EVENT 0x0

#define INIT_VALUE_DEBUG 0x0

// GTLBC 来控制 guest MTLB 的数量，只是为了调节性能参数吧
// hvcl 指令 : hyerpcall
#define INVALID_CODEFLOW_1 .word(0x00298000 | (0x1))
#define INVALID_CODEFLOW_2 .word(0x00298000 | (0x2))
#define INVALID_CODEFLOW_3 .word(0x00298000 | (0x3))
#define INVALID_EBASE_POSITION 0x4
#define UNIMP_ERROR .word(0x00298000 | (0x5))
#define HYPERCALL .word 0x00298000

#endif /* end of include guard: INTERNAL_H_6IUWCEFP */
