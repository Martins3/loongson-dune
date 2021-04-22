#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "arch.h"
#include "internal.h"
#include "../interface.h"

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

	dprintf(debug_fd, "hi   : %016llx\n", (unsigned long long)regs.hi);
	dprintf(debug_fd, "lo   : %016llx\n", (unsigned long long)regs.lo);
	dprintf(debug_fd, "pc  : %016llx\n", (unsigned long long)regs.pc);

	dprintf(debug_fd, "\n");
}

extern int host_loop_pipe(int pipedes[2]);
void host_loop(struct kvm_cpu *vcpu);

struct cp0_reg {
	struct kvm_one_reg reg;
	char name[100];
	u64 v;
};

extern void ebase_tlb_entry_begin(void);
extern void ebase_tlb_entry_end(void);

extern void ebase_error_entry_begin(void);
extern void ebase_error_entry_end(void);

#define EBASE_TLB_OFFSET 0x0
#define EBASE_XTLB_OFFSET 0x80
#define EBASE_CACHE_OFFSET 0x100
#define EBASE_GE_OFFSET 0x180

static void ebase_alloc(struct kvm_cpu *cpu)
{
	int i;
	void *addr = mmap_one_page();
	cpu->info.ebase = addr;
	// hypercall instruction used for catching invalid access
	for (int i = 0; i < PAGESIZE / 4; ++i) {
		int *x = (int *)addr;
		x = x + i;
		*x = (0x42000028 + (INVALID_EBASE_POSITION << 11));
	}

	assert((void *)ebase_tlb_entry_end - (void *)ebase_error_entry_begin <
	       (EBASE_CACHE_OFFSET - EBASE_XTLB_OFFSET));
	pr_info("ebase address : %llx", (u64)addr);
}

