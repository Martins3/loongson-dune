#define _GNU_SOURCE
#include <asm/prctl.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "arch.h"
#include "internal.h"
#include "../interface.h"

extern void syscall_entry_begin(void);
extern void get_fpu_regs(struct kvm_fpu *);
extern void x86_ud_exception(void);
extern void x86_gp_exception(void);
extern void x86_pf_exception(void);

struct x86_guest_chunk_mapping {
	u64 gva_base;
	u64 gpa_base;
};

struct x86_shared_state {
	u64 *pml4_hva;
	u64 pml4_gpa;
	u64 *pdpt_hva[512];
	u64 pdpt_gpa[512];
	struct x86_guest_chunk_mapping chunks[X86_MAX_GUEST_CHUNKS];
	int nr_chunks;
};

struct desc64 {
	u16 limit0;
	u16 base0;
	u8 base1;
	u8 type : 4;
	u8 s : 1;
	u8 dpl : 2;
	u8 p : 1;
	u8 limit1 : 4;
	u8 avl : 1;
	u8 l : 1;
	u8 db : 1;
	u8 g : 1;
	u8 base2;
	u32 base3;
	u32 zero;
} __attribute__((packed));

struct idt_entry {
	u16 offset0;
	u16 selector;
	u16 ist : 3;
	u16 zero0 : 5;
	u16 type : 4;
	u16 zero1 : 1;
	u16 dpl : 2;
	u16 p : 1;
	u16 offset1;
	u32 offset2;
	u32 zero2;
} __attribute__((packed));

struct kvm_cpuid2_buffer {
	struct kvm_cpuid2 header;
	struct kvm_cpuid_entry2 entries[128];
};

struct kvm_msrs_6 {
	struct kvm_msrs header;
	struct kvm_msr_entry entries[6];
};

struct kvm_msrs_2 {
	struct kvm_msrs header;
	struct kvm_msr_entry entries[2];
};

struct kvm_msrs_1 {
	struct kvm_msrs header;
	struct kvm_msr_entry entries[1];
};

struct dune_procmap_entry {
	unsigned long begin;
	unsigned long end;
	long int offset;
	bool r;
	bool w;
	bool x;
	bool p;
	char *path;
	int type;
};

static inline long host_syscall6(u64 nr, u64 arg0, u64 arg1, u64 arg2, u64 arg3,
				 u64 arg4, u64 arg5)
{
	register long rax asm("rax") = nr;
	register long rdi asm("rdi") = arg0;
	register long rsi asm("rsi") = arg1;
	register long rdx asm("rdx") = arg2;
	register long r10 asm("r10") = arg3;
	register long r8 asm("r8") = arg4;
	register long r9 asm("r9") = arg5;

	asm volatile("syscall"
		     : "+a"(rax)
		     : "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8),
		       "r"(r9)
		     : "rcx", "r11", "memory");
	return rax;
}

static inline u64 align_up_u64(u64 value, u64 align)
{
	return (value + align - 1) & ~(align - 1);
}

static void x86_lock_vm(struct kvm_vm *vm)
{
	if (pthread_spin_lock(&vm->lock))
		die("locked failed");
}

static void x86_unlock_vm(struct kvm_vm *vm)
{
	if (pthread_spin_unlock(&vm->lock))
		die("unlocked failed");
}

static u64 x86_alloc_gpa_locked(struct kvm_vm *vm, u64 len, u64 align)
{
	u64 gpa = align_up_u64(vm->next_gpa, align);

	vm->next_gpa = gpa + len;
	return gpa;
}

static void x86_register_memslot_locked(struct kvm_vm *vm, u64 guest_phys_addr,
					u64 userspace_addr, u64 len)
{
	struct kvm_userspace_memory_region mem;

	if (vm->next_mem_slot >= vm->max_mem_slots)
		die("No more memslot to allocate\n");

	mem = (struct kvm_userspace_memory_region){
		.slot = vm->next_mem_slot++,
		.flags = 0,
		.guest_phys_addr = guest_phys_addr,
		.memory_size = len,
		.userspace_addr = userspace_addr,
	};

	if (ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem) < 0)
		die("KVM_SET_USER_MEMORY_REGION");
}

