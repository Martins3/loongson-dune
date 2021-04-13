/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2020  Loongson Technologies, Inc.  All rights reserved.
 * Authors: Sanjay Lal <sanjayl@kymasys.com>
 * Authors: Xing Li <lixing@loongson.cn>
 */

#ifndef __LINUX_KVM_LOONGARCH_H
#define __LINUX_KVM_LOONGARCH_H

#include <linux/types.h>

#define __KVM_HAVE_GUEST_DEBUG
#define KVM_GUESTDBG_USE_SW_BP 0x00010000

/*
 * KVM Loongarch specific structures and definitions.
 *
 * Some parts derived from the x86 version of this file.
 */

#define __KVM_HAVE_READONLY_MEM

#define KVM_COALESCED_MMIO_PAGE_OFFSET 1

/*
 * for KVM_GET_REGS and KVM_SET_REGS
 */
struct kvm_regs {
	/* out (KVM_GET_REGS) / in (KVM_SET_REGS) */
	__u64 gpr[32];
	__u64 pc;
};

/*
 * for KVM_GET_CPUCFG
 */
struct kvm_cpucfg {
	/* out (KVM_GET_CPUCFG) */
	__u32 cpucfg[64];
};

/*
 * for KVM_GET_FPU and KVM_SET_FPU
 */
struct kvm_fpu {
};


/*
 * For LOONGARCH, we use KVM_SET_ONE_REG and KVM_GET_ONE_REG to access various
 * registers.  The id field is broken down as follows:
 *
 *  bits[63..52] - As per linux/kvm.h
 *  bits[51..32] - Must be zero.
 *  bits[31..16] - Register set.
 *
 * Register set = 0: GP registers from kvm_regs (see definitions below).
 *
 * Register set = 1: CP0 registers.
 *  bits[15..8]  - COP0 register set.
 *
 *  COP0 register set = 0: Main CP0 registers.
 *   bits[7..3]   - Register 'rd'  index.
 *   bits[2..0]   - Register 'sel' index.
 *
 *  COP0 register set = 1: MAARs.
 *   bits[7..0]   - MAAR index.
 *
 * Register set = 2: KVM specific registers (see definitions below).
 *
 * Register set = 3: FPU / SIMD registers (see definitions below).
 *
 * Other sets registers may be added in the future.  Each set would
 * have its own identifier in bits[31..16].
 */

#define KVM_REG_LOONGARCH_GP		(KVM_REG_LOONGARCH | 0x0000000000000000ULL)
#define KVM_REG_LOONGARCH_CSR	(KVM_REG_LOONGARCH | 0x0000000000010000ULL)
#define KVM_REG_LOONGARCH_KVM	(KVM_REG_LOONGARCH | 0x0000000000020000ULL)
#define KVM_REG_LOONGARCH_FPU	(KVM_REG_LOONGARCH | 0x0000000000030000ULL)


/*
 * KVM_REG_LOONGARCH_GP - General purpose registers from kvm_regs.
 */

#define KVM_REG_LOONGARCH_R0		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  0)
#define KVM_REG_LOONGARCH_R1		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  1)
#define KVM_REG_LOONGARCH_R2		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  2)
#define KVM_REG_LOONGARCH_R3		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  3)
#define KVM_REG_LOONGARCH_R4		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  4)
#define KVM_REG_LOONGARCH_R5		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  5)
#define KVM_REG_LOONGARCH_R6		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  6)
#define KVM_REG_LOONGARCH_R7		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  7)
#define KVM_REG_LOONGARCH_R8		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  8)
#define KVM_REG_LOONGARCH_R9		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 |  9)
#define KVM_REG_LOONGARCH_R10	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 10)
#define KVM_REG_LOONGARCH_R11	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 11)
#define KVM_REG_LOONGARCH_R12	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 12)
#define KVM_REG_LOONGARCH_R13	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 13)
#define KVM_REG_LOONGARCH_R14	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 14)
#define KVM_REG_LOONGARCH_R15	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 15)
#define KVM_REG_LOONGARCH_R16	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 16)
#define KVM_REG_LOONGARCH_R17	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 17)
#define KVM_REG_LOONGARCH_R18	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 18)
#define KVM_REG_LOONGARCH_R19	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 19)
#define KVM_REG_LOONGARCH_R20	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 20)
#define KVM_REG_LOONGARCH_R21	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 21)
#define KVM_REG_LOONGARCH_R22	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 22)
#define KVM_REG_LOONGARCH_R23	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 23)
#define KVM_REG_LOONGARCH_R24	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 24)
#define KVM_REG_LOONGARCH_R25	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 25)
#define KVM_REG_LOONGARCH_R26	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 26)
#define KVM_REG_LOONGARCH_R27	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 27)
#define KVM_REG_LOONGARCH_R28	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 28)
#define KVM_REG_LOONGARCH_R29	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 29)
#define KVM_REG_LOONGARCH_R30	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 30)
#define KVM_REG_LOONGARCH_R31	(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 31)

