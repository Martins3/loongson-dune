/* MIPS KVM register ids */
#define MIPS_CP0_32(_R, _S)					\
	(KVM_REG_MIPS_CP0 | KVM_REG_SIZE_U32 | (8 * (_R) + (_S)))

#define MIPS_CP0_64(_R, _S)					\
	(KVM_REG_MIPS_CP0 | KVM_REG_SIZE_U64 | (8 * (_R) + (_S)))

#define KVM_REG_MIPS_CP0_INDEX		MIPS_CP0_32(0, 0)
#define KVM_REG_MIPS_CP0_RANDOM		MIPS_CP0_32(1, 0)
#define KVM_REG_MIPS_CP0_ENTRYLO0	MIPS_CP0_64(2, 0)
#define KVM_REG_MIPS_CP0_ENTRYLO1	MIPS_CP0_64(3, 0)
#define KVM_REG_MIPS_CP0_CONTEXT	MIPS_CP0_64(4, 0)
#define KVM_REG_MIPS_CP0_CONTEXTCONFIG	MIPS_CP0_32(4, 1)
#define KVM_REG_MIPS_CP0_USERLOCAL	MIPS_CP0_64(4, 2)
#define KVM_REG_MIPS_CP0_XCONTEXTCONFIG	MIPS_CP0_64(4, 3)
#define KVM_REG_MIPS_CP0_PAGEMASK	MIPS_CP0_32(5, 0)
#define KVM_REG_MIPS_CP0_PAGEGRAIN	MIPS_CP0_32(5, 1)
#define KVM_REG_MIPS_CP0_SEGCTL0	MIPS_CP0_64(5, 2)
#define KVM_REG_MIPS_CP0_SEGCTL1	MIPS_CP0_64(5, 3)
#define KVM_REG_MIPS_CP0_SEGCTL2	MIPS_CP0_64(5, 4)
#define KVM_REG_MIPS_CP0_PWBASE		MIPS_CP0_64(5, 5)
#define KVM_REG_MIPS_CP0_PWFIELD	MIPS_CP0_64(5, 6)
#define KVM_REG_MIPS_CP0_PWSIZE		MIPS_CP0_64(5, 7)
#define KVM_REG_MIPS_CP0_WIRED		MIPS_CP0_32(6, 0)
#define KVM_REG_MIPS_CP0_PWCTL		MIPS_CP0_32(6, 6)
#define KVM_REG_MIPS_CP0_HWRENA		MIPS_CP0_32(7, 0)
#define KVM_REG_MIPS_CP0_BADVADDR	MIPS_CP0_64(8, 0)
#define KVM_REG_MIPS_CP0_BADINSTR	MIPS_CP0_32(8, 1)
#define KVM_REG_MIPS_CP0_BADINSTRP	MIPS_CP0_32(8, 2)
#define KVM_REG_MIPS_CP0_COUNT		MIPS_CP0_32(9, 0)
#define KVM_REG_MIPS_CP0_ENTRYHI	MIPS_CP0_64(10, 0)
#define KVM_REG_MIPS_CP0_COMPARE	MIPS_CP0_32(11, 0)
#define KVM_REG_MIPS_CP0_STATUS		MIPS_CP0_32(12, 0)
#define KVM_REG_MIPS_CP0_INTCTL		MIPS_CP0_32(12, 1)
#define KVM_REG_MIPS_CP0_CAUSE		MIPS_CP0_32(13, 0)
#define KVM_REG_MIPS_CP0_EPC		MIPS_CP0_64(14, 0)
#define KVM_REG_MIPS_CP0_PRID		MIPS_CP0_32(15, 0)
#define KVM_REG_MIPS_CP0_EBASE		MIPS_CP0_64(15, 1)
#define KVM_REG_MIPS_CP0_CONFIG		MIPS_CP0_32(16, 0)
#define KVM_REG_MIPS_CP0_CONFIG1	MIPS_CP0_32(16, 1)
#define KVM_REG_MIPS_CP0_CONFIG2	MIPS_CP0_32(16, 2)
#define KVM_REG_MIPS_CP0_CONFIG3	MIPS_CP0_32(16, 3)
#define KVM_REG_MIPS_CP0_CONFIG4	MIPS_CP0_32(16, 4)
#define KVM_REG_MIPS_CP0_CONFIG5	MIPS_CP0_32(16, 5)
#define KVM_REG_MIPS_CP0_CONFIG6	MIPS_CP0_32(16, 6)
#define KVM_REG_MIPS_CP0_CONFIG7	MIPS_CP0_64(16, 7)
#define KVM_REG_MIPS_CP0_MAARI		MIPS_CP0_64(17, 2)
#define KVM_REG_MIPS_CP0_XCONTEXT	MIPS_CP0_64(20, 0)
#define KVM_REG_MIPS_CP0_GSCAUSE	MIPS_CP0_64(22, 1)
#define KVM_REG_MIPS_CP0_ERROREPC	MIPS_CP0_64(30, 0)
#define KVM_REG_MIPS_CP0_KSCRATCH1	MIPS_CP0_64(31, 2)
#define KVM_REG_MIPS_CP0_KSCRATCH2	MIPS_CP0_64(31, 3)
#define KVM_REG_MIPS_CP0_KSCRATCH3	MIPS_CP0_64(31, 4)
#define KVM_REG_MIPS_CP0_KSCRATCH4	MIPS_CP0_64(31, 5)
#define KVM_REG_MIPS_CP0_KSCRATCH5	MIPS_CP0_64(31, 6)
#define KVM_REG_MIPS_CP0_KSCRATCH6	MIPS_CP0_64(31, 7)