static u64 x86_map_hva_locked(struct kvm_vm *vm, u64 userspace_addr, u64 len,
			      u64 align)
{
	u64 guest_phys_addr = x86_alloc_gpa_locked(vm, len, align);

	x86_register_memslot_locked(vm, guest_phys_addr, userspace_addr, len);
	return guest_phys_addr;
}

static struct x86_shared_state *x86_shared(const struct kvm_cpu *cpu)
{
	return cpu->info.shared;
}

static struct x86_guest_chunk_mapping *x86_find_chunk(struct x86_shared_state *shared,
						       u64 chunk_base)
{
	for (int i = 0; i < shared->nr_chunks; ++i) {
		if (shared->chunks[i].gva_base == chunk_base)
			return &shared->chunks[i];
	}

	return NULL;
}

static void x86_map_process_range(struct kvm_cpu *cpu, u64 start, u64 len);

static inline void kvm_seg_set_unusable(struct kvm_segment *segp)
{
	memset(segp, 0, sizeof(*segp));
	segp->unusable = true;
}

static void set_idt_entry(void *idt, int vector, unsigned long addr)
{
	struct idt_entry *base = (struct idt_entry *)idt;
	struct idt_entry *e = &base[vector];

	memset(e, 0, sizeof(*e));
	e->offset0 = addr;
	e->selector = KERNEL_CS;
	e->type = 14;
	e->dpl = 0;
	e->p = 1;
	e->offset1 = addr >> 16;
	e->offset2 = addr >> 32;
}

static void kvm_seg_fill_gdt_64bit(const struct kvm_cpu *cpu,
				   const struct kvm_segment *segp)
{
	struct desc64 *desc =
		(struct desc64 *)((u8 *)cpu->info.gdt + (segp->selector >> 3) * 8);

	memset(desc, 0, sizeof(*desc));
	desc->limit0 = segp->limit & 0xffff;
	desc->base0 = segp->base & 0xffff;
	desc->base1 = segp->base >> 16;
	desc->type = segp->type;
	desc->s = segp->s;
	desc->dpl = segp->dpl;
	desc->p = segp->present;
	desc->limit1 = segp->limit >> 16;
	desc->avl = segp->avl;
	desc->l = segp->l;
	desc->db = segp->db;
	desc->g = segp->g;
	desc->base2 = segp->base >> 24;
	if (!segp->s)
		desc->base3 = segp->base >> 32;
}

static void kvm_seg_set_kernel_code_64bit(struct kvm_segment *segp)
{
	memset(segp, 0, sizeof(*segp));
	segp->selector = KERNEL_CS;
	segp->limit = 0xffffffffu;
	segp->s = 1;
	segp->type = 0x0b;
	segp->g = 1;
	segp->l = 1;
	segp->present = 1;
}

static void kvm_seg_set_kernel_data_64bit(struct kvm_segment *segp)
{
	memset(segp, 0, sizeof(*segp));
	segp->selector = KERNEL_DS;
	segp->limit = 0xffffffffu;
	segp->s = 1;
	segp->type = 0x03;
	segp->g = 1;
	segp->present = 1;
}

static void kvm_seg_set_tss_64bit(const struct kvm_cpu *cpu,
				  struct kvm_segment *segp)
{
	memset(segp, 0, sizeof(*segp));
	segp->base = (u64)cpu->info.tss;
	segp->limit = 0x67;
	segp->selector = KERNEL_TSS;
	segp->type = 0x0b;
	segp->present = 1;
}

