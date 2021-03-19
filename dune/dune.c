#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "cp0.h"

// https://stackoverflow.com/questions/22449342/clone-vm-undeclared-first-use-in-this-function
#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>
#include <pthread.h>

#ifndef LOONGSON
#include "kvm.h"
#else
#include <linux/kvm.h>
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// It doesn't matter what KVM_VM_TYPE is, loongson just ignore it, see funtion :
// arch/mips/kvm/mips.c:kvm_arch_init_vm
#define KVM_VM_TYPE 1

#define KVM_MAX_VCPUS 16

struct kvm_cpu;
struct vcpu_pool_ele {
	bool valid;
	struct kvm_cpu *vcpu;
};

struct kvm_vm {
	int sys_fd;
	int vm_fd;
	int kvm_run_mmap_size;

	struct vcpu_pool_ele vcpu_pool[KVM_MAX_VCPUS];
	pthread_spinlock_t lock;
};

// reference from arch/mips/include/asm/processor.h
#define FPU_REG_WIDTH 256
#define NUM_FPU_REGS 32

union fpureg {
	__u32 val32[FPU_REG_WIDTH / 32];
	__u64 val64[FPU_REG_WIDTH / 64];
};

struct mips_fpu_struct {
	union fpureg fpr[NUM_FPU_REGS];
	unsigned int fcr31;
	unsigned int msacsr;
};

struct thread_info {
	struct kvm_regs regs;
	u64 epc;
	struct mips_fpu_struct fpu;
};

// reference : kvmtool/mips/include/kvm/kvm-cpu-arch.h
struct kvm_cpu {
	int cpu_id;
	struct kvm_vm *vm;
	int vcpu_fd; /* For VCPU ioctls() */
	struct kvm_run *kvm_run;
	void *ebase;
	// TODO 修改成为寄存器
	long syscall_parameter[7];
	int debug_fd;
	struct thread_info info;
};

struct kvm_cpu *kvm_init_vm_with_one_cpu();

bool do_syscall6(struct kvm_cpu *cpu, bool is_fork);

int host_loop_pipe(int pipedes[2]);
void host_loop_exit();

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

static void report(const char *prefix, const char *err, va_list params)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stdout, " %s%s\n", prefix, msg);
}

static void error_builtin(const char *err, va_list params)
{
	printf("%s", KCYN);
	report("Error: ", err, params);
	printf("%s", KNRM);
}

static void die_builtin(const char *err, va_list params)
{
	printf("%s", KRED);
	report(" Fatal: ", err, params);
	printf("%s", KNRM);
	exit(128);
}

static void info_builtin(const char *info, va_list params)
{
	report(" Info: ", info, params);
}

void pr_err(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	error_builtin(err, params);
	va_end(params);
}

void pr_info(const char *info, ...)
{
	va_list params;

	va_start(params, info);
	info_builtin(info, params);
	va_end(params);
}

void die(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	die_builtin(err, params);
	va_end(params);
}

void kvm_dump_regs(int debug_fd, struct kvm_regs regs)
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

struct cp0_reg {
	struct kvm_one_reg reg;
	char name[100];
	u64 v;
};

// TODO why kvmtool ignored PROT_EXEC ?
#define PROT_RWX (PROT_READ | PROT_WRITE | PROT_EXEC)
#define PROT_RW (PROT_READ | PROT_WRITE)
#define MAP_ANON_NORESERVE (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE)

extern void ebase_tlb_entry_begin(void);
extern void ebase_tlb_entry_end(void);

extern void ebase_error_entry_begin(void);
extern void ebase_error_entry_end(void);

#define EBASE_TLB_OFFSET 0x0
#define EBASE_XTLB_OFFSET 0x80
#define EBASE_CACHE_OFFSET 0x100
#define EBASE_GE_OFFSET 0x180

