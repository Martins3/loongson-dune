#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "arch.h"
#include "internal.h"
#include "../interface.h"

#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>

void arch_dump_regs(int debug_fd, struct kvm_regs regs)
{
	dprintf(debug_fd, "\n Registers:\n");
	dprintf(debug_fd, " ----------\n");
	dprintf(debug_fd, "$0   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[0],
		(unsigned long long)regs.gpr[1],
		(unsigned long long)regs.gpr[2],
		(unsigned long long)regs.gpr[3]);
	dprintf(debug_fd, "$4   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[4],
		(unsigned long long)regs.gpr[5],
		(unsigned long long)regs.gpr[6],
		(unsigned long long)regs.gpr[7]);
	dprintf(debug_fd, "$8   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[8],
		(unsigned long long)regs.gpr[9],
		(unsigned long long)regs.gpr[10],
		(unsigned long long)regs.gpr[11]);
	dprintf(debug_fd, "$12  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[12],
		(unsigned long long)regs.gpr[13],
		(unsigned long long)regs.gpr[14],
		(unsigned long long)regs.gpr[15]);
	dprintf(debug_fd, "$16  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[16],
		(unsigned long long)regs.gpr[17],
		(unsigned long long)regs.gpr[18],
		(unsigned long long)regs.gpr[19]);
	dprintf(debug_fd, "$20  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[20],
		(unsigned long long)regs.gpr[21],
		(unsigned long long)regs.gpr[22],
		(unsigned long long)regs.gpr[23]);
	dprintf(debug_fd, "$24  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[24],
		(unsigned long long)regs.gpr[25],
		(unsigned long long)regs.gpr[26],
		(unsigned long long)regs.gpr[27]);
	dprintf(debug_fd, "$28  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[28],
		(unsigned long long)regs.gpr[29],
		(unsigned long long)regs.gpr[30],
		(unsigned long long)regs.gpr[31]);

	dprintf(debug_fd, "pc  : %016llx\n", (unsigned long long)regs.pc);

	dprintf(debug_fd, "\n");
}

enum ACCESS_OP {
	GET = KVM_GET_ONE_REG,
	SET = KVM_SET_ONE_REG,
};

static void kvm_access_reg(const struct kvm_cpu *cpu, struct kvm_one_reg *reg,
			   enum ACCESS_OP op)
{
	if (ioctl(cpu->vcpu_fd, op, reg) < 0)
		die("kvm_access_reg");
}

static u64 kvm_access_csr_reg(const struct kvm_cpu *cpu, u64 id,
			      enum ACCESS_OP op, u64 value)
{
	struct kvm_one_reg reg;
	u64 v = (op == GET) ? 0 : value;
	reg.addr = (u64) & (v);
	reg.id = id;

	kvm_access_reg(cpu, &reg, op);
	return v;
}

static u64 kvm_get_csr_reg(const struct kvm_cpu *cpu, u64 id)
{
	return kvm_access_csr_reg(cpu, id, GET, 0);
}

static void kvm_set_csr_reg(const struct kvm_cpu *cpu, u64 id, u64 v)
{
	kvm_access_csr_reg(cpu, id, SET, v);
}

/*

1. 三种寄存器还是保持重叠的
2. 一共 32 个
- [ ] 如何 enable fpu
- [ ] 保存和设置 ?
/home/maritns3/core/loongson-dune/la-4.19/arch/loongarch/kvm/fpu.S
- [ ] lasx lsx fpu 还是可以保存最大的哪一个吗 ?

kvm_loongarch_guest_has_fpu : 
- [ ] KVM_REG_LOONGARCH_FCR_IR / KVM_REG_LOONGARCH_FCR_CSR / KVM_REG_LOONGARCH_FCCR 的定义 ?

- 3.1.3 : IR 只读, CSR : 浮点 eflags, FCCR : 控制寄存器 

- [ ] lsx 和 lasx 是否存在对应的控制器 ?
- [ ] KVM_REG_LOONGARCH_VIR

KVM_REG_LOONGARCH_VCSR
*/