static void setup_descriptor_tables(struct kvm_cpu *cpu)
{
	struct kvm_segment seg;

	if (!cpu->info.gdt)
		cpu->info.gdt = mmap_one_page();
	if (!cpu->info.idt)
		cpu->info.idt = mmap_one_page();
	if (!cpu->info.tss)
		cpu->info.tss = mmap_one_page();

	x86_map_process_range(cpu, (u64)cpu->info.gdt, PAGESIZE);
	x86_map_process_range(cpu, (u64)cpu->info.idt, PAGESIZE);
	x86_map_process_range(cpu, (u64)cpu->info.tss, PAGESIZE);

	memset(cpu->info.gdt, 0, PAGESIZE);
	memset(cpu->info.idt, 0, PAGESIZE);
	memset(cpu->info.tss, 0, PAGESIZE);

	set_idt_entry(cpu->info.idt, 6, (unsigned long)x86_ud_exception);
	set_idt_entry(cpu->info.idt, 13, (unsigned long)x86_gp_exception);
	set_idt_entry(cpu->info.idt, 14, (unsigned long)x86_pf_exception);

	kvm_seg_set_kernel_code_64bit(&seg);
	kvm_seg_fill_gdt_64bit(cpu, &seg);

	kvm_seg_set_kernel_data_64bit(&seg);
	kvm_seg_fill_gdt_64bit(cpu, &seg);

	kvm_seg_set_tss_64bit(cpu, &seg);
	kvm_seg_fill_gdt_64bit(cpu, &seg);
}

static struct x86_shared_state *x86_create_shared_state(struct kvm_cpu *cpu)
{
	struct x86_shared_state *shared = calloc(1, sizeof(*shared));

	if (!shared)
		die("calloc x86 shared state");

	shared->pml4_hva = mmap_one_page();
	memset(shared->pml4_hva, 0, PAGESIZE);

	x86_lock_vm(cpu->vm);
	shared->pml4_gpa =
		x86_map_hva_locked(cpu->vm, (u64)shared->pml4_hva, PAGESIZE,
				   PAGESIZE);
	x86_unlock_vm(cpu->vm);

	return shared;
}

static void x86_ensure_chunk_mapping(struct kvm_cpu *cpu, u64 gva)
{
	struct x86_shared_state *shared = x86_shared(cpu);
	u64 chunk_base = gva & ~(X86_GPA_CHUNK_SIZE - 1);
	u64 pml4_index = (chunk_base >> 39) & 0x1ff;
	u64 pdpt_index = (chunk_base >> 30) & 0x1ff;
	u64 *pdpt_hva;
	u64 chunk_gpa;

	if (chunk_base >= KVM_GUEST_MEMORY_SIZE)
		return;

	x86_lock_vm(cpu->vm);

	if (x86_find_chunk(shared, chunk_base)) {
		x86_unlock_vm(cpu->vm);
		return;
	}

	if (!shared->pdpt_hva[pml4_index]) {
		shared->pdpt_hva[pml4_index] = mmap_one_page();
		memset(shared->pdpt_hva[pml4_index], 0, PAGESIZE);
		shared->pdpt_gpa[pml4_index] =
			x86_map_hva_locked(cpu->vm, (u64)shared->pdpt_hva[pml4_index],
					   PAGESIZE, PAGESIZE);
		shared->pml4_hva[pml4_index] =
			shared->pdpt_gpa[pml4_index] | PAGE_PRESENT | PAGE_WRITE;
	}

	chunk_gpa =
		x86_map_hva_locked(cpu->vm, chunk_base, X86_GPA_CHUNK_SIZE,
				   X86_GPA_CHUNK_SIZE);
	pdpt_hva = shared->pdpt_hva[pml4_index];
	pdpt_hva[pdpt_index] = chunk_gpa | PAGE_PRESENT | PAGE_WRITE | PAGE_LARGE;

	if (shared->nr_chunks >= X86_MAX_GUEST_CHUNKS)
		die("Too many guest chunks");

	shared->chunks[shared->nr_chunks++] = (struct x86_guest_chunk_mapping){
		.gva_base = chunk_base,
		.gpa_base = chunk_gpa,
	};

	x86_unlock_vm(cpu->vm);
}

