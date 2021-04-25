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

static void dup_fpu(struct kvm_cpu *child_cpu,
		    const struct loongarch_fpu_struct *parent_fpu)
{
	kvm_enable_fpu(child_cpu);
	kvm_set_fpu_regs(child_cpu, parent_fpu);
}

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(parent_cpu->info.regs)) <
	    0)
		die("KVM_GET_REGS");

	parent_cpu->info.era =
		kvm_get_csr_reg(parent_cpu, KVM_CSR_EPC);

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
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fpr[0]) ==
		     VCPU_FPR0);
	BUILD_ASSERT(offsetof(struct loongarch_fpu_struct, fpr[31]) ==
		     VCPU_FPR0 + 31 * VCPU_FPR_LEN);

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
		*x = (0x002b8000 | INVALID_EBASE_POSITION);
	}

	// extern void err_entry_begin(void);
	// extern void err_entry_end(void);
	extern void tlb_refill_entry_begin(void);
	extern void tlb_refill_entry_end(void);
	extern void syscall_entry_begin(void);
	extern void syscall_entry_end(void);

	memcpy(cpu->info.ebase, tlb_refill_entry_begin,
	       tlb_refill_entry_end - tlb_refill_entry_begin);
	memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_SYS, syscall_entry_begin,
	       syscall_entry_end - syscall_entry_begin);
	// memcpy(cpu->info.ebase + ERREBASE_OFFSET, err_entry_begin,
	// err_entry_end - err_entry_begin);
  
	pr_info("ebase address : %llx", cpu->info.ebase);
	pr_info("ebase address : %llx", cpu->info.ebase + VEC_SIZE * EXCCODE_SYS);
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
	u64 INIT_VALUE_KSCRATCH5 = (u64)cpu->syscall_parameter + CSR_DMW1_BASE;
  u64 INIT_VALUE_KSCRATCH6 = TLBRELO0_STANDARD_BITS;
	u64 INIT_VALUE_KSCRATCH7 = TLBRELO1_STANDARD_BITS;

	u64 INIT_VALUE_TLBREBASE = (u64)cpu->info.ebase;
	u64 INIT_VALUE_EBASE = (u64)cpu->info.ebase;

	// FIXME 从手册和内核上，都是 tid 代替 cpuid 的感觉，kvm 中将会将 tid 初始化为 cpu->cpu_id
	u64 INIT_VALUE_CPUNUM = cpu->cpu_id;

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
		// CSR_INIT_REG(GTLBC),
		// CSR_INIT_REG(TRGP),
		// CSR_INIT_REG(ASID),
		// CSR_INIT_REG(PGDL),
		// CSR_INIT_REG(PGDH),
		// CSR_INIT_REG(PGD),
		CSR_INIT_REG(PWCTL0), CSR_INIT_REG(PWCTL1),
		CSR_INIT_REG(STLBPS), CSR_INIT_REG(RVACFG),
		CSR_INIT_REG(CPUNUM),
		// CSR_INIT_REG(PRCFG1),
		// CSR_INIT_REG(PRCFG2),
		// CSR_INIT_REG(PRCFG3),
		// CSR_INIT_REG(KSCRATCH0),
		// CSR_INIT_REG(KSCRATCH1),
		// CSR_INIT_REG(KSCRATCH2),
		// CSR_INIT_REG(KSCRATCH3),
		// CSR_INIT_REG(KSCRATCH4),
		CSR_INIT_REG(KSCRATCH5), CSR_INIT_REG(KSCRATCH6),
		CSR_INIT_REG(KSCRATCH7),
		// CSR_INIT_REG(TIMERID), // kvm 会初始化
		// 从 kvm_vz_queue_timer_int_cb 看，disable 掉 TIMERCFG::EN 的确可以不被注入
		// 时钟中断
		CSR_INIT_REG(TIMERCFG),
		// CSR_INIT_REG(TIMERTICK),
		// CSR_INIT_REG(TIMEROFFSET),
		// CSR_INIT_REG(GSTAT), gcsr
		// CSR_INIT_REG(GCFG),  gcsr
		// CSR_INIT_REG(GINTC), gcsr
		// CSR_INIT_REG(GCNTC), gcsr
		// CSR_INIT_REG(LLBCTL), // 没有这个入口
		// CSR_INIT_REG(IMPCTL1),
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
		// 按照现在的涉及，guest 中间不应该出现 merr
		// CSR_INIT_REG(ERRCTL),
		// CSR_INIT_REG(ERRINFO1),
		// CSR_INIT_REG(ERRINFO2),
		// CSR_INIT_REG(ERREBASE),
		// CSR_INIT_REG(ERREPC),
		// CSR_INIT_REG(ERRSAVE),
		// CSR_INIT_REG(CTAG),
		// CSR_INIT_REG(MCSR0),
		// CSR_INIT_REG(MCSR1),
		// CSR_INIT_REG(MCSR2),
		// CSR_INIT_REG(MCSR3),
		// CSR_INIT_REG(MCSR8),
		// CSR_INIT_REG(MCSR9),
		// CSR_INIT_REG(MCSR10),
		// CSR_INIT_REG(MCSR24),
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
		// FIXME : 没有办法控制 perf 寄存器, 最后会不会导致 perf 其实默认是打开的
		// 等到可以进入到 guest 打开试试
		// CSR_INIT_REG(PERF0_EVENT),
		// CSR_INIT_REG(PERF0_COUNT),
		// CSR_INIT_REG(PERF1_EVENT),
		// CSR_INIT_REG(PERF1_COUNT),
		// CSR_INIT_REG(PERF2_EVENT),
		// CSR_INIT_REG(PERF2_COUNT),
		// CSR_INIT_REG(PERF3_EVENT),
		// CSR_INIT_REG(PERF3_COUNT),
		// CSR_INIT_REG(DEBUG),
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
  BUILD_ASSERT(offsetof(struct kvm_regs, pc) == 256);
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

	arch_dump_regs(STDOUT_FILENO, *regs);

	init_ebase(cpu);
	init_csr(cpu);
	init_fpu(cpu);

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
	struct kvm_regs regs;
	BUILD_ASSERT(256 == offsetof(struct kvm_regs, pc));
	kvm_launch(cpu, &regs);
}

