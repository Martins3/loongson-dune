#include <sys/mman.h>
#include <stddef.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cp0.h"

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

struct kvm {
	int sys_fd;
	int vm_fd;
};

// reference : /home/maritns3/core/tool/mips/include/kvm/kvm-cpu-arch.h
struct kvm_cpu {
	unsigned long cpu_id;
	struct kvm *kvm;
	int vcpu_fd; /* For VCPU ioctls() */
	struct kvm_run *kvm_run;
	struct kvm_regs regs;
};

void die_perror(const char *s)
{
	perror(s);
	exit(1);
}

static void report(const char *prefix, const char *err, va_list params)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, " %s%s\n", prefix, msg);
}

static void error_builtin(const char *err, va_list params)
{
	report(" Error: ", err, params);
}

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

static NORETURN void die_builtin(const char *err, va_list params)
{
	report(" Fatal: ", err, params);
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

void kvm_cpu__show_registers(int vcpu_fd, int debug_fd)
{
	struct kvm_regs regs;

	if (ioctl(vcpu_fd, KVM_GET_REGS, &regs) < 0)
		die("KVM_GET_REGS failed");
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
	dprintf(debug_fd, "epc  : %016llx\n", (unsigned long long)regs.pc);

	dprintf(debug_fd, "\n");
}

static int kvm__run_guest()
{
	// 1. 使用内联汇编将 gpr 装入到数组中间
	//   1. kvm mips.c 的 guest.c 处理
	// 2. 调用 kvm__run_guest
	//
	// 3. 使用内联汇编将 从数组中间恢复，但是此时，已经是在 guest 态了
	return -errno;
}

// TODO 调查 hello kvm 的将字节导入到内核的方法 ?
// TLB rixi, TLB invalid 似乎也是不用考虑的
static int setup_syscall_routine()
{
	return -errno;
}

static int setup_tlb_routine()
{
	return -errno;
}

static int setup_cache_routine()
{
	return -errno;
}

struct cp0_reg {
	struct kvm_one_reg reg;
	u64 v;
};


static int init_cp0(struct kvm_cpu *cpu)
{
  int i;
  struct cp0_reg one_regs[] = { 
    { .reg = {.id = KVM_REG_MIPS_CP0_INDEX},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_RANDOM},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_ENTRYLO0},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_ENTRYLO1},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONTEXT},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONTEXTCONFIG},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_USERLOCAL},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_XCONTEXTCONFIG},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PAGEMASK},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PAGEGRAIN},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_SEGCTL0},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_SEGCTL1},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_SEGCTL2},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PWBASE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PWFIELD},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PWSIZE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_WIRED},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PWCTL},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_HWRENA},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_BADVADDR},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_BADINSTR},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_BADINSTRP},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_COUNT},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_ENTRYHI},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_COMPARE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_STATUS},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_INTCTL},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CAUSE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_EPC},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_PRID},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_EBASE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG1},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG2},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG3},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG4},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG5},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG6},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_CONFIG7},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_MAARI},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_XCONTEXT},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_GSCAUSE},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_ERROREPC},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH1},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH2},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH3},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH4},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH5},  .v = 12345678 },
    { .reg = {.id = KVM_REG_MIPS_CP0_KSCRATCH6},  .v = 12345678 },
  };

  for (i = 0; i < sizeof(one_regs)/sizeof(struct cp0_reg); ++i) {
    one_regs[i].reg.addr = (u64)&(one_regs[i].v);
  }

  for (i = 0; i < sizeof(one_regs)/sizeof(struct cp0_reg); ++i) {
    if (ioctl(cpu->vcpu_fd, KVM_SET_ONE_REG, &one_regs[i]) < 0)
      die_perror("KVM_SET_ONE_REG failed");
  }
	return -errno;
}

// TODO /home/maritns3/core/loongson-dune/cross/arch/mips/include/uapi/asm/kvm.h
// definition of `struct fpu` is empty
//
// TODO use fpu in guest will cause vm exit ?
static int init_fpu()
{
	return -errno;
}

static int init_simd()
{
	return -errno;
}

static int init_config()
{
	return -errno;
}

static int kvm__init_guest()
{
	int ret = 0;

	ret = init_cp0();
	if (ret < 0)
		return ret;

	ret = init_config();
	if (ret < 0)
		return ret;

	return ret;
}

int syscall_emulation(struct kvm_cpu *cpu)
{
	return -errno;
}

void kvm_cpu__run(struct kvm_cpu *vcpu)
{
	int err;

	err = ioctl(vcpu->vcpu_fd, KVM_RUN, 0);
	if (err < 0 && (errno != EINTR && errno != EAGAIN))
		die_perror("KVM_RUN");
}

int kvm_cpu__start(struct kvm_cpu *cpu)
{
	if (kvm__init_guest(cpu)) {
		pr_err("guest init\n");
	} else {
		pr_info("guest init\n");
	}

	while (true) {
		kvm_cpu__run(cpu);
		switch (cpu->kvm_run->exit_reason) {
		case KVM_EXIT_HYPERCALL: {
			if (syscall_emulation(cpu)) {
				pr_err("syscall emulation");
			}
			break;
		}
		default:
			die_perror(
				"TODO : there are so many exit reason that I didn't check");
		}
	}
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

struct kvm_cpu *kvm_cpu__init(struct kvm *kvm, unsigned long cpu_id)
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

	return vcpu;
}

static void kvm_cpu__setup_regs(struct kvm_cpu *vcpu)
{
	struct kvm_regs regs;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0)
		die_perror("KVM_SET_REGS failed");
}

// TODO 关于信号之类，需要从 guest 中间借鉴
// 而且需要提供两个入口，用于 fork
// 似乎，当使用上 kvm 的时候，就不用再特意处理 signal 了
//
// fork 的实现方式 : 创建另个 vcpu 出来，然后 kvm_run 其
// 并不是所有的进程都是需要放入到 dune 中间的
int kvm__init()
{
	char dev_path[] = "/dev/kvm";
	int ret;
	struct kvm dune;

	dune.sys_fd = -1;
	dune.vm_fd = -1;

	ret = open(dev_path, O_RDWR);
	if (ret < 0) {
		pr_err("unable to open %s", dev_path);
		goto err;
	} else {
		dune.sys_fd = ret;
		pr_info("open %s", dev_path);
	}

	ret = ioctl(dune.sys_fd, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION) {
		pr_err("KVM_GET_API_VERSION");
		goto err_sys_fd;
	} else {
		pr_info("KVM_GET_API_VERSION");
	}

	ret = ioctl(dune.sys_fd, KVM_CREATE_VM, KVM_VM_TYPE);
	if (ret < 0) {
		pr_err("KVM_CREATE_VM");
		goto err_sys_fd;
	} else {
		dune.vm_fd = ret;
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

	ret = ioctl(dune.vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
	if (ret < 0) {
		pr_err("KVM_SET_USER_MEMORY_REGION");
		goto err_vm_fd;
	} else {
		pr_info("KVM_SET_USER_MEMORY_REGION");
	}

	struct kvm_cpu *cpu = kvm_cpu__init(&dune, 0);
	kvm_cpu__start(cpu);

// TODO maybe just exit, no need to close them
err_vm_fd:
	close(dune.vm_fd);
err_sys_fd:
	close(dune.vm_fd);
err:
	return ret;
}

int main(int argc, char *argv[])
{
#ifndef LOONGSON
	die_perror("run it in loongson\n");
#endif
	if (kvm__init()) {
		pr_err("KVM failed");
	}

	return 0;
}