static void x86_map_process_range(struct kvm_cpu *cpu, u64 start, u64 len)
{
	u64 end;

	if (len == 0)
		return;
	if (start >= KVM_GUEST_MEMORY_SIZE)
		return;

	end = start + len;
	if (end > KVM_GUEST_MEMORY_SIZE)
		end = KVM_GUEST_MEMORY_SIZE;

	for (u64 addr = start & ~(X86_GPA_CHUNK_SIZE - 1); addr < end;
	     addr += X86_GPA_CHUNK_SIZE) {
		x86_ensure_chunk_mapping(cpu, addr);
	}
}

static void x86_map_current_process(struct kvm_cpu *cpu)
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
		if (fgets(line, sizeof(line), map) == NULL)
			break;
		sscanf((char *)&line, "%lx-%lx %c%c%c%c %lx %x:%x %d %s",
		       &e.begin, &e.end, &read, &write, &execute, &private,
		       &e.offset, &dev1, &dev2, &inode, path);
		if (e.end <= e.begin)
			continue;
		x86_map_process_range(cpu, e.begin, e.end - e.begin);
	}

	if (fclose(map))
		die("close file");
}

static void init_cpuid(struct kvm_cpu *cpu)
{
	struct kvm_cpuid2_buffer cpuid;

	memset(&cpuid, 0, sizeof(cpuid));
	cpuid.header.nent = sizeof(cpuid.entries) / sizeof(cpuid.entries[0]);

	if (ioctl(cpu->vm->sys_fd, KVM_GET_SUPPORTED_CPUID, &cpuid.header) < 0)
		die("KVM_GET_SUPPORTED_CPUID");
	if (ioctl(cpu->vcpu_fd, KVM_SET_CPUID2, &cpuid.header) < 0)
		die("KVM_SET_CPUID2");
}

static void init_fpu(struct kvm_cpu *cpu)
{
	struct kvm_fpu fpu_regs;

	memset(&fpu_regs, 0, sizeof(fpu_regs));
	get_fpu_regs(&fpu_regs);
	if (ioctl(cpu->vcpu_fd, KVM_SET_FPU, &fpu_regs) < 0)
		die("KVM_SET_FPU");
}

static u64 get_guest_msr(struct kvm_cpu *cpu, u32 index)
{
	struct kvm_msrs_1 msrs;

	memset(&msrs, 0, sizeof(msrs));
	msrs.header.nmsrs = 1;
	msrs.entries[0].index = index;

	if (ioctl(cpu->vcpu_fd, KVM_GET_MSRS, &msrs.header) != 1)
		die("KVM_GET_MSRS");
	return msrs.entries[0].data;
}

static void set_guest_msr(struct kvm_cpu *cpu, u32 index, u64 value)
{
	struct kvm_msrs_1 msrs;

	memset(&msrs, 0, sizeof(msrs));
	msrs.header.nmsrs = 1;
	msrs.entries[0].index = index;
	msrs.entries[0].data = value;

	if (ioctl(cpu->vcpu_fd, KVM_SET_MSRS, &msrs.header) != 1)
		die("KVM_SET_MSRS");
}

static void dup_fpu(struct kvm_cpu *child_cpu, const struct kvm_fpu *parent_fpu)
{
	if (ioctl(child_cpu->vcpu_fd, KVM_SET_FPU, parent_fpu) < 0)
		die("KVM_SET_FPU");
}

static u64 host_get_thread_base(int request)
{
	unsigned long base = 0;

	if (syscall(SYS_arch_prctl, request, &base) != 0)
		die("arch_prctl");
	return base;
}

static u64 host_get_xcr0(void)
{
	u32 eax;
	u32 edx;

	asm volatile(".byte 0x0f, 0x01, 0xd0"
		     : "=a"(eax), "=d"(edx)
		     : "c"(0));
	return ((u64)edx << 32) | eax;
}

static void set_guest_xcrs(struct kvm_cpu *cpu, u64 xcr0)
{
	struct kvm_xcrs xcrs;

	memset(&xcrs, 0, sizeof(xcrs));
	xcrs.nr_xcrs = 1;
	xcrs.xcrs[0].xcr = 0;
	xcrs.xcrs[0].value = xcr0;

	if (ioctl(cpu->vcpu_fd, KVM_SET_XCRS, &xcrs) < 0)
		die("KVM_SET_XCRS");
}