void *mmap_one_page()
{
	void *addr = mmap(NULL, PAGESIZE, PROT_RWX, MAP_ANON_NORESERVE, -1, 0);
	if (addr == NULL && ((u64)addr & 0xffff) != 0)
		die("mmap_one_page");
	return addr;
}

static void ebase_alloc(struct kvm_cpu *cpu)
{
	int i;
	void *addr = mmap_one_page();
	cpu->ebase = addr;
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
	memcpy(cpu->ebase + EBASE_TLB_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static void ebase_init_xtlb(struct kvm_cpu *cpu)
{
	memcpy(cpu->ebase + EBASE_XTLB_OFFSET, ebase_tlb_entry_begin,
	       ebase_tlb_entry_end - ebase_tlb_entry_begin);
}

static void ebase_init_cache(struct kvm_cpu *cpu)
{
	memcpy(cpu->ebase + EBASE_CACHE_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static void ebase_init_general(struct kvm_cpu *cpu)
{
	extern void ebase_general_entry_begin(void);
	extern void ebase_general_entry_end(void);
	memcpy(cpu->ebase + EBASE_GE_OFFSET, ebase_general_entry_begin,
	       ebase_general_entry_end - ebase_general_entry_begin);
}

#define CP0_INIT_REG(X)                                                        \
	{                                                                      \
		.reg = { .id = KVM_REG_MIPS_CP0_##X }, .name = #X,             \
		.v = INIT_VALUE_##X                                            \
	}

const u64 MIPS_XKPHYSX_CACHED = 0x9800000000000000;

// TODO how it works ?
// 1. every process has different value for USERLOCAL and won't change it once set it up
static inline u64 get_tp()
{
	u64 tp;
	__asm__("rdhwr %0, $29" : "=r"(tp));
	return tp;
}

void ebase_init(struct kvm_cpu *cpu)
{
	ebase_alloc(cpu);
	ebase_init_tlb(cpu);
	ebase_init_xtlb(cpu);
	ebase_init_cache(cpu);
	ebase_init_general(cpu);
}

void ebase_share(struct kvm_cpu *child_cpu, const struct kvm_cpu *parent_cpu)
{
	child_cpu->ebase = parent_cpu->ebase;
}

// TODO 验证一下
// cpu_guest_has_contextconfig
// cpu_guest_has_segments
// cpu_guest_has_maar && !cpu_guest_has_dyn_maar
static int init_cp0(struct kvm_cpu *cpu)
{
	if (!cpu->ebase)
		die("init_cp0 with invalid ebase");
	u64 INIT_VALUE_EBASE = (u64)cpu->ebase + MIPS_XKPHYSX_CACHED;
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
			pr_err("KVM_SET_ONE_REG %s", one_regs[i].name);
			return -1;
		} else {
			// pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
			// one_regs[i].v);
		}
	}
	return 0;
}

// TODO 将所有的返回值修改为这种模式
enum RETURN_TYPE { Err, Ok };

#define isErrMsg(val, msg)                                                     \
	while (0) {                                                            \
		if (val == Err) {                                              \
			pr_err(msg);                                           \
			return Err;                                            \
		}                                                              \
	}

#define isErr(val)                                                             \
	while (0) {                                                            \
		if (val == Err) {                                              \
			return Err;                                            \
		}                                                              \
	}

extern void get_fpu_regs(struct mips_fpu_struct *);
extern void get_fcsr(unsigned int *);
extern void get_msacsr(unsigned int *);

#define KVM_REG_MIPS_VEC_256(n) (KVM_REG_MIPS_FPR | KVM_REG_SIZE_U256 | (n))

int kvm_enable_fpu(struct kvm_cpu *cpu)
{
	struct kvm_enable_cap cap;
	memset(&cap, 0, sizeof(cap));
	cap.cap = KVM_CAP_MIPS_FPU;

	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		pr_err("Unable enable fpu in guest");
		return -1;
	}

	cap.cap = KVM_CAP_MIPS_MSA;
	if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
		pr_err("Unable enable msa in guest");
		return -1;
	}

	// 从 kvm_arch_init_vm 可以看到不需要手动打开 lasx
	return 0;
}

enum ACCESS_OP {
	GET = KVM_GET_ONE_REG,
	SET = KVM_SET_ONE_REG,
};

// TODO 忽然感觉 set reg 还需要 check ret 真的很烦人
int kvm_access_reg(const struct kvm_cpu *cpu, struct kvm_one_reg *reg,
		   enum ACCESS_OP op)
{
	if (ioctl(cpu->vcpu_fd, op, reg) < 0)
		return -1;
	return 0;
}

u64 kvm_access_cp0_reg(const struct kvm_cpu *cpu, u64 id, enum ACCESS_OP op,
		       u64 value)
{
	struct kvm_one_reg reg;
	u64 v = (op == GET) ? 0 : value;
	reg.addr = (u64) & (v);
	reg.id = id;

	kvm_access_reg(cpu, &reg, op);
	return v;
}

u64 kvm_get_cp0_reg(const struct kvm_cpu *cpu, u64 id)
{
	return kvm_access_cp0_reg(cpu, id, GET, 0);
}

u64 kvm_set_cp0_reg(const struct kvm_cpu *cpu, u64 id, u64 v)
{
	return kvm_access_cp0_reg(cpu, id, SET, v);
}

// TODO
// 1. 重新定义一下函数, 如果是从 kvm 中间 get 叫做 kvm_get, 类似的
// 2. 文件的上半部分全部定义 kvm 开始的 ioctl 封装函数, 下面定义组合内容
int kvm_access_fpu_regs(struct kvm_cpu *cpu,
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

	return 0;
}

int kvm_get_fpu_regs(struct kvm_cpu *cpu,
		     const struct mips_fpu_struct *fpu_regs)
{
	kvm_access_fpu_regs(cpu, fpu_regs, GET);
}

int kvm_set_fpu_regs(struct kvm_cpu *cpu,
		     const struct mips_fpu_struct *fpu_regs)
{
	kvm_access_fpu_regs(cpu, fpu_regs, SET);
}

static int init_fpu(struct kvm_cpu *cpu)
{
	if (kvm_enable_fpu(cpu)) {
		return -1;
	}

	struct mips_fpu_struct fpu_regs;
	get_fpu_regs(&fpu_regs);
	get_fcsr(&fpu_regs.fcr31);
	get_msacsr(&fpu_regs.msacsr);

	if (kvm_set_fpu_regs(cpu, &fpu_regs)) {
		return -1;
	}

	return 0;
}

struct kvm_cpu *kvm_free_vcpu(struct kvm_cpu *vcpu)
{
	struct kvm_vm *kvm = vcpu->vm;

	if (pthread_spin_lock(&kvm->lock)) {
		die("locked failed");
	}
	assert(kvm->vcpu_pool[vcpu->cpu_id].valid);
	kvm->vcpu_pool[vcpu->cpu_id].valid = false;

	if (pthread_spin_unlock(&kvm->lock)) {
		die("unlocked failed");
	}
}

struct kvm_cpu *kvm_alloc_vcpu(struct kvm_vm *kvm)
{
	struct kvm_cpu *vcpu;
	int cpu_id = -1;

	if (pthread_spin_lock(&kvm->lock)) {
		die("locked failed");
	}

	for (int i = 0; i < KVM_MAX_VCPUS; ++i) {
		if (kvm->vcpu_pool[i].valid)
			continue;

		if (kvm->vcpu_pool[i].vcpu != NULL) {
			kvm->vcpu_pool[i].valid = true;
			if (pthread_spin_unlock(&kvm->lock)) {
				die("unlocked failed");
			}
			return kvm->vcpu_pool[i].vcpu;
		}
	}

	for (int i = 0; i < KVM_MAX_VCPUS; ++i) {
		if (kvm->vcpu_pool[i].valid)
			continue;

		assert(kvm->vcpu_pool[i].vcpu == NULL);
		kvm->vcpu_pool[i].valid = true;
		cpu_id = i;
		break;
	}

	if (pthread_spin_unlock(&kvm->lock)) {
		die("unlocked failed");
	}

	if (cpu_id == -1) {
		die("No more vcpu to allocate\n");
	}

	vcpu = calloc(1, sizeof(struct kvm_cpu));
	if (!vcpu)
		return NULL;

	kvm->vcpu_pool[cpu_id].vcpu = vcpu;
	vcpu->vm = kvm;
	vcpu->cpu_id = cpu_id;

	vcpu->vcpu_fd = ioctl(vcpu->vm->vm_fd, KVM_CREATE_VCPU, vcpu->cpu_id);
	if (vcpu->vcpu_fd < 0)
		die("KVM_CREATE_VCPU ioctl");

	vcpu->kvm_run = mmap(NULL, kvm->kvm_run_mmap_size, PROT_RW, MAP_SHARED,
			     vcpu->vcpu_fd, 0);
	if (vcpu->kvm_run == MAP_FAILED)
		die("unable to mmap vcpu fd");

	return vcpu;
}

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif

#define BUILD_ASSERT(cond)                                                     \
	do {                                                                   \
		(void)sizeof(char[1 - 2 * !(cond)]);                           \
	} while (0)

extern void switch_stack(struct kvm_cpu *cpu, u64 host_stack);

void vacate_current_stack(struct kvm_cpu *cpu)
{
	void *host_stack = mmap_one_page();
	switch_stack(cpu, (u64)host_stack + PAGESIZE);
}

int kvm_cpu__start(struct kvm_cpu *cpu, struct kvm_regs *regs)
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

	if (init_cp0(cpu) < 0) {
		pr_err("init_cp0 failed");
		return -1;
	}

	if (init_fpu(cpu) < 0) {
		pr_err("init_fpu failed");
		return -1;
	}

	if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, regs) < 0) {
		pr_err("KVM_SET_REGS failed");
		return -1;
	}

	vacate_current_stack(cpu);
	die("host never reach here\n");