static void ebase_init_tlb(struct kvm_cpu *cpu)
{
	memcpy(cpu->info.ebase + EBASE_TLB_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static void ebase_init_xtlb(struct kvm_cpu *cpu)
{
	memcpy(cpu->info.ebase + EBASE_XTLB_OFFSET, ebase_tlb_entry_begin,
	       ebase_tlb_entry_end - ebase_tlb_entry_begin);
}

static void ebase_init_cache(struct kvm_cpu *cpu)
{
	memcpy(cpu->info.ebase + EBASE_CACHE_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static void ebase_init_general(struct kvm_cpu *cpu)
{
	extern void ebase_general_entry_begin(void);
	extern void ebase_general_entry_end(void);
	memcpy(cpu->info.ebase + EBASE_GE_OFFSET, ebase_general_entry_begin,
	       ebase_general_entry_end - ebase_general_entry_begin);
}

#define CP0_INIT_REG(X)                                                        \
	{                                                                      \
		.reg = { .id = KVM_REG_MIPS_CP0_##X }, .name = #X,             \
		.v = INIT_VALUE_##X                                            \
	}

static const u64 MIPS_XKPHYSX_CACHED = 0x9800000000000000;

static inline u64 get_tp()
{
	u64 tp;
	__asm__("rdhwr %0, $29" : "=r"(tp));
	return tp;
}

static void ebase_init(struct kvm_cpu *cpu)
{
	ebase_alloc(cpu);
	ebase_init_tlb(cpu);
	ebase_init_xtlb(cpu);
	ebase_init_cache(cpu);
	ebase_init_general(cpu);
}

static void ebase_share(struct kvm_cpu *child_cpu,
			const struct kvm_cpu *parent_cpu)
{
	child_cpu->info.ebase = parent_cpu->info.ebase;
}

static void init_cp0(struct kvm_cpu *cpu)
{
	if (!cpu->info.ebase)
		die("init_cp0 with invalid ebase");
	u64 INIT_VALUE_EBASE = (u64)cpu->info.ebase + MIPS_XKPHYSX_CACHED;
	u64 INIT_VALUE_USERLOCAL = get_tp();
	u64 INIT_VALUE_KSCRATCH1 =
		(u64)(&cpu->syscall_parameter) + MIPS_XKPHYSX_CACHED;

	int i;
	struct cp0_reg one_regs[] = {
		CP0_INIT_REG(INDEX),
		CP0_INIT_REG(RANDOM),
		CP0_INIT_REG(ENTRYLO0),
		CP0_INIT_REG(ENTRYLO1),
		CP0_INIT_REG(CONTEXT),
		// CP0_INIT_REG(CONTEXTCONFIG),
		CP0_INIT_REG(USERLOCAL),
		// CP0_INIT_REG(XCONTEXTCONFIG),
		CP0_INIT_REG(PAGEMASK),
		CP0_INIT_REG(PAGEGRAIN),
		// CP0_INIT_REG(SEGCTL0),
		// CP0_INIT_REG(SEGCTL1),
		// CP0_INIT_REG(SEGCTL2),
		CP0_INIT_REG(PWBASE),
		CP0_INIT_REG(PWFIELD),
		CP0_INIT_REG(PWSIZE),
		CP0_INIT_REG(WIRED),
		CP0_INIT_REG(PWCTL),
		CP0_INIT_REG(HWRENA),
		CP0_INIT_REG(BADVADDR),
		CP0_INIT_REG(BADINSTR),
		CP0_INIT_REG(BADINSTRP),
		CP0_INIT_REG(COUNT),
		CP0_INIT_REG(ENTRYHI),
		CP0_INIT_REG(COMPARE),
		CP0_INIT_REG(STATUS),
		CP0_INIT_REG(INTCTL),
		CP0_INIT_REG(CAUSE),
		CP0_INIT_REG(EPC),
		CP0_INIT_REG(PRID),
		CP0_INIT_REG(EBASE),

		CP0_INIT_REG(CONFIG),
		CP0_INIT_REG(CONFIG1),
		CP0_INIT_REG(CONFIG2),
		CP0_INIT_REG(CONFIG3),
		CP0_INIT_REG(CONFIG4),
		CP0_INIT_REG(CONFIG5),
		CP0_INIT_REG(CONFIG6),
		CP0_INIT_REG(CONFIG7),

		CP0_INIT_REG(XCONTEXT),
		CP0_INIT_REG(GSCAUSE),
		CP0_INIT_REG(ERROREPC),

		CP0_INIT_REG(KSCRATCH1),
		CP0_INIT_REG(KSCRATCH2),
		CP0_INIT_REG(KSCRATCH3),
		CP0_INIT_REG(KSCRATCH4),
		CP0_INIT_REG(KSCRATCH5),
		CP0_INIT_REG(KSCRATCH6),
	};

	for (i = 0; i < sizeof(one_regs) / sizeof(struct cp0_reg); ++i) {
		one_regs[i].reg.addr = (u64) & (one_regs[i].v);
	}

	for (i = 0; i < sizeof(one_regs) / sizeof(struct cp0_reg); ++i) {
		if (ioctl(cpu->vcpu_fd, KVM_SET_ONE_REG, &(one_regs[i].reg)) <
		    0) {
			die("KVM_SET_ONE_REG %s", one_regs[i].name);
		} else {
			// pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
			// one_regs[i].v);
		}
	}
}

extern void get_fpu_regs(struct mips_fpu_struct *);
extern void get_fcsr(unsigned int *);
extern void get_msacsr(unsigned int *);

#define KVM_REG_MIPS_VEC_256(n) (KVM_REG_MIPS_FPR | KVM_REG_SIZE_U256 | (n))

static void kvm_enable_fpu(struct kvm_cpu *cpu)
{
	struct kvm_enable_cap cap;
	memset(&cap, 0, sizeof(cap));
	cap.cap = KVM_CAP_MIPS_FPU;

	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		die("Unable enable fpu in guest");
	}

	cap.cap = KVM_CAP_MIPS_MSA;
	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		die("Unable enable msa in guest");
	}

	// 从 kvm_arch_init_vm 可以看到不需要手动打开 lasx
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

static u64 kvm_access_cp0_reg(const struct kvm_cpu *cpu, u64 id,
			      enum ACCESS_OP op, u64 value)
{
	struct kvm_one_reg reg;
	u64 v = (op == GET) ? 0 : value;
	reg.addr = (u64) & (v);
	reg.id = id;

	kvm_access_reg(cpu, &reg, op);
	return v;
}

static u64 kvm_get_cp0_reg(const struct kvm_cpu *cpu, u64 id)
{
	return kvm_access_cp0_reg(cpu, id, GET, 0);
}

static void kvm_set_cp0_reg(const struct kvm_cpu *cpu, u64 id, u64 v)
{
	kvm_access_cp0_reg(cpu, id, SET, v);
}

static void kvm_access_fpu_regs(struct kvm_cpu *cpu,
				const struct mips_fpu_struct *fpu_regs,
				enum ACCESS_OP op)
{
	struct kvm_one_reg reg;

	for (int i = 0; i < NUM_FPU_REGS; ++i) {
		reg.id = KVM_REG_MIPS_VEC_256(i);
		reg.addr = (u64) & (fpu_regs->fpr[i]);
		kvm_access_reg(cpu, &reg, op);
	}

	reg.id = KVM_REG_MIPS_FCR_CSR;
	reg.addr = (u64) & (fpu_regs->fcr31);
	kvm_access_reg(cpu, &reg, op);

	reg.id = KVM_REG_MIPS_MSA_CSR;
	reg.addr = (u64) & (fpu_regs->msacsr);
	kvm_access_reg(cpu, &reg, op);
}

static void kvm_get_fpu_regs(struct kvm_cpu *cpu,
			     const struct mips_fpu_struct *fpu_regs)
{
	kvm_access_fpu_regs(cpu, fpu_regs, GET);
}

static void kvm_set_fpu_regs(struct kvm_cpu *cpu,
			     const struct mips_fpu_struct *fpu_regs)
{
	kvm_access_fpu_regs(cpu, fpu_regs, SET);
}

static void init_fpu(struct kvm_cpu *cpu)
{
	kvm_enable_fpu(cpu);

	struct mips_fpu_struct fpu_regs;
	get_fpu_regs(&fpu_regs);
	get_fcsr(&fpu_regs.fcr31);
	get_msacsr(&fpu_regs.msacsr);

	kvm_set_fpu_regs(cpu, &fpu_regs);
}

static int __attribute__((noinline))
kvm_launch(struct kvm_cpu *cpu, struct kvm_regs *regs)
{
	asm goto(".set noat\n\t"
		 ".set noreorder\n\t"

		 "sd $0, 0($a1)\n\t"
		 "sd $1, 8($a1)\n\t"
		 "sd $2, 16($a1)\n\t"
		 "sd $3, 24($a1)\n\t"
		 "sd $4, 32($a1)\n\t"
		 "sd $5, 40($a1)\n\t"
		 "sd $6, 48($a1)\n\t"
		 "sd $7, 56($a1)\n\t"
		 "sd $8, 64($a1)\n\t"
		 "sd $9, 72($a1)\n\t"
		 "sd $10, 80($a1)\n\t"
		 "sd $11, 88($a1)\n\t"
		 "sd $12, 96($a1)\n\t"
		 "sd $13, 104($a1)\n\t"
		 "sd $14, 112($a1)\n\t"
		 "sd $15, 120($a1)\n\t"
		 "sd $16, 128($a1)\n\t"
		 "sd $17, 136($a1)\n\t"
		 "sd $18, 144($a1)\n\t"
		 "sd $19, 152($a1)\n\t"
		 "sd $20, 160($a1)\n\t"
		 "sd $21, 168($a1)\n\t"
		 "sd $22, 176($a1)\n\t"
		 "sd $23, 184($a1)\n\t"
		 "sd $24, 192($a1)\n\t"
		 "sd $25, 200($a1)\n\t"
		 "sd $26, 208($a1)\n\t"
		 "sd $27, 216($a1)\n\t"
		 "sd $28, 224($a1)\n\t"
		 "sd $29, 232($a1)\n\t"
		 "sd $30, 240($a1)\n\t"
		 "sd $31, 248($a1)\n\t"

		 "mfhi $8\n\t"
		 "sd $8, 256($a1)\n\t"

		 "mflo $8\n\t"
		 "sd $8, 264($a1)\n\t"

		 "dla $8, %l[guest_entry]\n\t"
		 "sd $8, 272($a1)\n\t"

		 "ld $8, 64($a1)\n\t" // restore $8
		 ".set at\n\t"
		 ".set reorder\n\t"
		 :
		 :
		 : "memory"
		 : guest_entry);

	// dump_kvm_regs(STDOUT_FILENO, *regs);

	ebase_init(cpu);

	init_cp0(cpu);

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
	BUILD_ASSERT(272 == offsetof(struct kvm_regs, pc));
	kvm_launch(cpu, &regs);
}

bool do_syscall(struct kvm_cpu *cpu, bool is_fork)
{
	register long r4 __asm__("$4") = cpu->syscall_parameter[1];
	register long r5 __asm__("$5") = cpu->syscall_parameter[2];
	register long r6 __asm__("$6") = cpu->syscall_parameter[3];
	register long r7 __asm__("$7") = cpu->syscall_parameter[4];
	register long r8 __asm__("$8") = cpu->syscall_parameter[5];
	register long r9 __asm__("$9") = cpu->syscall_parameter[6];
	register long r2 __asm__("$2");
	register long r3 __asm__("$3");

	__asm__ __volatile__("daddu $2,$0,%2 ; syscall"
			     : "=&r"(r2), "+r"(r7)
			     : "ir"(cpu->syscall_parameter[0]), "0"(r2),
			       "r"(r4), "r"(r5), "r"(r6), "r"(r8), "r"(r9)
			     : SYSCALL_CLOBBERLIST);

	if (is_fork && r2 == 0 && r7 == 0) {
		return true;
	}

	cpu->syscall_parameter[0] = r2;
	cpu->syscall_parameter[4] = r7;
	return false;
}

void child_entry(struct kvm_cpu *cpu)
{
	kvm_set_cp0_reg(cpu, KVM_REG_MIPS_CP0_USERLOCAL, get_tp());
	host_loop(cpu);
}

static void dup_fpu(struct kvm_cpu *child_cpu,
		    const struct mips_fpu_struct *parent_fpu)
{
	kvm_enable_fpu(child_cpu);
	kvm_set_fpu_regs(child_cpu, parent_fpu);
}

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(parent_cpu->info.regs)) <
	    0)
		die("KVM_GET_REGS");

	parent_cpu->info.epc =
		kvm_get_cp0_reg(parent_cpu, KVM_REG_MIPS_CP0_EPC);

	kvm_get_fpu_regs(parent_cpu, &parent_cpu->info.fpu);
}

// TODO 如果 fork 或者 clone 失败，创建的虚拟机和 vcpu 都需要销毁才对
//
// TODO 为什么新创建的 vcpu 不能直接进入到 syscall handler 中间 ?
// 我的猜测是因为 : pc 导致直接进入 xphysc 的位置，pagegrain 没有打开，所以实际
// 上出现问题
void init_child_thread_info(struct kvm_cpu *child_cpu,
			    const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_regs child_regs;
	ebase_share(child_cpu, parent_cpu);

	memcpy(&child_regs, &parent_cpu->info.regs, sizeof(struct kvm_regs));

	// child always return 0
	child_regs.gpr[2] = 0;
	child_regs.gpr[7] = 0;

	// loongson kvm assign gpr[2] with hypercall.ret in in kvm_arch_vcpu_ioctl_run
	child_cpu->kvm_run->hypercall.ret = child_regs.gpr[2];

	// #define sp	$29
	if (sysno == SYS_CLONE) {
		// see linux kernel fork.c:copy_thread
		if (parent_cpu->syscall_parameter[2] != 0)
			child_regs.gpr[29] = parent_cpu->syscall_parameter[2];
	} else if (sysno == SYS_CLONE3) {
		die("No support for clone3");
	}

	// child start at next instruction of syscall
	child_regs.pc = parent_cpu->info.epc + 4;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &child_regs) < 0)
		die("KVM_SET_REGS");

	dup_fpu(child_cpu, &parent_cpu->info.fpu);

	init_cp0(child_cpu);
}