static void set_vcpu_runnable(struct kvm_cpu *cpu)
{
	struct kvm_mp_state mp_state;

	if (cpu->cpu_id == 0)
		return;

	memset(&mp_state, 0, sizeof(mp_state));
	mp_state.mp_state = KVM_MP_STATE_RUNNABLE;

	if (ioctl(cpu->vcpu_fd, KVM_SET_MP_STATE, &mp_state) < 0)
		die("KVM_SET_MP_STATE");
}

static void set_guest_sregs(struct kvm_cpu *cpu, u64 fs_base, u64 gs_base)
{
	struct x86_shared_state *shared = x86_shared(cpu);
	struct kvm_sregs sregs;
	struct kvm_segment code;
	struct kvm_segment data;
	struct kvm_segment tss;

	if (ioctl(cpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0)
		die("KVM_GET_SREGS");

	kvm_seg_set_kernel_code_64bit(&code);
	kvm_seg_set_kernel_data_64bit(&data);
	kvm_seg_set_tss_64bit(cpu, &tss);
	kvm_seg_set_unusable(&sregs.ldt);

	sregs.cr0 = X86_CR0_PE | X86_CR0_NE | X86_CR0_PG;
	sregs.cr4 = X86_CR4_PSE | X86_CR4_PAE | X86_CR4_OSFXSR |
		    X86_CR4_FSGSBASE | X86_CR4_OSXSAVE;
	sregs.efer = EFER_SCE | EFER_LME | EFER_LMA | EFER_NX;

	sregs.gdt.base = (u64)cpu->info.gdt;
	sregs.gdt.limit = PAGESIZE - 1;
	sregs.idt.base = (u64)cpu->info.idt;
	sregs.idt.limit = PAGESIZE - 1;
	sregs.cr3 = shared->pml4_gpa;

	sregs.cs = code;
	sregs.ss = data;
	sregs.ds = data;
	sregs.es = data;
	sregs.fs = data;
	sregs.gs = data;
	sregs.tr = tss;
	sregs.fs.base = fs_base;
	sregs.gs.base = gs_base;

	if (ioctl(cpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0)
		die("KVM_SET_SREGS");
}

static void set_guest_msrs(struct kvm_cpu *cpu, u64 fs_base, u64 gs_base,
			   u64 kernel_gs_base)
{
	struct kvm_msrs_6 msrs;
	u64 star = (u64)KERNEL_CS << 32;

	memset(&msrs, 0, sizeof(msrs));
	msrs.header.nmsrs = 6;

	msrs.entries[0].index = MSR_STAR;
	msrs.entries[0].data = star;
	msrs.entries[1].index = MSR_LSTAR;
	msrs.entries[1].data = (u64)syscall_entry_begin;
	msrs.entries[2].index = MSR_SYSCALL_MASK;
	msrs.entries[2].data = 0;
	msrs.entries[3].index = MSR_FS_BASE;
	msrs.entries[3].data = fs_base;
	msrs.entries[4].index = MSR_GS_BASE;
	msrs.entries[4].data = gs_base;
	msrs.entries[5].index = MSR_KERNEL_GS_BASE;
	msrs.entries[5].data = kernel_gs_base;

	if (ioctl(cpu->vcpu_fd, KVM_SET_MSRS, &msrs.header) !=
	    (int)msrs.header.nmsrs)
		die("KVM_SET_MSRS");
}

static void set_guest_base_msrs(struct kvm_cpu *cpu, u64 fs_base, u64 gs_base)
{
	struct kvm_msrs_2 msrs;

	memset(&msrs, 0, sizeof(msrs));
	msrs.header.nmsrs = 2;
	msrs.entries[0].index = MSR_FS_BASE;
	msrs.entries[0].data = fs_base;
	msrs.entries[1].index = MSR_GS_BASE;
	msrs.entries[1].data = gs_base;

	if (ioctl(cpu->vcpu_fd, KVM_SET_MSRS, &msrs.header) !=
	    (int)msrs.header.nmsrs)
		die("KVM_SET_MSRS");
}

static void prepare_guest_runtime(struct kvm_cpu *cpu, u64 fs_base,
				  u64 user_gs_base, bool resume_from_syscall)
{
	u64 current_gs_base = user_gs_base;
	u64 kernel_gs_base = (u64)&cpu->syscall_parameter[0];

	if (!cpu->info.shared) {
		cpu->info.shared = x86_create_shared_state(cpu);
		x86_map_current_process(cpu);
	}

	cpu->info.fs_base = fs_base;
	cpu->info.gs_base = user_gs_base;
	if (resume_from_syscall) {
		current_gs_base = (u64)&cpu->syscall_parameter[0];
		kernel_gs_base = user_gs_base;
	}

	setup_descriptor_tables(cpu);
	set_guest_sregs(cpu, fs_base, current_gs_base);
	set_guest_msrs(cpu, fs_base, current_gs_base, kernel_gs_base);
	set_guest_xcrs(cpu, host_get_xcr0());
}

static int __attribute__((noinline)) kvm_launch(struct kvm_cpu *cpu)
{
	struct kvm_regs regs;
	u64 fs_base = host_get_thread_base(ARCH_GET_FS);
	u64 gs_base = host_get_thread_base(ARCH_GET_GS);

	memset(&regs, 0, sizeof(regs));
	regs.rip = (u64)&&guest_entry;

	asm volatile("mov %%rbx, %0\n\t"
		     "mov %%rbp, %1\n\t"
		     "mov %%r12, %2\n\t"
		     "mov %%r13, %3\n\t"
		     "mov %%r14, %4\n\t"
		     "mov %%r15, %5\n\t"
		     "mov %%rsp, %6\n\t"
		     "pushfq\n\t"
		     "popq %7\n\t"
		     : "=m"(regs.rbx), "=m"(regs.rbp), "=m"(regs.r12),
		       "=m"(regs.r13), "=m"(regs.r14), "=m"(regs.r15),
		       "=m"(regs.rsp), "=m"(regs.rflags)
		     :
		     : "memory");

	regs.rflags |= 0x2;

	init_cpuid(cpu);
	prepare_guest_runtime(cpu, fs_base, gs_base, false);
	init_fpu(cpu);

	if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, &regs) < 0)
		die("KVM_SET_REGS");

	vacate_current_stack(cpu);
	die("host never reach here\n");

guest_entry:
	return 0;
}