guest_entry:
	return 0; // 不能去掉，否则 guest_entry 后面没有语句，会报错
}

struct kvm_cpu *kvm_init_vm_with_one_cpu()
{
	char dev_path[] = "/dev/kvm";
	int ret;
	struct kvm_vm *vm;

	vm = calloc(1, sizeof(struct kvm_vm));

	vm->sys_fd = -1;
	vm->vm_fd = -1;
	vm->kvm_run_mmap_size = -1;

	if (pthread_spin_init(&vm->lock, PTHREAD_PROCESS_PRIVATE) != 0) {
		pr_err("pthread_spin_init failed\n");
		goto err;
	}

	for (int i = 0; i < KVM_MAX_VCPUS; ++i) {
		vm->vcpu_pool[i].valid = false;
		vm->vcpu_pool[i].vcpu = NULL;
	}

	ret = open(dev_path, O_RDWR);
	if (ret < 0) {
		pr_err("unable to open %s", dev_path);
		goto err;
	} else {
		vm->sys_fd = ret;
		pr_info("open %s", dev_path);
	}

	ret = ioctl(vm->sys_fd, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION) {
		pr_err("KVM_GET_API_VERSION");
		goto err_sys_fd;
	} else {
		pr_info("KVM_GET_API_VERSION");
	}