static void kvm_enable_fpu(struct kvm_cpu *cpu)
{
	struct kvm_enable_cap cap;
	memset(&cap, 0, sizeof(cap));
	cap.cap = KVM_CAP_LOONGARCH_FPU;

	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		die("Unable enable fpu in guest");
	}

	cap.cap = KVM_CAP_LOONGARCH_LSX;
	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		die("Unable enable msa in guest");
	}

	// 从 kvm_vcpu_ioctl_enable_cap 可以看到不需要手动打开 lasx
}

static void kvm_access_fpu_regs(struct kvm_cpu *cpu,
        const struct loongarch_fpu_struct *fpu_regs,
        enum ACCESS_OP op)
{
  struct kvm_one_reg reg;

  for (int i = 0; i < NUM_FPU_REGS; ++i) {
    reg.id = KVM_REG_LOONGARCH_VEC_256(i);
    reg.addr = (u64) & (fpu_regs->fpr[i]);
    kvm_access_reg(cpu, &reg, op);
  }

  reg.id = KVM_REG_LOONGARCH_FCR_CSR;
  reg.addr = (u64) & (fpu_regs->fcsr);
  kvm_access_reg(cpu, &reg, op);

  reg.id = KVM_REG_LOONGARCH_VCSR;
  reg.addr = (u64) & (fpu_regs->vcsr);
  kvm_access_reg(cpu, &reg, op);

  // MIPS 中只是设置了两个数值 FCR_CSR 和 VCSR
  reg.id = KVM_REG_LOONGARCH_FCCR;
  reg.addr = (u64) & (fpu_regs->fcc);
  kvm_access_reg(cpu, &reg, op);
}

static void kvm_get_fpu_regs(struct kvm_cpu *cpu,
           const struct loongarch_fpu_struct *fpu_regs)
{
  kvm_access_fpu_regs(cpu, fpu_regs, GET);
}

static void kvm_set_fpu_regs(struct kvm_cpu *cpu,
           const struct loongarch_fpu_struct *fpu_regs)
{
  kvm_access_fpu_regs(cpu, fpu_regs, SET);
}

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(parent_cpu->info.regs)) <
	    0)
		die("KVM_GET_REGS");

	parent_cpu->info.era =
		kvm_get_csr_reg(parent_cpu, KVM_LOONGARCH_CSR_EPC);

	kvm_get_fpu_regs(parent_cpu, &parent_cpu->info.fpu);
}

static void init_fpu(struct kvm_cpu *cpu)
{
	kvm_enable_fpu(cpu);

	struct loongarch_fpu_struct fpu_regs;

	extern void get_fpu_regs(struct loongarch_fpu_struct *);
	get_fpu_regs(&fpu_regs);

	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fcsr) == VCPU_FCSR0);
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, vcsr) == VCPU_VCSR);
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fcc) == VCPU_FCC);
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fpr[0]) == VCPU_FPR0);
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fpr[31]) == VCPU_FPR0 + 31 * VCPU_FPR_LEN);

  kvm_set_fpu_regs(cpu, &fpu_regs);
}

static void init_ebase(struct kvm_cpu *cpu)
{
	BUILD_ASSERT(512 == VEC_SIZE);
	BUILD_ASSERT(INT_OFFSET * VEC_SIZE == PAGESIZE * 2);
	BUILD_ASSERT(VEC_SIZE * 14 < PAGESIZE);

	cpu->info.ebase = mmap_pages(4);
	for (int i = 0; i < PAGESIZE; ++i) {
		int *x = (int *)cpu->info.ebase;
		x = x + i;
		*x = (0x00298000 | INVALID_EBASE_POSITION);
	}

	pr_info("ebase address : %llx", cpu->info.ebase);

	extern void err_entry_begin(void);
	extern void err_entry_end(void);
	extern void tlb_refill_entry_begin(void);
	extern void tlb_refill_entry_end(void);
	extern void syscall_entry_begin(void);
	extern void syscall_entry_end(void);

	memcpy(cpu->info.ebase, tlb_refill_entry_begin,
	       tlb_refill_entry_end - tlb_refill_entry_begin);
	memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_SYS, syscall_entry_begin,
	       syscall_entry_end - syscall_entry_begin);
	memcpy(cpu->info.ebase + ERREBASE_OFFSET, err_entry_begin,
	       err_entry_end - err_entry_begin);
}

struct csr_reg {
	struct kvm_one_reg reg;
	char name[100];
	u64 v;
};