#define KVM_REG_LOONGARCH_HI		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 32)
#define KVM_REG_LOONGARCH_LO		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 33)
#define KVM_REG_LOONGARCH_PC		(KVM_REG_LOONGARCH_GP | KVM_REG_SIZE_U64 | 34)


/*
 * KVM_REG_LOONGARCH_KVM - KVM specific control registers.
 */

/*
 * CP0_Count control
 * DC:    Set 0: Master disable CP0_Count and set COUNT_RESUME to now
 *        Set 1: Master re-enable CP0_Count with unchanged bias, handling timer
 *               interrupts since COUNT_RESUME
 *        This can be used to freeze the timer to get a consistent snapshot of
 *        the CP0_Count and timer interrupt pending state, while also resuming
 *        safely without losing time or guest timer interrupts.
 * Other: Reserved, do not change.
 */
#define KVM_REG_LOONGARCH_COUNT_CTL	    (KVM_REG_LOONGARCH_KVM | KVM_REG_SIZE_U64 | 0)
#define KVM_REG_LOONGARCH_COUNT_CTL_DC	0x00000001

/*
 * CP0_Count resume monotonic nanoseconds
 * The monotonic nanosecond time of the last set of COUNT_CTL.DC (master
 * disable). Any reads and writes of Count related registers while
 * COUNT_CTL.DC=1 will appear to occur at this time. When COUNT_CTL.DC is
 * cleared again (master enable) any timer interrupts since this time will be
 * emulated.
 * Modifications to times in the future are rejected.
 */
#define KVM_REG_LOONGARCH_COUNT_RESUME   (KVM_REG_LOONGARCH_KVM | KVM_REG_SIZE_U64 | 1)
/*
 * CP0_Count rate in Hz
 * Specifies the rate of the CP0_Count timer in Hz. Modifications occur without
 * discontinuities in CP0_Count.
 */
#define KVM_REG_LOONGARCH_COUNT_HZ	    (KVM_REG_LOONGARCH_KVM | KVM_REG_SIZE_U64 | 2)

#define KVM_REG_LOONGARCH_COUNTER    (KVM_REG_LOONGARCH_KVM | KVM_REG_SIZE_U64 | 3)

/*
 * KVM_REG_LOONGARCH_FPU - Floating Point and SIMD registers.
 *
 *  bits[15..8]  - Register subset (see definitions below).
 *  bits[7..5]   - Must be zero.
 *  bits[4..0]   - Register number within register subset.
 */

#define KVM_REG_LOONGARCH_FPR	(KVM_REG_LOONGARCH_FPU | 0x0000000000000000ULL)
#define KVM_REG_LOONGARCH_FCR	(KVM_REG_LOONGARCH_FPU | 0x0000000000000100ULL)
#define KVM_REG_LOONGARCH_VCR	(KVM_REG_LOONGARCH_FPU | 0x0000000000000200ULL)
#define KVM_REG_LOONGARCH_FCC	(KVM_REG_LOONGARCH_FPU | 0x0000000000000400ULL)

/*
 * KVM_REG_LOONGARCH_FPR - Floating point / Vector registers.
 */
