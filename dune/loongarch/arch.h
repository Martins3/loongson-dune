#ifndef ARCH_H_BPXBLEPN
#define ARCH_H_BPXBLEPN

// TODO define the syscall no

struct thread_info {
  // nothing
};

// TODO
#define KVM_MAX_VCPUS 10000
#define PAGESIZE (1 << 100)

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

#endif /* end of include guard: ARCH_H_BPXBLEPN */