#define CSR_INIT_REG(X)                                                        \
	{                                                                      \
		.reg = { .id = KVM_CSR_##X }, .name = #X, .v = INIT_VALUE_##X  \
	}

static void init_csr(struct kvm_cpu *cpu)
{
	if (!cpu->info.ebase)
		die("You forget to init ebase");

	u64 INIT_VALUE_DMWIN1 = CSR_DMW1_INIT;
	u64 INIT_VALUE_KSCRATCH6 = (u64)cpu->syscall_parameter + CSR_DMW1_BASE;
	u64 INIT_VALUE_KSCRATCH7 = TLBRELO0_STANDARD_BITS;
	u64 INIT_VALUE_KSCRATCH8 = TLBRELO1_STANDARD_BITS;

	u64 INIT_VALUE_TLBREBASE = (u64)cpu->info.ebase;
	u64 INIT_VALUE_EBASE = (u64)cpu->info.ebase;

	u64 INIT_VALUE_CPUNUM = cpu->cpu_id;

	u64 INIT_VALUE_ERREBASE = (u64)cpu->info.ebase + ERREBASE_OFFSET;

	// 在 arch/loongarch/include/asm/loongarchregs.h
	// 中间定义了一堆 IOCSR 寄存器，实际上完全不知道如何使用
	// - [x] 如果不正确设置 IOCSR，其影响是什么?
	//  - 这个外设本身就不是我们管理的
	// - [x] LOONGARCH_CSR_TMID 和 LOONGARCH_IOCSR_TIMER_CFG 的关系是什么?
	//  - 应该是 node 的时钟 和 本地时钟的关系吧
	//
	// IOCSR 只是用于各种地址空间的。

	u64 gg = KVM_CSR_CRMD;
	struct csr_reg one_regs[] = {
		CSR_INIT_REG(CRMD),
		// CSR_INIT_REG(PRMD),
		CSR_INIT_REG(EUEN), CSR_INIT_REG(MISC), CSR_INIT_REG(ECFG),
		// CSR_INIT_REG(ESTAT),
		// CSR_INIT_REG(EPC),
		// CSR_INIT_REG(BADV),
		// CSR_INIT_REG(BADI),
		CSR_INIT_REG(EBASE),
		// CSR_INIT_REG(TLBIDX),
		// CSR_INIT_REG(TLBHI),
		// CSR_INIT_REG(TLBLO0),
		// CSR_INIT_REG(TLBLO1),
		//
		// CSR_INIT_REG(GTLBC), gcsr
		// CSR_INIT_REG(TRGP),  gcsr
		//
		// 实际上，基本相同:
		// 进行 gpa 到 gpa 之间的映射，一致都是 kvm 在维护的
		// kvm->arch.gpa_mm.pgd
		// 实际上，根本找不到在什么地方进行了 TLB refill 的分析
		//
		//
		// 检查 kvm 的源代码，分析是如何处理 inctl 的, 中断直接到达的，还是 ?
		// 在内核中间搜索 ginct 的
		// 1. 设置 KVM_CSR_ESTAT 的作用:
		//
		// 合乎逻辑
		// case KVM_CSR_ESTAT:
		// write_gcsr_estat(v);
		// write_csr_gintc((v & 0x3fc) >> 2);
		//
		// 2. 在 vcpu_load 和 vcpu_set 的时候存在一些作用
		//  - kvm_loongarch_deliver_interrupts
		//
		// 分析完成 kvm_loongarch_deliver_interrupts 的调用之后,
		// 目前的 kvm 实际上并没有用上这些的高级功能，只是可以通过 ioctl 将中断注入到
		// 其中的方法。
		//
		// 重新阅读 entry.S 的代码 : 一些汇编，比以前的代码算是清晰很多了
		//
		// 和 VMM 相关的例外 : TLB refill 以及各种类型的
		// 手册中间，描述 vm exit 的种类 ? 1.3
		//
		// load / store 监视
		// 暂时和虚拟化是没有什么关系的，只是当存在虚拟机的时候需要
		// 难道 guest 中间无法使用 load / store 监视吗? (可以的
		// 如何初始化 load / store 监视点之类寄存器(因为现在不需要进行初始化，所以直接 disable 处理就可以了
		//
		// 监视点例外 : 的确是在列表中间
		//
		// TRGP 的作用
		// TLBRD : 是根据 Index 获取 TLB 的信息，所以不一定知道自己到底在哪一个位置
		// 根据虚拟地址获取的 TLBSRCH 指令
		//
		// 1.4.4 / 1.4.5 软中断 和 硬中断的描述
		// 关闭硬件的所有方式
		// HWIC : 用 host 的硬件撤销清除 Guest 中断
		// HWIP : 直接相连
		// HWIS : 中断注入
		// 其实就是中断可以直接注入，也可以完全转发，或者部分转发
		//
		// 软中断的控制比较简单了
		//
		// 控制了 guest 使用 TLB 的数量?
		// 在逻辑上是怎么操作的，TLBFILL 和 INDEX 可以选中的范围吗 ?
		// 对于软件层的影响是什么 ?
		// 实际上，在软件层次，这没有任何的影响，kvm 的代码没有任何的检查。
		//
		// STLB 和 MTLB 的共享
		// - [x] 如果在 guest 中间设置的 STB 的 size 和 host 不同, 两者还可以共享吗?(是的，所以保证 STLBSZ 相同)
		// - [x] GID 的使用逻辑 1.2.3 中描述
		// - [x] VMM page : gpa 到 hpa 映射也是放到 TLB 中间的
		//
		// 问题是靠什么区分 VMM page 和 host page 的 TLB
		// - gpa 和 hva 的作为索引根本无法区分两者
		// - 查看内核的代码?
		// - 猜测在 guest mode 中间，当进行使用 Host 的, TLB refill 是自动的，并且自动使用 pgd.mm 的内容
		//
		// 1.2.2 中间说明, 非常的模糊, 莫名奇妙
		// - [x] PGDL 和 PGDH 的符号扩展?
		// - [x] 什么叫做 PGDH 和 GPDL 只有一套，所以其内容是相同的(什么垃圾文档
		//
		// 检查一下虚拟地址空间的范围是什么 ? 不用看，肯定没有问题的
		// - [x] 内核的代码的检查
		// - [x] 内核模块检查一下
		//
		// LONG_L	\tmp, \tmp1, VM_GPGD
		// csrwr	\tmp, LOONGARCH_CSR_PGDL
		//
		// GTLBC 在控制 Host 的 Guest ID 的取值
		//
		// 为什么 TLBRPRMD 中间含有 PGM，但是在 PRMD 中间没有
		// 1.7.1 : 这个记录了之前是否是客户机，如果是，那么就执行 eret 恢复客户机的状态
		// 当 eret 的时候，硬件用于区分当前是否在 Host 的状态 还是 Guest 的
		//
		// 在一般的状态，GSTAT 的 PGM 中间了保存了应该有的信息。
		//
		// GSTAT::GID 和 GTLBC::TGID :
		// 1. 一个是设置给虚拟机的 GID
		// 2. 一个是如果，想要进行控制虚拟机的 TLB, 那么需要提前设置该数值
		//
		// CSR_INIT_REG(ASID),
		// CSR_INIT_REG(PGDL),
		// CSR_INIT_REG(PGDH),
		// CSR_INIT_REG(PGD),
		CSR_INIT_REG(PWCTL0), CSR_INIT_REG(PWCTL1),
		CSR_INIT_REG(STLBPS), CSR_INIT_REG(RVACFG),
		CSR_INIT_REG(CPUNUM), CSR_INIT_REG(PRCFG1),
		CSR_INIT_REG(PRCFG2), CSR_INIT_REG(PRCFG3),
		// CSR_INIT_REG(KSCRATCH0),
		// CSR_INIT_REG(KSCRATCH1),
		// CSR_INIT_REG(KSCRATCH2),
		// CSR_INIT_REG(KSCRATCH3),
		// CSR_INIT_REG(KSCRATCH4),
		// CSR_INIT_REG(KSCRATCH5),
		// CSR_INIT_REG(KSCRATCH6),
		CSR_INIT_REG(KSCRATCH7), CSR_INIT_REG(KSCRATCH8),
		// CSR_INIT_REG(TIMERID), // kvm 会初始化
		CSR_INIT_REG(TIMERCFG),
		// CSR_INIT_REG(TIMERTICK),
		// CSR_INIT_REG(TIMEROFFSET),
		// CSR_INIT_REG(GSTAT), gcsr
		// CSR_INIT_REG(GCFG),  gcsr
		// CSR_INIT_REG(GINTC), gcsr
		// CSR_INIT_REG(GCNTC), gcsr
		CSR_INIT_REG(LLBCTL), CSR_INIT_REG(IMPCTL1),
		// CSR_INIT_REG(IMPCTL2),
		// CSR_INIT_REG(GNMI),
		CSR_INIT_REG(TLBREBASE),
		// CSR_INIT_REG(TLBRBADV),
		// CSR_INIT_REG(TLBREPC),
		// CSR_INIT_REG(TLBRSAVE),
		// CSR_INIT_REG(TLBRELO0),
		// CSR_INIT_REG(TLBRELO1),
		// CSR_INIT_REG(TLBREHI),
		// CSR_INIT_REG(TLBRPRMD),
		CSR_INIT_REG(ERRCTL),
		// CSR_INIT_REG(ERRINFO1),
		// CSR_INIT_REG(ERRINFO2),
		CSR_INIT_REG(ERREBASE),
		// CSR_INIT_REG(ERREPC),
		// CSR_INIT_REG(ERRSAVE),
		// CSR_INIT_REG(CTAG),
		// 虚拟化手册 1.3.3 这都是微结构相关的寄存器，修改需要 gpsi 来处理
		// 关于 mcsr 的具体含义，暂时也是不清楚的, host 是什么就填写什么
		CSR_INIT_REG(MCSR0), CSR_INIT_REG(MCSR1), CSR_INIT_REG(MCSR2),
		CSR_INIT_REG(MCSR3), CSR_INIT_REG(MCSR8), CSR_INIT_REG(MCSR9),
		CSR_INIT_REG(MCSR10), CSR_INIT_REG(MCSR24),
		// Uncached accelerate windows registers
		// CSR_INIT_REG(UCWIN),
		// CSR_INIT_REG(UCWIN0_LO),
		// CSR_INIT_REG(UCWIN0_HI),
		// CSR_INIT_REG(UCWIN1_LO),
		// CSR_INIT_REG(UCWIN1_HI),
		// CSR_INIT_REG(UCWIN2_LO),
		// CSR_INIT_REG(UCWIN2_HI),
		// CSR_INIT_REG(UCWIN3_LO),
		// CSR_INIT_REG(UCWIN3_HI),
		// CSR_INIT_REG(DMWIN0),
		CSR_INIT_REG(DMWIN1),
		// CSR_INIT_REG(DMWIN2),
		// CSR_INIT_REG(DMWIN3),
		CSR_INIT_REG(PERF0_EVENT),
		// CSR_INIT_REG(PERF0_COUNT),
		CSR_INIT_REG(PERF1_EVENT),
		// CSR_INIT_REG(PERF1_COUNT),
		CSR_INIT_REG(PERF2_EVENT),
		// CSR_INIT_REG(PERF2_COUNT),
		CSR_INIT_REG(PERF3_EVENT),
		// CSR_INIT_REG(PERF3_COUNT),
		CSR_INIT_REG(DEBUG),
		// CSR_INIT_REG(DEPC),
		// CSR_INIT_REG(DESAVE),
	};

	for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
		one_regs[i].reg.addr = (u64) & (one_regs[i].v);
	}

	for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
		if (ioctl(cpu->vcpu_fd, KVM_SET_ONE_REG, &(one_regs[i].reg)) <
		    0) {
			die("KVM_SET_ONE_REG %s", one_regs[i].name);
		} else {
			// pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
			// one_regs[i].v);
		}
	}
}