#define KVM_REG_LOONGARCH_FPR_32(n)	(KVM_REG_LOONGARCH_FPR | KVM_REG_SIZE_U32  | (n))
#define KVM_REG_LOONGARCH_FPR_64(n)	(KVM_REG_LOONGARCH_FPR | KVM_REG_SIZE_U64  | (n))
#define KVM_REG_LOONGARCH_VEC_128(n)	(KVM_REG_LOONGARCH_FPR | KVM_REG_SIZE_U128 | (n))
#define KVM_REG_LOONGARCH_VEC_256(n)	(KVM_REG_LOONGARCH_FPR | KVM_REG_SIZE_U256 | (n))

/*
 * KVM_REG_LOONGARCH_FCR - Floating point control registers.
 */
#define KVM_REG_LOONGARCH_FCR_IR	(KVM_REG_LOONGARCH_FCR | KVM_REG_SIZE_U32 |  0)
#define KVM_REG_LOONGARCH_FCR_CSR	(KVM_REG_LOONGARCH_FCR | KVM_REG_SIZE_U32 | 31)

/*
 * KVM_REG_LOONGARCH_VCR - Vector (SIMD) control registers.
 */
#define KVM_REG_LOONGARCH_VIR	 (KVM_REG_LOONGARCH_VCR | KVM_REG_SIZE_U32 |  0)
#define KVM_REG_LOONGARCH_VCSR	 (KVM_REG_LOONGARCH_VCR | KVM_REG_SIZE_U32 |  16)

/*
 * KVM_REG_LOONGARCH_FCCR - LOONGARCH SIMD Architecture (LSX) control registers.
 */
#define KVM_REG_LOONGARCH_FCCR	 (KVM_REG_LOONGARCH_FCC | KVM_REG_SIZE_U64 |  0)


#define __KVM_HAVE_IRQ_LINE

struct kvm_debug_exit_arch {
	__u64 epc;
};

/* for KVM_SET_GUEST_DEBUG */
struct kvm_guest_debug_arch {
};

/* definition of registers in kvm_run */
struct kvm_sync_regs {
};

/* dummy definition */
struct kvm_sregs {
};

struct kvm_loongarch_interrupt {
	/* in */
	__u32 cpu;
	__u32 irq;
};

#define KVM_IRQCHIP_LS7A_IOAPIC	0x0
#define KVM_IRQCHIP_LS3A_GIPI	0x1
#define KVM_IRQCHIP_LS3A_HT_IRQ	0x2
#define KVM_IRQCHIP_LS3A_ROUTE	0x3
#define KVM_IRQCHIP_LS3A_EXTIRQ	0x4
#define KVM_IRQCHIP_LS3A_IPMASK	0x5
#define KVM_NR_IRQCHIPS          1
#define KVM_IRQCHIP_NUM_PINS    64

struct ls7a_ioapic_state {
	/* 0x000 interrupt id register */
	__u64 int_id;
	/* 0x020 interrupt mask register */
	__u64 int_mask;
	/* 0x040 1=msi */
	__u64 htmsi_en;
	/* 0x060 edge=1 level  =0 */
	__u64 intedge;
	/* 0x080 for clean edge int,set 1 clean,set 0 is noused */
	__u64 intclr;
	/* 0x0c0 */
	__u64 auto_crtl0;
	/* 0x0e0 */
	__u64 auto_crtl1;
	/* 0x100 - 0x140 */
	__u8 route_entry[64];
	/* 0x200 - 0x240 */
	__u8 htmsi_vector[64];
	/* 0x300 */
	__u64 intisr_chip0;
	/* 0x320 */
	__u64 intisr_chip1;
	/* edge detection */
	__u64 last_intirr;
	/* 0x380 interrupt request register */
	__u64 intirr;
	/* 0x3a0 interrupt service register */
	__u64 intisr;
	/* 0x3e0 interrupt level polarity selection register,
	 * 0 for high level tirgger
	 */
	__u64 int_polarity;
};

struct loongarch_gipi_single {
	__u32 status;
	__u32 en;
	__u32 set;
	__u32 clear;
	__u64 buf[4];
};

struct loongarch_gipiState {
	struct loongarch_gipi_single core[16];
};