	ret = ioctl(vm->sys_fd, KVM_CREATE_VM, KVM_VM_TYPE);
	if (ret < 0) {
		pr_err("KVM_CREATE_VM");
		goto err_sys_fd;
	} else {
		vm->vm_fd = ret;
		pr_info("KVM_CREATE_VM");
	}

	int mmap_size = ioctl(vm->sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
	if (mmap_size < 0)
		die("KVM_GET_VCPU_MMAP_SIZE");
	vm->kvm_run_mmap_size = mmap_size;

	struct kvm_userspace_memory_region mem =
		(struct kvm_userspace_memory_region){
			.slot = 0,
			.flags = 0,
			.guest_phys_addr = 0,
			.memory_size = (u64)(1) << 40,
			.userspace_addr = 0,
		};

	ret = ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
	if (ret < 0) {
		pr_err("KVM_SET_USER_MEMORY_REGION");
		goto err_vm_fd;
	} else {
		pr_info("KVM_SET_USER_MEMORY_REGION");
	}

	return kvm_alloc_vcpu(vm);

err_vm_fd:
	close(vm->vm_fd);
err_sys_fd:
	close(vm->vm_fd);
err:
	pr_err("setup_vm_with_one_cpu");
	return NULL;
}

int dune_enter()
{
	struct kvm_regs regs;
	BUILD_ASSERT(272 == offsetof(struct kvm_regs, pc));
	struct kvm_cpu *cpu = kvm_init_vm_with_one_cpu();
	if (cpu == NULL)
		return -1;
	if (kvm_cpu__start(cpu, &regs))
		return -1;

	// exit(guest_clone());
	// exit(guest_fork());
	// exit(guest_syscall());
	return 0;
}

/** 
 * copied form : https://github.com/torvalds/linux/blob/master/kernel/fork.c
 *
 * SYSCALL_DEFINE5(clone,
 * unsigned long, clone_flags,
 * unsigned long, newsp,
 * int __user *, parent_tidptr,
 * int __user *, child_tidptr,
 * unsigned long, tls)
 *
 * SYSCALL_DEFINE2(clone3, struct clone_args __user *, uargs, size_t, size)
 */

struct clone3_args {
	u64 flags; /* Flags bit mask */
	u64 pidfd; /* Where to store PID file descriptor
                                    (pid_t *) */
	u64 child_tid; /* Where to store child TID,
                                    in child's memory (pid_t *) */
	u64 parent_tid; /* Where to store child TID,
                                    in parent's memory (int *) */
	u64 exit_signal; /* Signal to deliver to parent on
                                    child termination */
	u64 stack; /* Pointer to lowest byte of stack */
	u64 stack_size; /* Size of stack */
	u64 tls; /* Location of new TLS */
	u64 set_tid; /* Pointer to a pid_t array */
	u64 set_tid_size; /* Number of elements in set_tid */
};

int dup_fpu(struct kvm_cpu *child_cpu, const struct mips_fpu_struct *parent_fpu)
{
	if (kvm_enable_fpu(child_cpu)) {
		return -1;
	}

	if (kvm_set_fpu_regs(child_cpu, parent_fpu)) {
		return -1;
	}
	return 0;
}

// TODO do we still need it ?
void dup_cp0(struct kvm_cpu *parent_cpu, struct kvm_cpu *child_cpu, u64 id)
{
	struct cp0_reg cp0_reg;
	cp0_reg.reg.addr = (u64) & (cp0_reg.v);
	cp0_reg.reg.id = id;

	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_ONE_REG, &(cp0_reg.reg)) < 0)
		die("KVM_GET_ONE_REG");

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_ONE_REG, &(cp0_reg.reg)) < 0)
		die("KVM_SET_ONE_REG");
}