static int __attribute__((noinline))
kvm_launch(struct kvm_cpu *cpu, struct kvm_regs *regs)
{
	asm goto("\n\t"
		 "st.d $r0,  $r5, 0\n\t"
		 "st.d $r1,  $r5, 8\n\t"
		 "st.d $r2,  $r5, 16\n\t"
		 "st.d $r3,  $r5, 24\n\t"
		 "st.d $r4,  $r5, 32\n\t"
		 "st.d $r5,  $r5, 40\n\t"
		 "st.d $r6,  $r5, 48\n\t"
		 "st.d $r7,  $r5, 56\n\t"
		 "st.d $r8,  $r5, 64\n\t"
		 "st.d $r9,  $r5, 72\n\t"
		 "st.d $r10, $r5, 80\n\t"
		 "st.d $r11, $r5, 88\n\t"
		 "st.d $r12, $r5, 96\n\t"
		 "st.d $r13, $r5, 104\n\t"
		 "st.d $r14, $r5, 112\n\t"
		 "st.d $r15, $r5, 120\n\t"
		 "st.d $r16, $r5, 128\n\t"
		 "st.d $r17, $r5, 136\n\t"
		 "st.d $r18, $r5, 144\n\t"
		 "st.d $r19, $r5, 152\n\t"
		 "st.d $r20, $r5, 160\n\t"
		 "st.d $r21, $r5, 168\n\t"
		 "st.d $r22, $r5, 176\n\t"
		 "st.d $r23, $r5, 184\n\t"
		 "st.d $r24, $r5, 192\n\t"
		 "st.d $r25, $r5, 200\n\t"
		 "st.d $r26, $r5, 208\n\t"
		 "st.d $r27, $r5, 216\n\t"
		 "st.d $r28, $r5, 224\n\t"
		 "st.d $r29, $r5, 232\n\t"
		 "st.d $r30, $r5, 240\n\t"
		 "st.d $r31, $r5, 248\n\t"

		 "la.local $r6, %l[guest_entry]\n\t"
		 "st.d $r6, $r5, 256\n\t"
		 "ld.d $r6, $r5, 64\n\t" // restore $6
		 :
		 :
		 : "memory"
		 : guest_entry);

	regs->pc -= 4; // let guest jump guest_entry directly
	arch_dump_regs(STDOUT_FILENO, *regs);

	init_ebase(cpu);
	init_csr(cpu);
	// init_fpu(cpu); // TODO

	if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, regs) < 0) {
		die("KVM_SET_REGS failed");
	}

	vacate_current_stack(cpu);
	die("host never reach here\n");
