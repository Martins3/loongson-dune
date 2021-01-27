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
#include "cp0.h"
#include "syscall_arch.h"

#include <sys/mman.h>
#include <sys/ioctl.h>

#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>

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
// /home/maritns3/core/loongson-dune/cross/arch/mips/kvm/mips.c:kvm_arch_init_vm
#define KVM_VM_TYPE 1

// TODO protect this global variable
int vm_serial_number = 0;

struct kvm {
	int sys_fd;
	int vm_fd;
	int vm_id; // TODO maybe shmem and memlock
};

struct thread_info {
	struct kvm_regs regs;
	u64 epc;
	// simd
	// fpu
};

// reference : /home/maritns3/core/tool/mips/include/kvm/kvm-cpu-arch.h
struct kvm_cpu {
	unsigned long cpu_id;
	struct kvm *kvm;
	int vcpu_fd; /* For VCPU ioctls() */
	struct kvm_run *kvm_run;
	void *ebase;
	long syscall_parameter[7];
	int debug_fd;
};

struct kvm_cpu *setup_vm_with_one_cpu();

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

void die_perror(const char *s)
{
	perror(s);
	exit(1);
}

static void report(const char *prefix, const char *err, va_list params)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stdout, " %s%s\n", prefix, msg);
}

static void error_builtin(const char *err, va_list params)
{
	printf("%s", KCYN);
	report(" Error: ", err, params);
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

void dump_kvm_regs(int debug_fd, struct kvm_regs regs)
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

void dune_show_registers(int vcpu_fd, int debug_fd)
{
	struct kvm_regs regs;

	if (ioctl(vcpu_fd, KVM_GET_REGS, &regs) < 0)
		die("KVM_GET_REGS : show_registers");
	dump_kvm_regs(debug_fd, regs);
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
		die_perror("mmap_one_page");
	return addr;
}

static void alloc_ebase(struct kvm_cpu *cpu)
{
	int i;
	void *addr = mmap_one_page();
	cpu->ebase = addr;
	for (int i = 0; i < PAGESIZE / 4; ++i) {
		int *x = (int *)addr;
		x = x + i;
		*x = (0x42000028 + (0x14 << 11));
	}

	assert((void *)ebase_tlb_entry_end - (void *)ebase_error_entry_begin <
	       (EBASE_CACHE_OFFSET - EBASE_XTLB_OFFSET));
	pr_info("ebase address : %llx", (u64)addr);
}

static int init_ebase_tlb(struct kvm_cpu *cpu)
{
	memcpy(cpu->ebase + EBASE_TLB_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static int init_ebase_xtlb(struct kvm_cpu *cpu)
{
	memcpy(cpu->ebase + EBASE_XTLB_OFFSET, ebase_tlb_entry_begin,
	       ebase_tlb_entry_end - ebase_tlb_entry_begin);
	return 0;
}

static int init_ebase_cache(struct kvm_cpu *cpu)
{
	memcpy(cpu->ebase + EBASE_CACHE_OFFSET, ebase_error_entry_begin,
	       ebase_error_entry_end - ebase_error_entry_begin);
}

static int init_ebase_general(struct kvm_cpu *cpu)
{
	extern void ebase_general_entry_begin(void);
	extern void ebase_general_entry_end(void);
	memcpy(cpu->ebase + EBASE_GE_OFFSET, ebase_general_entry_begin,
	       ebase_general_entry_end - ebase_general_entry_begin);
	return 0;
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

void init_ebase(struct kvm_cpu *cpu)
{
	alloc_ebase(cpu);
	init_ebase_tlb(cpu);
	init_ebase_xtlb(cpu);
	init_ebase_cache(cpu);
	init_ebase_general(cpu);
}

// TODO 验证一下
// cpu_guest_has_contextconfig
// cpu_guest_has_segments
// cpu_guest_has_maar && !cpu_guest_has_dyn_maar
static int init_cp0(struct kvm_cpu *cpu)
{
	u64 INIT_VALUE_EBASE = (u64)cpu->ebase + MIPS_XKPHYSX_CACHED;
	u64 INIT_VALUE_USERLOCAL = get_tp();

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
			die_perror("KVM_SET_ONE_REG");
		} else {
			// pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
			// one_regs[i].v);
		}
	}
	return 0;
}

// TODO /home/maritns3/core/loongson-dune/cross/arch/mips/include/uapi/asm/kvm.h
// definition of `struct fpu` is empty
//
// TODO use fpu in guest will cause vm exit ?
//
// TODO 当前 qemu 使用的 fpu 是不是需要拷贝到 guest 中间 ?
static int init_fpu()
{
	return -errno;
}

static int init_simd()
{
	return -errno;
}

static int kvm__init_guest(struct kvm_cpu *cpu)
{
	init_ebase(cpu);

	if (init_cp0(cpu) < 0)
		return -errno;

	init_fpu();

	init_simd();

	return 0;
}

static struct kvm_cpu *kvm_cpu__new(struct kvm *kvm)
{
	struct kvm_cpu *vcpu;

	vcpu = calloc(1, sizeof(*vcpu));
	if (!vcpu)
		return NULL;

	vcpu->kvm = kvm;

	return vcpu;
}

struct kvm_cpu *kvm_cpu__init(struct kvm *kvm, int cpu_id)
{
	struct kvm_cpu *vcpu;
	int mmap_size;

	vcpu = kvm_cpu__new(kvm);
	if (!vcpu)
		return NULL;

	vcpu->cpu_id = cpu_id;

	vcpu->vcpu_fd = ioctl(vcpu->kvm->vm_fd, KVM_CREATE_VCPU, cpu_id);
	if (vcpu->vcpu_fd < 0)
		die_perror("KVM_CREATE_VCPU ioctl");

	mmap_size = ioctl(vcpu->kvm->sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
	if (mmap_size < 0)
		die_perror("KVM_GET_VCPU_MMAP_SIZE");

	vcpu->kvm_run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
			     MAP_SHARED, vcpu->vcpu_fd, 0);
	if (vcpu->kvm_run == MAP_FAILED)
		die("unable to mmap vcpu fd");

	// TODO remove the debug related code when finished
	char name[40];
	memset(name, 0, sizeof(name));
	snprintf(name, 40, "%d-%d-syscall.txt", kvm->vm_id, cpu_id);
	vcpu->debug_fd = open(name, O_TRUNC | O_WRONLY | O_CREAT, 0644);
	if (vcpu->debug_fd == -1) {
		perror("open failed");
		exit(1);
	}

	return vcpu;
}

void kvm_cpu__run(struct kvm_cpu *vcpu)
{
	int err;

	err = ioctl(vcpu->vcpu_fd, KVM_RUN, 0);
	// TODO why only this ioctl check errno with EINTR and EAGAIN
	if (err < 0 && (errno != EINTR && errno != EAGAIN))
		die_perror("KVM_RUN");
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

	if (kvm__init_guest(cpu) < 0) {
		die_perror("guest init\n");
	}

	if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, regs) < 0)
		die_perror("KVM_SET_REGS failed");

	vacate_current_stack(cpu);
guest_entry:
	return 0;
}