void set_cp0(struct kvm_cpu *child_cpu, u64 id, u64 value)
{
	struct cp0_reg cp0_reg;
	cp0_reg.reg.addr = (u64) & (cp0_reg.v);
	cp0_reg.reg.id = id;
	cp0_reg.v = value;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_ONE_REG, &(cp0_reg.reg)) < 0)
		die("KVM_SET_ONE_REG");
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
		die("TODO : support clone3");
	}

	// child start at next instruction of syscall
	child_regs.pc = parent_cpu->info.epc + 4;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &child_regs) < 0)
		die("KVM_SET_REGS");

	// TODO 处理返回值
	// 唯一引用位置
	if (dup_fpu(child_cpu, &parent_cpu->info.fpu) < 0) {
		die("dup_fpu\n");
	}

	// TODO 这个返回值处理一下
	if (init_cp0(child_cpu) < 0) {
		die("init_cp0\n");
	}
}

struct kvm_cpu *dup_vcpu(const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_cpu *child_cpu = kvm_alloc_vcpu(parent_cpu->vm);
	if (child_cpu == NULL)
		return NULL;

	ebase_share(child_cpu, parent_cpu);

	// TODO 这个函数的返回值处理一下
	init_child_thread_info(child_cpu, parent_cpu, sysno);

	return child_cpu;
}