guest_entry:
	// return expression is needed by guest_entry, otherwise gcc inline asm would
	// complaint
	return 0;
}

void arch_dune_enter(struct kvm_cpu *cpu)
{
	BUILD_ASSERT(INIT_VALUE_PRCFG2 & (1 << TLB_PS));

	struct kvm_regs regs;
	BUILD_ASSERT(256 == offsetof(struct kvm_regs, pc));
	kvm_launch(cpu, &regs);
}

// a7 是作为 syscall number

/**
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)                         \
	internal_syscall##nr(number, err, args)

#define internal_syscall7(number, err, arg0, arg1, arg2, arg3, arg4, arg5,     \
			  arg6)                                                \
	({                                                                     \
		long int _sys_result;                                          \
                                                                               \
		{                                                              \
			register long int __a7 asm("$a7") = number;            \
			register long int __a0 asm("$a0") = (long int)(arg0);  \
			register long int __a1 asm("$a1") = (long int)(arg1);  \
			register long int __a2 asm("$a2") = (long int)(arg2);  \
			register long int __a3 asm("$a3") = (long int)(arg3);  \
			register long int __a4 asm("$a4") = (long int)(arg4);  \
			register long int __a5 asm("$a5") = (long int)(arg5);  \
			register long int __a6 asm("$a6") = (long int)(arg6);  \
			__asm__ volatile("syscall	0\n\t"                       \
					 : "+r"(__a0)                          \
					 : "r"(__a7), "r"(__a1), "r"(__a2),    \
					   "r"(__a3), "r"(__a4), "r"(__a5),    \
					   "r"(__a6)                           \
					 : __SYSCALL_CLOBBERS);                \
			_sys_result = __a0;                                    \
		}                                                              \
		_sys_result;                                                   \
	})


# define INTERNAL_SYSCALL_ERROR_P(val, err) \
	((unsigned long int) (val) > -4096UL)

long int syscall(long int syscall_number, long int arg1, long int arg2,
		 long int arg3, long int arg4, long int arg5, long int arg6,
		 long int arg7)
{
	long int ret;
	INTERNAL_SYSCALL_DECL(err);

	ret = INTERNAL_SYSCALL_NCS(syscall_number, err, 7, arg1, arg2, arg3,
				   arg4, arg5, arg6, arg7);

	if (INTERNAL_SYSCALL_ERROR_P(ret, err))
		return __syscall_error(ret);

	return ret;
}
*/