#define KVM_LS3A_NODES			4
#define KVM_MAX_CORES			16
#define KVM_EXTIOI_IRQS			(256)
#define KVM_EXTIOI_IRQS_BITMAP_SIZE	(KVM_EXTIOI_IRQS / 8)
/* map to ipnum per 32 irqs */
#define KVM_EXTIOI_IRQS_IPMAP_SIZE	(KVM_EXTIOI_IRQS / 32)
#define KVM_EXTIOI_IRQS_PER_GROUP	32
#define KVM_EXTIOI_IRQS_COREMAP_SIZE	(KVM_EXTIOI_IRQS)
#define KVM_EXTIOI_IRQS_NODETYPE_SIZE	16

struct kvm_loongarch_ls3a_extirq_state {
	union ext_en_r {
		__u64 reg_u64[KVM_EXTIOI_IRQS_BITMAP_SIZE / 8];
		__u32 reg_u32[KVM_EXTIOI_IRQS_BITMAP_SIZE / 4];
		__u8 reg_u8[KVM_EXTIOI_IRQS_BITMAP_SIZE];
	} ext_en_r;
	union bounce_r {
		__u64 reg_u64[KVM_EXTIOI_IRQS_BITMAP_SIZE / 8];
		__u32 reg_u32[KVM_EXTIOI_IRQS_BITMAP_SIZE / 4];
		__u8 reg_u8[KVM_EXTIOI_IRQS_BITMAP_SIZE];
	} bounce_r;
	union ext_isr_r {
		__u64 reg_u64[KVM_EXTIOI_IRQS_BITMAP_SIZE / 8];
		__u32 reg_u32[KVM_EXTIOI_IRQS_BITMAP_SIZE / 4];
		__u8 reg_u8[KVM_EXTIOI_IRQS_BITMAP_SIZE];
	} ext_isr_r;
	union ext_core_isr_r {
		__u64 reg_u64[KVM_MAX_CORES][KVM_EXTIOI_IRQS_BITMAP_SIZE / 8];
		__u32 reg_u32[KVM_MAX_CORES][KVM_EXTIOI_IRQS_BITMAP_SIZE / 4];
		__u8 reg_u8[KVM_MAX_CORES][KVM_EXTIOI_IRQS_BITMAP_SIZE];
	} ext_core_isr_r;
	union ip_map_r {
		__u64 reg_u64;
		__u32 reg_u32[KVM_EXTIOI_IRQS_IPMAP_SIZE / 4];
		__u8 reg_u8[KVM_EXTIOI_IRQS_IPMAP_SIZE];
	} ip_map_r;
	union core_map_r {
		__u64 reg_u64[KVM_LS3A_NODES][KVM_EXTIOI_IRQS_COREMAP_SIZE / 8];
		__u32 reg_u32[KVM_LS3A_NODES][KVM_EXTIOI_IRQS_COREMAP_SIZE / 4];
		__u8 reg_u8[KVM_LS3A_NODES][KVM_EXTIOI_IRQS_COREMAP_SIZE];
	} core_map_r;
	union node_type_r {
		__u64 reg_u64[KVM_EXTIOI_IRQS_NODETYPE_SIZE / 4];
		__u32 reg_u32[KVM_EXTIOI_IRQS_NODETYPE_SIZE / 2];
		__u16 reg_u16[KVM_EXTIOI_IRQS_NODETYPE_SIZE];
		__u8 reg_u8[KVM_EXTIOI_IRQS_NODETYPE_SIZE * 2];
	} node_type_r;
};

struct loongarch_kvm_irqchip {
	__u32 chip_id;
	__u32 node;
	union {
		__u8 dummy[2048];  /* reserving space */
		struct ls7a_ioapic_state ls7a_ioapic;
		struct loongarch_gipiState ls3a_gipistate;
		__u8 ht_irq_reg[0x100];
		__u8 ls3a_router_reg[0x100];
		struct kvm_loongarch_ls3a_extirq_state ls3a_extirq;
		__u16 core_ipmask[128][4];
	} chip;
};

#endif /* __LINUX_KVM_LOONGARCH_H */
