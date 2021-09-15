#ifndef ARCH_H_BPXBLEPN
#define ARCH_H_BPXBLEPN

#ifndef LOONGSON
#include "../linux-headers/kvm.h"
#else
#include <linux/kvm.h>
#endif

#define PAGESHIFT 14
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define NUM_FPU_REGS 32

struct thread_info {
  struct kvm_regs regs;
  struct kvm_fpu fpu;

  u64 era;
  void *ebase;
};

#define KVM_MAX_VCPUS 16
#define PAGESIZE (1 << PAGESHIFT)

/**
 * copied from https://github.com/torvalds/linux/tree/master/include/uapi/asm-generic/unistd.h
 */
#define __NR_clone 220
#define __NR_exit 93
#define __NR_kexec_load 104
#define __NR_set_tid_address 96

#define SYS_CLONE __NR_clone
#define SYS_EXIT __NR_exit
#define SYS_KEXEC_LOAD __NR_kexec_load
#define SYS_SET_THREAD_AREA __NR_set_tid_address

#define SYS_CLONE3 0x3f3f3f3f
#define SYS_FORK 0x3f3f3f3f
#endif /* end of include guard: ARCH_H_BPXBLEPN */