#define __SYSCALL_CLOBBERS                                                     \
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"

#ifdef LOONGSON
bool do_syscall(struct kvm_cpu *cpu, bool is_fork)
{
	register long int __a7 asm("$a7") = cpu->syscall_parameter[0];
	register long int __a0 asm("$a0") = cpu->syscall_parameter[1];
	register long int __a1 asm("$a1") = cpu->syscall_parameter[2];
	register long int __a2 asm("$a2") = cpu->syscall_parameter[3];
	register long int __a3 asm("$a3") = cpu->syscall_parameter[4];
	register long int __a4 asm("$a4") = cpu->syscall_parameter[5];
	register long int __a5 asm("$a5") = cpu->syscall_parameter[6];
	register long int __a6 asm("$a6") = cpu->syscall_parameter[7];

	__asm__ volatile("syscall	0\n\t"
			 : "+r"(__a0)
			 : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3),
			   "r"(__a4), "r"(__a5), "r"(__a6)
			 : __SYSCALL_CLOBBERS);

	if (is_fork && __a0 == 0) {
		return true;
	}

	cpu->syscall_parameter[0] = __a0;
	return false;
}
#endif

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(parent_cpu->info.regs)) <
	    0)
		die("KVM_GET_REGS");

	parent_cpu->info.era =
		kvm_get_csr_reg(parent_cpu, KVM_LOONGARCH_CSR_EPC);

	// TODO
	// kvm_get_fpu_regs(parent_cpu, &parent_cpu->info.fpu);
}