typedef void (*CHILD_ENTRY_PTR)(struct kvm_cpu *cpu);

void child_entry(struct kvm_cpu *child_cpu)
{
	vacate_current_stack(child_cpu);
}

extern u64 dune_clone(u64 r4, u64 r5, u64 r6, u64 r7, u64 r8, u64 r9);

void emulate_fork_by_another_vcpu(struct kvm_cpu *parent_cpu)
{
	u64 r4 = parent_cpu->syscall_parameter[1];
	u64 r5 = parent_cpu->syscall_parameter[2];
	u64 r6 = parent_cpu->syscall_parameter[3];
	u64 r7 = parent_cpu->syscall_parameter[4];
	u64 r8 = parent_cpu->syscall_parameter[5];
	u64 r9 = parent_cpu->syscall_parameter[6];
	// parent 原路返回，child 进入到 child_entry 中间
	long child_pid = dune_clone(r4, r5, r6, r7, r8, r9);

	if (child_pid > 0) {
		parent_cpu->syscall_parameter[0] = child_pid;
		parent_cpu->syscall_parameter[4] = 0;
	} else {
		parent_cpu->syscall_parameter[0] = -child_pid;
		parent_cpu->syscall_parameter[4] = 1;
	}
}

struct child_args {
	CHILD_ENTRY_PTR entry;
	struct kvm_cpu *cpu;
};

bool is_vm_shared(const struct kvm_cpu *parent_cpu, int sysno)
{
	if (sysno == SYS_FORK)
		return false;

	// If CLONE_VM is set, the calling process and the child process run in the same memory  space.
	if (sysno == SYS_CLONE)
		return parent_cpu->syscall_parameter[1] & CLONE_VM;

	if (sysno == SYS_CLONE3) {
		struct clone3_args *args =
			(struct clone3_args *)(parent_cpu->syscall_parameter[1]);
		return args->flags | CLONE_VM;
	}

	die("unexpected sysno");
}

// TODO 如果 fork 成功，在 child 中间 dup_vm 却失败, 其返回值也是 NULL
struct kvm_cpu *dup_vm(const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_cpu *child_cpu = kvm_init_vm_with_one_cpu();
	if (child_cpu == NULL) {
		die("dup_vm");
	}

	init_child_thread_info(child_cpu, parent_cpu, sysno);
	return child_cpu;
}

struct kvm_cpu *emulate_fork_by_two_vm(struct kvm_cpu *parent_cpu, int sysno)
{
	if (do_syscall6(parent_cpu, true)) {
		// TODO 如果 fork 成功，在 child 中间 dup_vm 却失败, 其返回值也是 NULL
		return dup_vm(parent_cpu, sysno);
	}
	// parent return null
	return NULL;
}

// TODO 如果 clone 失败，记得回收这些资源
struct kvm_cpu *emulate_fork_by_two_vcpu(struct kvm_cpu *parent_cpu, int sysno)
{
	// without CLONE_VM
	// 1. creating one vcpu is enough
	// 2. child host need one stack for `host_loop`
	struct kvm_cpu *child_cpu = dup_vcpu(parent_cpu, sysno);
	if (child_cpu == NULL)
		die("DUP_VCPU");

