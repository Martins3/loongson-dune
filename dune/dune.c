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

#include <sys/time.h>
#include <sys/resource.h>

#include "aux.h"
#include "interface.h"

// https://stackoverflow.com/questions/22449342/clone-vm-undeclared-first-use-in-this-function
#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>
#include <pthread.h>

struct kvm_cpu *kvm_init_vm_with_one_cpu();

void kvm_free_vcpu(struct kvm_cpu *vcpu)
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

		kvm->vcpu_pool[i].valid = true;
		if (kvm->vcpu_pool[i].vcpu != NULL) {
			if (pthread_spin_unlock(&kvm->lock)) {
				die("unlocked failed");
			}
			return kvm->vcpu_pool[i].vcpu;
		} else {
			cpu_id = i;
			break;
		}
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

void vacate_current_stack(struct kvm_cpu *cpu)
{
	void *host_stack = mmap_one_page();
	switch_stack(cpu, (u64)host_stack + PAGESIZE);
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
		die("pthread_spin_init failed\n");
	}

	for (int i = 0; i < KVM_MAX_VCPUS; ++i) {
		vm->vcpu_pool[i].valid = false;
		vm->vcpu_pool[i].vcpu = NULL;
	}

	ret = open(dev_path, O_RDWR);
	if (ret < 0) {
		die("unable to open %s", dev_path);
	} else {
		vm->sys_fd = ret;
		pr_info("open %s", dev_path);
	}

	ret = ioctl(vm->sys_fd, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION) {
		die("KVM_GET_API_VERSION");
	} else {
		pr_info("KVM_GET_API_VERSION");
	}

	ret = ioctl(vm->sys_fd, KVM_CREATE_VM, KVM_VM_TYPE);
	if (ret < 0) {
		die("KVM_CREATE_VM");
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
		die("KVM_SET_USER_MEMORY_REGION");
	} else {
		pr_info("KVM_SET_USER_MEMORY_REGION");
	}

#ifdef DUNE_DEBUG
	vm->debug_fd = open("syscall.txt", O_TRUNC | O_WRONLY | O_CREAT, 0644);
	if (vm->debug_fd == -1) {
		perror("open failed");
		exit(1);
	}
#endif

	return kvm_alloc_vcpu(vm);
}

struct dune_procmap_entry {
	unsigned long begin;
	unsigned long end;
	long int offset;
	bool r; // Readable
	bool w; // Writable
	bool x; // Executable
	bool p; // Private (or shared)
	char *path;
	int type;
};

static u64 expand_stack_getrlimit()
{
	struct rlimit rlim;
	if (getrlimit(RLIMIT_STACK, &rlim) == -1)
		die("get rlimit failed");
	return rlim.rlim_cur;
}

static u64 expand_stack_get_stack_top()
{
	struct dune_procmap_entry e;

	char line[512];
	char path[256];
	unsigned int dev1, dev2, inode;
	char read, write, execute, private;

	FILE *map = fopen("/proc/self/maps", "r");
	if (map == NULL)
		die("Could not open /proc/self/maps!\n");

	if (setvbuf(map, NULL, _IOFBF, 8192))
		die("setvbuf");

	while (!feof(map)) {
		path[0] = '\0';
		if (fgets(line, 512, map) == NULL)
			break;
		sscanf((char *)&line, "%lx-%lx %c%c%c%c %lx %x:%x %d %s",
		       &e.begin, &e.end, &read, &write, &execute, &private,
		       &e.offset, &dev1, &dev2, &inode, path);
		if (strncmp(path, "[stack", 6) == 0) {
			return e.end;
		}
	}

	if (fclose(map))
		die("close file");

	die("stack entry not found in /proc/self/maps");
	return 0;
}

void expand_stack()
{
	u64 limit = expand_stack_getrlimit();
	u64 top = expand_stack_get_stack_top();
	pr_info("top = %llx, limit = %llx", top, limit);
	*(int *)(top - limit) = 0;
}

int dune_enter()
{
	expand_stack();
	struct kvm_cpu *cpu = kvm_init_vm_with_one_cpu();
	if (cpu == NULL)
		die("kvm_init_vm_with_one_cpu");

	arch_dune_enter(cpu);
	return 0;
}

struct kvm_cpu *dup_vcpu(const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_cpu *child_cpu = kvm_alloc_vcpu(parent_cpu->vm);
	if (child_cpu == NULL)
		return NULL;

	init_child_thread_info(child_cpu, parent_cpu, sysno);

	return child_cpu;
}

typedef void (*CHILD_ENTRY_PTR)(struct kvm_cpu *cpu);

extern u64 dune_clone(u64 r4, u64 r5, u64 r6, u64 r7, u64 r8, u64 r9);

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
  return false;
}

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

void child_entry(struct kvm_cpu *cpu);

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
		u64 child_host_stack = (u64)mmap_one_page() + PAGESIZE;
		child_host_stack += -(sizeof(struct child_args));
		assert(sizeof(struct child_args) == 16); // check dune_clone
		struct child_args *child_args_on_stack_top =
			(struct child_args *)(child_host_stack);
		child_args_on_stack_top->entry = child_entry;
		child_args_on_stack_top->cpu = child_cpu;

		emulate_fork_by_another_vcpu(parent_cpu, child_host_stack);
	}

	else if (sysno == SYS_CLONE3) {
		die("Unable test clone3 with 4.19 kernel\n");
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

void host_loop(struct kvm_cpu *vcpu)
{
	while (true) {
		long err = ioctl(vcpu->vcpu_fd, KVM_RUN, 0);
		u64 sysno = vcpu->syscall_parameter[0];
		struct kvm_regs regs;

		if (err < 0 && (errno != EINTR && errno != EAGAIN)) {
			die("KVM_RUN : err=%d\n", err);
		}

		if (vcpu->kvm_run->exit_reason == KVM_EXIT_INTR) {
			continue;
		}

		if (vcpu->kvm_run->exit_reason != KVM_EXIT_HYPERCALL) {
			die("KVM_EXIT_IS_NOT_HYPERCALL vcpu=%d exit_reason=%d",
			    vcpu->cpu_id, vcpu->kvm_run->exit_reason);
		}

		if (sysno == SYS_KEXEC_LOAD)
			die("Unsupported syscall");

#ifdef DUNE_DEBUG
		dprintf(vcpu->vm->debug_fd,
			"vcpu=%d sysno=%ld:  %llx %llx %llx %llx %llx %llx\n",
			vcpu->cpu_id, sysno, vcpu->syscall_parameter[1],
			vcpu->syscall_parameter[2], vcpu->syscall_parameter[3],
			vcpu->syscall_parameter[4], vcpu->syscall_parameter[5],
			vcpu->syscall_parameter[6]);
#endif

		// exit_group will destroy the vm, so don't bother to remove vcpu
		if (sysno == SYS_EXIT) {
			kvm_free_vcpu(vcpu);
		}

		if (sysno == SYS_SET_THREAD_AREA) {
			arch_handle_tls(vcpu);
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
			}
			continue;
		}

		if (arch_handle_special_syscall(vcpu, sysno))
			continue;

		do_syscall6(vcpu, false);
	}
}
