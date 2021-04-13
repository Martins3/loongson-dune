#ifndef ARCH_H_BPXBLEPN
#define ARCH_H_BPXBLEPN

#include "../aux.h"

#ifndef LOONGSON
#include "../linux-headers/kvm.h"
#else
#include <linux/kvm.h>
#endif

#define PAGESHIFT 14

// TODO define the syscall no
struct thread_info {
  // nothing
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