void init_child_thread_info(struct kvm_cpu *child_cpu,
			    const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_regs child_regs;
	ebase_share(child_cpu, parent_cpu);

	memcpy(&child_regs, &parent_cpu->info.regs, sizeof(struct kvm_regs));

	// #define v0 $r4
	child_regs.gpr[4] = 0;
	// see arch/loongarch/kvm/loongisa.c:kvm_arch_vcpu_ioctl_run
	child_cpu->kvm_run->hypercall.ret = child_regs.gpr[4];

	if (parent_cpu->syscall_parameter[0] | CLONE_SETTLS) {
		child_regs.gpr[2] = parent_cpu->syscall_parameter[4];
	}

	// #define a1 $r5
	child_regs.gpr[5] = (u64)child_cpu;

	if (sysno == SYS_CLONE) {
		// see linux kernel fork.c:copy_thread
		// #define sp $r3
		if (parent_cpu->syscall_parameter[1] != 0)
			child_regs.gpr[3] = parent_cpu->syscall_parameter[1];
	} else if (sysno == SYS_CLONE3) {
		die("No support for clone3");
	}

	child_regs.pc = parent_cpu->info.era + 4;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &child_regs) < 0)
		die("KVM_SET_REGS");

	// TODO
	// dup_fpu(child_cpu, &parent_cpu->info.fpu);
	init_csr(child_cpu);
}

void arch_set_thread_area(struct kvm_cpu *vcpu)
{
	// loongarch 上没有 SYS_SET_THREAD_AREA
}

// 应该没有特殊的 syscall 需要处理
bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno)
{
	return false;
}

void escape()
{
	die("unimp");
}

u64 __do_simulate_clone(u64, u64, u64, u64, u64);

void do_simulate_clone(struct kvm_cpu *parent_cpu, u64 child_host_stack)
{
	u64 arg0 = parent_cpu->syscall_parameter[0];
	// u64 a1 = parent_cpu->syscall_parameter[1];
	u64 arg2 = parent_cpu->syscall_parameter[2];
	u64 arg3 = parent_cpu->syscall_parameter[3];
	u64 arg4 = parent_cpu->syscall_parameter[4];

	// parent 原路返回，child 进入到 child_entry 中间
	long child_pid =
		__do_simulate_clone(arg0, child_host_stack, arg2, arg3, arg4);

	if (child_pid > 0) {
		parent_cpu->syscall_parameter[0] = child_pid;
	} else {
		parent_cpu->syscall_parameter[0] = -child_pid;
	}
}