void arch_set_thread_area(struct kvm_cpu *vcpu)
{
	kvm_set_cp0_reg(vcpu, KVM_REG_MIPS_CP0_USERLOCAL, get_tp());
}

bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno)
{
	if (sysno == SYS_PIPE) {
		int *pipedes = (int *)vcpu->syscall_parameter[1];
		int ret = host_loop_pipe(pipedes);

		if (ret != 0) {
			vcpu->syscall_parameter[0] = ret;
			vcpu->syscall_parameter[4] = 1;
		} else {
			vcpu->syscall_parameter[0] = pipedes[0];
			vcpu->syscall_parameter[1] = pipedes[1];
			vcpu->syscall_parameter[4] = 0;
		}
		return true;
	}
	return false;
}

u64 dune_clone(u64 r4, u64 r5, u64 r6, u64 r7, u64 r8, u64 r9);
// 这个函数想要做成什么想要
void emulate_fork_by_another_vcpu(struct kvm_cpu *parent_cpu,
				  u64 child_host_stack)
{
	u64 r4 = parent_cpu->syscall_parameter[1];
	// u64 r5 = parent_cpu->syscall_parameter[2];
	u64 r6 = parent_cpu->syscall_parameter[3];
	u64 r7 = parent_cpu->syscall_parameter[4];
	u64 r8 = parent_cpu->syscall_parameter[5];
	u64 r9 = parent_cpu->syscall_parameter[6];
	// parent 原路返回，child 进入到 child_entry 中间
	long child_pid = dune_clone(r4, child_host_stack, r6, r7, r8, r9);

	// This dependes on arch!
	if (child_pid > 0) {
		parent_cpu->syscall_parameter[0] = child_pid;
		parent_cpu->syscall_parameter[4] = 0;
	} else {
		parent_cpu->syscall_parameter[0] = -child_pid;
		parent_cpu->syscall_parameter[4] = 1;
	}
}

