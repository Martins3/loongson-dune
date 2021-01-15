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

// 当前思路:
// - 可以不会触发 ri xi, 但是什么叫做采用不同的入口
//   - ebase 入口设计，现在的想法是，建立一个 memslot，将 gebase 放到 physical 的位置
#define PAGEGRAIN_RIE 31
#define PAGEGRAIN_XIE 30
#define PAGEGRAIN_ELPA 29
#define PAGEGRAIN_ESP 28
#define PAGEGRAIN_IEC 27
#define INIT_VALUE_PAGEGRAIN                                                       \
	PAGEGRAIN_RIE | PAGEGRAIN_XIE | PAGEGRAIN_ELPA | PAGEGRAIN_IEC

#define INIT_VALUE_USERLOCAL 0
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
#define STATUS_BIT_UX (1 << 5)
#define STATUS_BIT_SX (1 << 6)
#define STATUS_BIT_KX (1 << 7)
#define STATUS_BIT_FR (1 << 26)
#define STATUS_BIT_CP0 (1 << 28)
#define STATUS_BIT_MM (1 << 30)
#define INIT_VALUE_STATUS STATUS_BIT_UX | STATUS_BIT_FR |STATUS_BIT_CP0 |STATUS_BIT_MM
// TODO how interrupt works ?
// TODO what's meaning of EIC and VI ?
#define INIT_VALUE_INTCTL 0xfc000100
#define INIT_VALUE_CAUSE 0
#define INIT_VALUE_EPC 0

#define INIT_VALUE_PRID 0x14c004

// TODO 显然操作系统是没有对于 config 进行初始化的 ?
// 所以应该不用进行初始化
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

#define INIT_VALUE_KSCRATCH1 0
#define INIT_VALUE_KSCRATCH2 0
#define INIT_VALUE_KSCRATCH3 0
#define INIT_VALUE_KSCRATCH4 0
#define INIT_VALUE_KSCRATCH5 0
#define INIT_VALUE_KSCRATCH6 0
