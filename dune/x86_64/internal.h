#ifndef INTERNAL_H_X86_64_Z1SGY4TR
#define INTERNAL_H_X86_64_Z1SGY4TR

#define DUNE_SYSCALL_PORT 0x1000

#define DUNE_SYSCALL_ARG0 0
#define DUNE_SYSCALL_ARG1 8
#define DUNE_SYSCALL_ARG2 16
#define DUNE_SYSCALL_ARG3 24
#define DUNE_SYSCALL_ARG4 32
#define DUNE_SYSCALL_ARG5 40
#define DUNE_SYSCALL_ARG6 48
#define DUNE_SYSCALL_NR 56
#define DUNE_SYSCALL_RET_RIP 64
#define DUNE_SYSCALL_RET_RFLAGS 72
#define DUNE_SYSCALL_RESUME_RIP 80
#define DUNE_EXCEPTION_NR 0xffffffffffffffffULL

#define X86_64_SYS_CLONE 56
#define X86_64_SYS_EXIT 60
#define X86_GPA_CHUNK_SIZE (1ULL << 30)
#define X86_MAX_GUEST_CHUNKS 128

#define KERNEL_CS 0x8
#define KERNEL_DS 0x10
#define KERNEL_TSS 0x18

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITE (1ULL << 1)
#define PAGE_LARGE (1ULL << 7)

#define X86_CR0_PE (1ULL << 0)
#define X86_CR0_NE (1ULL << 5)
#define X86_CR0_PG (1ULL << 31)

#define X86_CR4_PSE (1ULL << 4)
#define X86_CR4_PAE (1ULL << 5)
#define X86_CR4_OSFXSR (1ULL << 9)
#define X86_CR4_FSGSBASE (1ULL << 16)
#define X86_CR4_OSXSAVE (1ULL << 18)

#define EFER_SCE (1ULL << 0)
#define EFER_LME (1ULL << 8)
#define EFER_LMA (1ULL << 10)
#define EFER_NX (1ULL << 11)

#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_SYSCALL_MASK 0xc0000084
#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101
#define MSR_KERNEL_GS_BASE 0xc0000102

#define X86_LOWER_CANONICAL_PML4E_COUNT 256

#ifdef __ASSEMBLY__
#define ENTRY(symbol)                                                          \
	.globl symbol;                                                         \
	.type symbol, @function;                                               \
symbol:

#define END(symbol) .size symbol, .-symbol
#endif

#endif /* end of include guard: INTERNAL_H_X86_64_Z1SGY4TR */