// TODO we need a better machanism to deal with cpu_id
struct kvm_cpu *setup_vm_with_one_cpu(int cpu_id)
{
	char dev_path[] = "/dev/kvm";
	int ret;
	struct kvm *dune;

	dune = calloc(1, sizeof(dune));

	dune->sys_fd = -1;
	dune->vm_fd = -1;
	// TODO if we want to keep a accurate vm_id
	// maybe shmem and mutex is necessary
	dune->vm_id = 100;

	ret = open(dev_path, O_RDWR);
	if (ret < 0) {
		pr_err("unable to open %s", dev_path);
		goto err;
	} else {
		dune->sys_fd = ret;
		pr_info("open %s", dev_path);
	}

	ret = ioctl(dune->sys_fd, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION) {
		pr_err("KVM_GET_API_VERSION");
		goto err_sys_fd;
	} else {
		pr_info("KVM_GET_API_VERSION");
	}

	ret = ioctl(dune->sys_fd, KVM_CREATE_VM, KVM_VM_TYPE);
	if (ret < 0) {
		pr_err("KVM_CREATE_VM");
		goto err_sys_fd;
	} else {
		dune->vm_fd = ret;
		pr_info("KVM_CREATE_VM");
	}

	struct kvm_userspace_memory_region mem =
		(struct kvm_userspace_memory_region){
			.slot = 0,
			.flags = 0,
			.guest_phys_addr = 0,
			.memory_size = (u64)(1) << 40,
			.userspace_addr = 0,
		};