void arch_dune_enter(struct kvm_cpu *cpu)
{
	kvm_launch(cpu);
}

void arch_share_vm_state(struct kvm_cpu *child_cpu,
			 const struct kvm_cpu *parent_cpu)
{
	child_cpu->info.shared = parent_cpu->info.shared;
	child_cpu->info.fs_base = parent_cpu->info.fs_base;
	child_cpu->info.gs_base = parent_cpu->info.gs_base;
}

bool arch_is_syscall_exit(const struct kvm_cpu *cpu)
{
	return cpu->kvm_run->exit_reason == KVM_EXIT_IO &&
	       cpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
	       cpu->kvm_run->io.port == DUNE_SYSCALL_PORT &&
	       cpu->kvm_run->io.size == 4 && cpu->kvm_run->io.count == 1;
}

u64 arch_get_sysno(const struct kvm_cpu *cpu)
{
	return cpu->syscall_parameter[7];
}

static struct clone3_args get_clone3_args(const struct kvm_cpu *cpu,
					  size_t *size_out)
{
	struct clone3_args args;
	size_t size = cpu->syscall_parameter[1];

	memset(&args, 0, sizeof(args));
	if (size > sizeof(args))
		size = sizeof(args);
	if (size != 0)
		memcpy(&args, (const void *)cpu->syscall_parameter[0], size);
	if (size_out)
		*size_out = size;
	return args;
}

