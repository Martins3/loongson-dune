#ifndef ARCH_H_IXTSIDHV
#define ARCH_H_IXTSIDHV

#include "../aux.h"

#ifndef LOONGSON
#include "../linux-headers/kvm.h"
#else
#include <linux/kvm.h>
#endif

// reference from arch/mips/include/asm/processor.h
#define FPU_REG_WIDTH 256
#define NUM_FPU_REGS 32
#define PAGESHIFT 14

union fpureg {
	u32 val32[FPU_REG_WIDTH / 32];
	u64 val64[FPU_REG_WIDTH / 64];
};

struct mips_fpu_struct {
	union fpureg fpr[NUM_FPU_REGS];
	unsigned int fcr31;
	unsigned int msacsr;
};

struct thread_info {
	struct kvm_regs regs;
	struct mips_fpu_struct fpu;

	u64 epc;
	void *ebase;
};
#define KVM_MAX_VCPUS 16
#define PAGESIZE (1 << PAGESHIFT)

/**
 * copied from https://github.com/torvalds/linux/arch/mips/kernel/syscalls/syscall_n64.tbl
 * 55	n64	clone				__sys_clone
 * 56	n64	fork				__sys_fork
 * 57	n64	execve			sys_execve
 * 270 n64	kexec_load			sys_kexec_load
 * 316 n64	execveat			sys_execveat
 * 435 n64	clone3		__sys_clone3 
 * */
#define SYS_PIPE 5021
#define SYS_CLONE 5055
#define SYS_FORK 5056
#define SYS_EXECVE 5057
#define SYS_EXIT 5058
#define SYS_KEXEC_LOAD 5270
#define SYS_EXECVEAT 5316
#define SYS_CLONE3 5435
#define SYS_SET_THREAD_AREA 5242

#endif /* end of include guard: ARCH_H_IXTSIDHV */
