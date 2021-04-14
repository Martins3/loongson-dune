#ifndef INTERFACE_H_32DCD7PQ
#define INTERFACE_H_32DCD7PQ
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "config.h"

#if ARCH == MIPS_ARCH
#include "mips/arch.h"
#elif ARCH == LOONGSON_ARCH
#include "loongarch/arch.h"
#else
#error Unsupported Architecture
#endif


void pr_warn(const char *err, ...);
void pr_info(const char *info, ...);
void die(const char *err, ...);

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


struct kvm_cpu;
struct vcpu_pool_ele {
	bool valid;
	struct kvm_cpu *vcpu;
};

struct kvm_vm {
	int sys_fd;
	int vm_fd;
	int kvm_run_mmap_size;
#ifdef DUNE_DEBUG
	int debug_fd;
#endif

	struct vcpu_pool_ele vcpu_pool[KVM_MAX_VCPUS];
	pthread_spinlock_t lock;
};

// reference : kvmtool/mips/include/kvm/kvm-cpu-arch.h
struct kvm_cpu {
	int cpu_id;
	struct kvm_vm *vm;
	int vcpu_fd; /* For VCPU ioctls() */
	struct kvm_run *kvm_run;
	u64 syscall_parameter[7]; // TODO 为什么是 7, linux 对于这个限制是什么?

  // architecture specified vm state
	struct thread_info info;
};

#define PROT_RWX (PROT_READ | PROT_WRITE | PROT_EXEC)
#define PROT_RW (PROT_READ | PROT_WRITE)
#define MAP_ANON_NORESERVE (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE)

static inline void *mmap_one_page()
{
	void *addr = mmap(NULL, PAGESIZE, PROT_RWX, MAP_ANON_NORESERVE, -1, 0);
	if (addr == NULL && ((u64)addr & 0xffff) != 0)
		die("mmap_one_page");
	return addr;
}

void vacate_current_stack(struct kvm_cpu *cpu);
void *mmap_one_page();

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

void arch_dune_enter(struct kvm_cpu *cpu);
void switch_stack(struct kvm_cpu *cpu, u64 host_stack);
bool do_syscall6(struct kvm_cpu *cpu, bool is_fork);
void child_entry(struct kvm_cpu *cpu);
void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu);
void init_child_thread_info(struct kvm_cpu *child_cpu,
			    const struct kvm_cpu *parent_cpu, int sysno);
void arch_handle_tls(struct kvm_cpu *vcpu);
bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno);
u64 dune_clone(u64 r4, u64 r5, u64 r6, u64 r7, u64 r8, u64 r9);
void escape(); // TODO
/**
 * History:        #0
 * Commit:         e08b96371625aaa84cb03f51acc4c8e0be27403a
 * Author:         Carsten Otte <cotte@de.ibm.com>
 * Committer:      Avi Kivity <avi@redhat.com>
 * Author Date:    Wed 04 Jan 2012 05:25:20 PM CST
 * Committer Date: Mon 05 Mar 2012 08:52:18 PM CST

 * KVM: s390: add parameter for KVM_CREATE_VM

 * This patch introduces a new config option for user controlled kernel
 * virtual machines. It introduces a parameter to KVM_CREATE_VM that
 * allows to set bits that alter the capabilities of the newly created
 * virtual machine.
 * The parameter is passed to kvm_arch_init_vm for all architectures.
 * The only valid modifier bit for now is KVM_VM_S390_UCONTROL.
 * This requires CAP_SYS_ADMIN privileges and creates a user controlled
 * virtual machine on s390 architectures.

 * Signed-off-by: Carsten Otte <cotte@de.ibm.com>
 * Signed-off-by: Marcelo Tosatti <mtosatti@redhat.com>
 * Signed-off-by: Avi Kivity <avi@redhat.com>
 */
#define KVM_VM_TYPE 0

#endif /* end of include guard: INTERFACE_H_32DCD7PQ */