enum CLONE_TYPE arch_get_clone_type(const struct kvm_cpu *parent_cpu, int sysno)
{
	u64 flags;
	u64 child_stack = 0;

	if (sysno == SYS_FORK)
		return DIFF_VM_OLD_STACk;

	if (sysno == SYS_CLONE) {
		flags = parent_cpu->syscall_parameter[0];
		child_stack = parent_cpu->syscall_parameter[1];
	} else if (sysno == SYS_CLONE3) {
		struct clone3_args args = get_clone3_args(parent_cpu, NULL);
		flags = args.flags;
		if (args.stack)
			child_stack = args.stack + args.stack_size;
	} else {
		die("unexpected sysno");
	}

	if (flags & CLONE_VM)
		return SAME_VM;
	if (child_stack)
		return DIFF_VM_NEW_STACK;
	return DIFF_VM_OLD_STACk;
}

bool arch_do_syscall(struct kvm_cpu *cpu, bool is_fork)
{
	u64 sysno = arch_get_sysno(cpu);
	long ret;

	if (sysno == SYS_CLONE3 && is_fork) {
		size_t args_size;
		struct clone3_args args = get_clone3_args(cpu, &args_size);

		args.flags &= ~CLONE_SETTLS;
		ret = host_syscall6(sysno, (u64)&args, args_size, 0,
				    0, 0, 0);
	} else {
		ret = host_syscall6(sysno, cpu->syscall_parameter[0],
				    cpu->syscall_parameter[1],
				    cpu->syscall_parameter[2],
				    cpu->syscall_parameter[3],
				    cpu->syscall_parameter[4],
				    cpu->syscall_parameter[5]);
	}

	cpu->syscall_parameter[0] = ret;
	return is_fork && ret == 0;
}

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	struct kvm_sregs sregs;

	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &parent_cpu->info.regs) < 0)
		die("KVM_GET_REGS");
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_FPU, &parent_cpu->info.fpu) < 0)
		die("KVM_GET_FPU");
	if (ioctl(parent_cpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0)
		die("KVM_GET_SREGS");

	parent_cpu->info.fs_base = sregs.fs.base;
	parent_cpu->info.gs_base = get_guest_msr(parent_cpu, MSR_KERNEL_GS_BASE);
}

void init_child_thread_info(struct kvm_cpu *child_cpu,
			    const struct kvm_cpu *parent_cpu, int sysno)
{
	struct kvm_regs child_regs;
	u64 child_fs_base = parent_cpu->info.fs_base;

	if (child_cpu->vm == parent_cpu->vm)
		arch_share_vm_state(child_cpu, parent_cpu);
	memcpy(&child_regs, &parent_cpu->info.regs, sizeof(child_regs));
	memcpy(child_cpu->syscall_parameter, parent_cpu->syscall_parameter,
	       sizeof(child_cpu->syscall_parameter));

	child_cpu->syscall_parameter[0] = 0;
	child_cpu->syscall_return_rip = parent_cpu->syscall_return_rip;
	child_cpu->syscall_return_rflags = parent_cpu->syscall_return_rflags;
	child_cpu->syscall_resume_rip = parent_cpu->syscall_resume_rip;

	child_regs.rax = 0;
	child_regs.rip = parent_cpu->syscall_resume_rip;

	if (sysno == SYS_CLONE) {
		if (parent_cpu->syscall_parameter[0] & CLONE_SETTLS)
			child_fs_base = parent_cpu->syscall_parameter[4];
		if (parent_cpu->syscall_parameter[1] != 0)
			child_regs.rsp = parent_cpu->syscall_parameter[1];
	} else if (sysno == SYS_CLONE3) {
		struct clone3_args args = get_clone3_args(parent_cpu, NULL);

		if (args.flags & CLONE_SETTLS)
			child_fs_base = args.tls;
		if (args.stack != 0)
			child_regs.rsp = args.stack + args.stack_size;
	}

	init_cpuid(child_cpu);
	set_vcpu_runnable(child_cpu);
	prepare_guest_runtime(child_cpu, child_fs_base, parent_cpu->info.gs_base,
			      true);
	if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &child_regs) < 0)
		die("KVM_SET_REGS");
	dup_fpu(child_cpu, &parent_cpu->info.fpu);
}