	if (sysno == SYS_CLONE) {
		// check musl/src/thread/mips64/clone.s to understand code below
		u64 child_stack_pointer = parent_cpu->syscall_parameter[2];
		/*
     * |dune child_args |musl funcp/argp|
     * |----------------|---------------|
     * ^                ^               ^
     * +                +               +
     * host           guest            user
     * syscall        syscall         syscall
     * paramerter    paramerter     paramerter
     *                  ||
     *               guest sp
     */
		child_stack_pointer &= -16; // # aligning stack to double word
		child_stack_pointer += -16;
		assert(sizeof(struct child_args) == 16);
		struct child_args *child_args_on_stack_top =
			(struct child_args *)(child_stack_pointer);
		child_args_on_stack_top->entry = child_entry;
		child_args_on_stack_top->cpu = child_cpu;
		parent_cpu->syscall_parameter[2] = child_stack_pointer;

		emulate_fork_by_another_vcpu(parent_cpu);
	}

	else if (sysno == SYS_CLONE3) {
		die("Unable test clone3 with 4.19 kernel\n");
		die("code below doesn't work, reference how SYS_CLONE works\n");
		struct clone3_args *args =
			(struct clone3_args *)(parent_cpu->syscall_parameter[1]);
		if (args->stack != 0) {
			emulate_fork_by_another_vcpu(parent_cpu);
		}
	}

	// TODO 通过返回 NULL 告知是 parent, 但是需要使用基本函数告知
	return NULL;
}

// sysno == SYS_FORK || sysno == SYS_CLONE || sysno == SYS_CLONE3
struct kvm_cpu *emulate_fork(struct kvm_cpu *parent_cpu, int sysno)
{
	if (is_vm_shared(parent_cpu, sysno))
		return emulate_fork_by_two_vcpu(parent_cpu, sysno);
	else
		return emulate_fork_by_two_vm(parent_cpu, sysno);
}

bool do_syscall6(struct kvm_cpu *cpu, bool is_fork)
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

void host_loop(struct kvm_cpu *vcpu)
{
	while (true) {
		long err = ioctl(vcpu->vcpu_fd, KVM_RUN, 0);
		long sysno = vcpu->syscall_parameter[0];
		struct kvm_regs regs;

		if (err < 0 && (errno != EINTR && errno != EAGAIN)) {
			die("KVM_RUN : err=%d\n", err);
		}

		if (vcpu->kvm_run->exit_reason == KVM_EXIT_INTR) {
			continue;
		}

		if (vcpu->kvm_run->exit_reason != KVM_EXIT_HYPERCALL) {
			// TODO 将错误的内容枚举一下?
			die("KVM_EXIT_IS_NOT_HYPERCALL vcpu=%d exit_reason=%d",
			    vcpu->cpu_id, vcpu->kvm_run->exit_reason);
		}

		if (sysno == SYS_EXECVE || sysno == SYS_EXECLOAD ||
		    sysno == SYS_EXECLOAD)
			die("Unsupported syscall");

		// exit_group will destroy the vm, so don't bother to remove vcpu
		if (sysno == SYS_EXIT) {
			kvm_free_vcpu(vcpu);
		}

		if (sysno == SYS_FORK || sysno == SYS_CLONE ||
		    sysno == SYS_CLONE3) {
			kvm_get_parent_thread_info(vcpu);

			struct kvm_cpu *child_cpu = emulate_fork(vcpu, sysno);
			// 在 guest 态中间，child 的 pc 指向 fork / clone 的下一条指令的位置,
			// cp0 被初始化为默认状态。 而 parent 需要像完成普通 syscall 一样，
			// 进行调整 status 和 pc 寄存器。
			if (child_cpu) {
				vcpu = child_cpu;
				continue;
			}
		} else if (sysno == SYS_PIPE) {
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
		} else {
			do_syscall6(vcpu, false);
		}
	}
}

// TODO mabye a special hypercall which help process escape the dune
void escape()
{
}