	ret = ioctl(dune->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
	if (ret < 0) {
		pr_err("KVM_SET_USER_MEMORY_REGION");
		goto err_vm_fd;
	} else {
		pr_info("KVM_SET_USER_MEMORY_REGION");
	}

	return kvm_cpu__init(dune, cpu_id);

err_vm_fd:
	close(dune->vm_fd);
err_sys_fd:
	close(dune->vm_fd);
err:
	die_perror("setup_vm_with_one_cpu");
	return NULL;
}

int guest_clone();
int guest_fork();
int guest_execution();
int guest_syscall()
{
	for (int i = 0; i < 10000; ++i) {
		printf("a\n");
	}
	return 1;
}

// TODO 关于信号之类，需要从 guest 中间借鉴
// 而且需要提供两个入口，用于 fork
// 似乎，当使用上 kvm 的时候，就不用再特意处理 signal 了
int dune_enter()
{
	struct kvm_regs regs;
	memset(&regs, 0, sizeof(struct kvm_regs));
	BUILD_ASSERT(272 == offsetof(struct kvm_regs, pc));
	struct kvm_cpu *cpu = setup_vm_with_one_cpu(0);
	if (cpu == NULL)
		return -errno;
	kvm_cpu__start(cpu, &regs);
	// exit(guest_execution());
	// exit(guest_clone());
	exit(guest_fork());
	// exit(guest_syscall());
}

int guest_fork()
{
	printf("fork you\n");
	long len = printf("liyawei\n");
	printf("ret : %ld\n", len);

	pid_t pid = fork();

	switch (pid) {
	case -1:
		printf("fork failed");
		break;
	case 0:
		printf("thsi is child\n");
		break;
	default:
		printf("this is parent\n");
		break;
	}
	return 12;
}

/** 
 * copied form : https://github.com/torvalds/linux/blob/master/kernel/fork.c
 *
 * SYSCALL_DEFINE5(clone, unsigned long, clone_flags, unsigned long, newsp,
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

bool do_syscall6(struct kvm_cpu *cpu, bool is_fork);

void dup_fpu(struct kvm_cpu *child_cpu, struct thread_info *info)
{
	// TODO
}

void dup_simd(struct kvm_cpu *child_cpu, struct thread_info *info)
{
	// TODO
}

enum ACCESS_CPU_OP {
	GET = KVM_GET_ONE_REG,
	SET = KVM_SET_ONE_REG,
};

u64 access_one_reg(struct kvm_cpu *cpu, u64 id, enum ACCESS_CPU_OP op,
		   u64 value)
{
	struct cp0_reg cp0_reg;
	cp0_reg.reg.addr = (u64) & (cp0_reg.v);
	cp0_reg.reg.id = id;
	cp0_reg.v = (op == GET) ? 0 : value;

	if (ioctl(cpu->vcpu_fd, op, &(cp0_reg.reg)) < 0)
		die_perror("KVM_GET_ONE_REG");

	return cp0_reg.v;
}

u64 get_cpu_one_reg(struct kvm_cpu *cpu, u64 id)
{
	return access_one_reg(cpu, id, GET, 0);
}

u64 set_cpu_one_reg(struct kvm_cpu *cpu, u64 id, u64 v)
{
	return access_one_reg(cpu, id, SET, v);
}

void copy_cp0(struct kvm_cpu *parent_cpu, struct kvm_cpu *child_cpu, u64 id)
{
	struct cp0_reg cp0_reg;
	cp0_reg.reg.addr = (u64) & (cp0_reg.v);
	cp0_reg.reg.id = id;

	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_ONE_REG, &(cp0_reg.reg)) < 0)
		die_perror("KVM_GET_ONE_REG");

	printf("guest syscall epc %llx", cp0_reg.v);

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_ONE_REG, &(cp0_reg.reg)) < 0)
		die_perror("KVM_SET_ONE_REG");
}

void set_cp0(struct kvm_cpu *child_cpu, u64 id, u64 value)
{
	struct cp0_reg cp0_reg;
	cp0_reg.reg.addr = (u64) & (cp0_reg.v);
	cp0_reg.reg.id = id;
	cp0_reg.v = value;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_ONE_REG, &(cp0_reg.reg)) < 0)
		die_perror("KVM_SET_ONE_REG");
}

void get_parent_thread_info(struct kvm_cpu *parent_cpu,
			    struct thread_info *info)
{
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(info->regs)) < 0)
		die_perror("KVM_GET_REGS");

	info->epc = get_cpu_one_reg(parent_cpu, KVM_REG_MIPS_CP0_EPC);

	// TODO fpu

	// TODO simd
}

void init_child_thread_info(struct kvm_cpu *child_cpu, struct thread_info *info,
			    u64 child_sp)
{
	info->regs.gpr[2] = 0;
	info->regs.gpr[7] = 0;
	if (child_sp)
		info->regs.gpr[29] = child_sp; //  #define sp	$29

	info->regs.pc = info->epc + 4;

	if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &(info->regs)) < 0)
		die_perror("KVM_SET_REGS");

	dup_fpu(child_cpu, info);

	dup_simd(child_cpu, info);

	if (init_cp0(child_cpu) < 0)
		return;
}

void share_ebase(struct kvm_cpu *parent_cpu, struct kvm_cpu *child_cpu)
{
	child_cpu->ebase = parent_cpu->ebase;
}

struct kvm_cpu *dup_vcpu(struct kvm_cpu *parent_cpu, int sysno)
{
	// FIXME
	// 1. cpu_id should be accessed excludsively
	// 2. I don't know how linux kernel use cpu_id, it should lower than something
	// it's a identifier for ?
	struct kvm_cpu *child_cpu =
		kvm_cpu__init(parent_cpu->kvm, parent_cpu->cpu_id + 1);
	share_ebase(parent_cpu, child_cpu);

	struct thread_info info;

	u64 child_sp = 0;
	if (sysno == SYS_CLONE) {
		child_sp = parent_cpu->syscall_parameter[2];
	} else {
		die_perror("TODO : support clone3");
	}

	get_parent_thread_info(parent_cpu, &info);

	init_child_thread_info(child_cpu, &info, child_sp);

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

bool is_vm_shared(struct kvm_cpu *parent_cpu, int sysno)
{
	if (sysno == SYS_FORK)
		return true;

	if (sysno == SYS_CLONE)
		return parent_cpu->syscall_parameter[1] | CLONE_VM;

	if (sysno == SYS_CLONE3) {
		struct clone3_args *args =
			(struct clone3_args *)(parent_cpu->syscall_parameter[1]);
		return args->flags | CLONE_VM;
	}

	die_perror("unexpected sysno");
}

struct kvm_cpu *dup_vm(struct kvm_cpu *parent_cpu, struct thread_info *info)
{
	struct kvm_cpu *child_cpu = setup_vm_with_one_cpu(1);
	share_ebase(parent_cpu, child_cpu);

	init_child_thread_info(child_cpu, info, 0);
	return child_cpu;
}

struct kvm_cpu *emulate_fork_by_another_vm(struct kvm_cpu *parent_cpu)
{
	struct thread_info info;
	get_parent_thread_info(parent_cpu, &info);

	if (do_syscall6(parent_cpu, true)) {
		return dup_vm(parent_cpu, &info);
	}
	return NULL;
}

struct kvm_cpu *emulate_fork_share_vm(struct kvm_cpu *parent_cpu, int sysno)
{
	// without CLONE_VM
	// 1. creating one vcpu is enough
	// 2. child host need one stack for `host_loop`
	struct kvm_cpu *child_cpu = dup_vcpu(parent_cpu, sysno);
	if (child_cpu == NULL)
		die_perror("DUP_VCPU");

	if (sysno == SYS_CLONE) {
		// TODO move code block to dune_clone as close as possible
		u64 child_stack_pointer = parent_cpu->syscall_parameter[2];
		child_stack_pointer &= -16; // # aligning stack to double word
		child_stack_pointer -= 16;
		struct child_args *child_args_on_stack_top =
			(struct child_args *)(child_stack_pointer);
		child_args_on_stack_top->entry = child_entry;
		child_args_on_stack_top->cpu = child_cpu;
		parent_cpu->syscall_parameter[2] = child_stack_pointer;

		if (child_stack_pointer) {
			emulate_fork_by_another_vcpu(parent_cpu);
		} else {
			die_perror(
				"TODO : maybe do some check on the child_stack_pointer, segment fault is stupid");
		}
	}

	// TODO I can't test clone3 with 4.19 kernel
	// TODO It doesn't work, reference how SYS_CLONE works
	if (sysno == SYS_CLONE3) {
		struct clone3_args *args =
			(struct clone3_args *)(parent_cpu->syscall_parameter[1]);
		if (args->stack != 0) {
			emulate_fork_by_another_vcpu(parent_cpu);
		}
	}
}

// sysno == SYS_FORK || sysno == SYS_CLONE || sysno == SYS_CLONE3
// TODO maybe stack is invalid, just segment fault is too stupid
struct kvm_cpu *emulate_fork(struct kvm_cpu *parent_cpu, int sysno)
{
	if (is_vm_shared(parent_cpu, sysno))
		return emulate_fork_by_another_vm(parent_cpu);
	else
		return emulate_fork_share_vm(parent_cpu, sysno);
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

void host_loop(struct kvm_cpu *cpu)
{
	while (true) {
		long err = ioctl(cpu->vcpu_fd, KVM_RUN, 0);
		long sysno = cpu->syscall_parameter[0];

		if (err < 0 && (errno != EINTR && errno != EAGAIN)) {
			pr_info("err %d\n", err);
			die_perror("KVM_RUN");
		}

		if (cpu->kvm_run->exit_reason != KVM_EXIT_HYPERCALL) {
			pr_err("vcpu_id : %d", cpu->cpu_id);
			die_perror("KVM_EXIT_IS_NOT_HYPERCALL");
		}

		if (sysno == SYS_EXECVE | sysno == SYS_EXECLOAD |
		    sysno == SYS_EXECLOAD)
			die_perror("Unsupported syscall");

		if (sysno == SYS_FORK || sysno == SYS_CLONE ||
		    sysno == SYS_CLONE3) {
      struct kvm_cpu * child_cpu = emulate_fork(cpu, sysno);
      cpu = child_cpu != NULL ? child_cpu : cpu;
		} else {
			do_syscall6(cpu, false);
		}

		struct kvm_regs regs;
		if (ioctl(cpu->vcpu_fd, KVM_GET_REGS, &regs) < 0)
			die_perror("KVM_GET_REGS : host_loop");

		// dump_kvm_regs(cpu->debug_fd, regs);
    regs.gpr[2] = cpu->syscall_parameter[0];
		regs.gpr[7] = cpu->syscall_parameter[4];
		cpu->kvm_run->hypercall.ret = cpu->syscall_parameter[0]; // loongson kvm

		// dprintf(cpu->debug_fd, "syscall %ld return %lld %lld\n", sysno,
		// regs.gpr[2], regs.gpr[7]);

		u64 epc = get_cpu_one_reg(cpu, KVM_REG_MIPS_CP0_EPC);
		// dprintf(cpu->fd, "return address %llx\n", epc);
		// dprintf(cpu->fd, "pc %llx\n", regs.pc);
		regs.pc = epc + 4;
		// dprintf(cpu->debug_fd, "new pc %llx\n", regs.pc);

		u64 status = get_cpu_one_reg(cpu, KVM_REG_MIPS_CP0_STATUS);
		// dprintf(cpu->debug_fd, "status %llx\n", status);
		status = set_cpu_one_reg(cpu, KVM_REG_MIPS_CP0_STATUS,
					 status & (~STATUS_BIT_EXL));
		// dprintf(cpu->fd, "new status %llx\n", status);

		if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, &regs) < 0)
			die_perror("KVM_SET_REGS");
	}
}

int main(int argc, char *argv[])
{
#ifndef LOONGSON
	die_perror("run it in loongson\n");
#endif
	if (dune_enter()) {
		pr_err("KVM failed");
	}

	return 12;
}

// mabye a epecial hypercall can lead to escape the process
void escape()
{
}
