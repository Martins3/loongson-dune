#ifndef ARCH_H_X86_64_PN9D4LWN
#define ARCH_H_X86_64_PN9D4LWN

#ifndef LOONGSON
#include "../linux-headers/kvm.h"
#else
#include <linux/kvm.h>
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define PAGESHIFT 12
#define PAGESIZE (1 << PAGESHIFT)
#define KVM_MAX_VCPUS 16
#define KVM_GUEST_MEMORY_SIZE (1ULL << 47)

struct x86_shared_state;

struct thread_info {
	struct kvm_regs regs;
	struct kvm_fpu fpu;
	struct x86_shared_state *shared;
	void *gdt;
	void *idt;
	void *tss;
	u64 fs_base;
	u64 gs_base;
};

#define SYS_CLONE 56
#define SYS_FORK 57
#define SYS_EXIT 60
#define SYS_MMAP 9
#define SYS_KEXEC_LOAD 246
#define SYS_ARCH_PRCTL 158
#define SYS_SET_TID_ADDRESS 218
#define SYS_CLONE3 435
#define SYS_GUEST_EXCEPTION (~0ULL)

#endif /* end of include guard: ARCH_H_X86_64_PN9D4LWN */