void arch_set_thread_area(struct kvm_cpu *vcpu)
{
	(void)vcpu;
}

bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno)
{
	struct kvm_sregs sregs;
	u64 op;
	u64 arg;
	long ret;

	if (sysno == SYS_MMAP) {
		ret = host_syscall6(sysno, vcpu->syscall_parameter[0],
				    vcpu->syscall_parameter[1],
				    vcpu->syscall_parameter[2],
				    vcpu->syscall_parameter[3],
				    vcpu->syscall_parameter[4],
				    vcpu->syscall_parameter[5]);
		vcpu->syscall_parameter[0] = ret;
		if (ret >= 0)
			x86_map_process_range(vcpu, ret, vcpu->syscall_parameter[1]);
		return true;
	}

	if (sysno != SYS_ARCH_PRCTL)
		return false;

	op = vcpu->syscall_parameter[0];
	arg = vcpu->syscall_parameter[1];

	if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0)
		die("KVM_GET_SREGS");

	switch (op) {
	case ARCH_SET_FS:
		sregs.fs.base = arg;
		if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0)
			die("KVM_SET_SREGS");
		vcpu->info.fs_base = arg;
		set_guest_base_msrs(vcpu, vcpu->info.fs_base, vcpu->info.gs_base);
		vcpu->syscall_parameter[0] = 0;
		return true;
	case ARCH_GET_FS:
		*(u64 *)arg = sregs.fs.base;
		vcpu->syscall_parameter[0] = 0;
		return true;
	case ARCH_SET_GS:
		vcpu->info.gs_base = arg;
		set_guest_msr(vcpu, MSR_KERNEL_GS_BASE, arg);
		vcpu->syscall_parameter[0] = 0;
		return true;
	case ARCH_GET_GS:
		*(u64 *)arg = get_guest_msr(vcpu, MSR_KERNEL_GS_BASE);
		vcpu->syscall_parameter[0] = 0;
		return true;
	default:
		vcpu->syscall_parameter[0] = -EINVAL;
		return true;
	}
}

u64 __do_simulate_clone(u64 flags, u64 child_host_stack, u64 parent_tid,
			u64 child_tid, u64 tls);

void do_simulate_clone(struct kvm_cpu *parent_cpu, u64 child_host_stack)
{
	u64 sysno = arch_get_sysno(parent_cpu);
	long child_pid;

	if (sysno == SYS_CLONE) {
		u64 flags = parent_cpu->syscall_parameter[0] & ~CLONE_SETTLS;

		child_pid = __do_simulate_clone(flags, child_host_stack,
						parent_cpu->syscall_parameter[2],
						parent_cpu->syscall_parameter[3], 0);
	} else if (sysno == SYS_CLONE3) {
		struct clone3_args args = get_clone3_args(parent_cpu, NULL);
		u64 flags;

		if ((args.flags & CLONE_PIDFD) || args.set_tid ||
		    args.set_tid_size || args.cgroup) {
			die("unsupported clone3 options pidfd=%#llx set_tid=%#llx "
			    "set_tid_size=%#llx cgroup=%#llx flags=%#llx "
			    "parent_tid=%#llx child_tid=%#llx tls=%#llx",
			    args.pidfd, args.set_tid, args.set_tid_size,
			    args.cgroup, args.flags, args.parent_tid,
			    args.child_tid, args.tls);
		}

		flags = args.flags & ~CLONE_SETTLS;
		child_pid = __do_simulate_clone(flags, child_host_stack,
						args.parent_tid, args.child_tid, 0);
	} else {
		die("unexpected sysno");
	}

	parent_cpu->syscall_parameter[0] = child_pid;
}

void escape()
{
	die("unimp");
}