// a7($r11) 是作为 syscall number
u64 arch_get_sysno(const struct kvm_cpu *cpu){
  return cpu->syscall_parameter[7];
}
#define __SYSCALL_CLOBBERS                                                     \
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"

#ifdef LOONGSON
bool arch_do_syscall(struct kvm_cpu *cpu, bool is_fork)
{
	register long int __a7 asm("$a7") = cpu->syscall_parameter[7];
	register long int __a0 asm("$a0") = cpu->syscall_parameter[0];
	register long int __a1 asm("$a1") = cpu->syscall_parameter[1];
	register long int __a2 asm("$a2") = cpu->syscall_parameter[2];
	register long int __a3 asm("$a3") = cpu->syscall_parameter[3];
	register long int __a4 asm("$a4") = cpu->syscall_parameter[4];
	register long int __a5 asm("$a5") = cpu->syscall_parameter[5];
	register long int __a6 asm("$a6") = cpu->syscall_parameter[6];

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

	if (parent_cpu->syscall_parameter[0] & CLONE_SETTLS) {
		child_regs.gpr[2] = parent_cpu->syscall_parameter[4];
	}

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

	dup_fpu(child_cpu, &parent_cpu->info.fpu);
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

u64 __do_simulate_clone(u64, u64, u64, u64, u64, u64);

void do_simulate_clone(struct kvm_cpu *parent_cpu, const struct kvm_cpu * child_cpu, u64 child_host_stack)
{
	u64 arg0 = parent_cpu->syscall_parameter[0];
	// u64 a1 = parent_cpu->syscall_parameter[1];
	u64 arg2 = parent_cpu->syscall_parameter[2];
	u64 arg3 = parent_cpu->syscall_parameter[3];
	u64 arg4 = parent_cpu->syscall_parameter[4];

	// parent 原路返回，child 进入到 child_entry 中间
	long child_pid =
		__do_simulate_clone(arg0, child_host_stack, arg2, arg3, arg4, (u64)child_cpu);

	if (child_pid > 0) {
		parent_cpu->syscall_parameter[0] = child_pid;
	} else {
		parent_cpu->syscall_parameter[0] = -child_pid;
	}
}
