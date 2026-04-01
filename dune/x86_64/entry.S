#include "internal.h"

	.text

.globl syscall_entry_begin
.type syscall_entry_begin, @function
syscall_entry_begin:
	swapgs
	movq %rdi, %gs:DUNE_SYSCALL_ARG0
	movq %rsi, %gs:DUNE_SYSCALL_ARG1
	movq %rdx, %gs:DUNE_SYSCALL_ARG2
	movq %r10, %gs:DUNE_SYSCALL_ARG3
	movq %r8, %gs:DUNE_SYSCALL_ARG4
	movq %r9, %gs:DUNE_SYSCALL_ARG5
	movq %rdi, %gs:DUNE_SYSCALL_ARG6
	movq %rax, %gs:DUNE_SYSCALL_NR
	movq %rcx, %gs:DUNE_SYSCALL_RET_RIP
	movq %r11, %gs:DUNE_SYSCALL_RET_RFLAGS
	leaq syscall_resume(%rip), %rax
	movq %rax, %gs:DUNE_SYSCALL_RESUME_RIP

	movl $DUNE_SYSCALL_PORT, %edx
	inl %dx, %eax

syscall_resume:
	movq %gs:DUNE_SYSCALL_ARG0, %rax
	movq %gs:DUNE_SYSCALL_ARG6, %rdi
	movq %gs:DUNE_SYSCALL_ARG1, %rsi
	movq %gs:DUNE_SYSCALL_ARG2, %rdx
	movq %gs:DUNE_SYSCALL_ARG3, %r10
	movq %gs:DUNE_SYSCALL_ARG4, %r8
	movq %gs:DUNE_SYSCALL_ARG5, %r9
	movq %gs:DUNE_SYSCALL_RET_RIP, %rcx
	movq %gs:DUNE_SYSCALL_RET_RFLAGS, %r11
	swapgs
	pushq %r11
	popfq
	jmp *%rcx
.size syscall_entry_begin, .-syscall_entry_begin

.globl switch_stack
.type switch_stack, @function
switch_stack:
	leaq -8(%rsi), %rsp
	jmp host_loop
.size switch_stack, .-switch_stack

.globl __do_simulate_clone
.type __do_simulate_clone, @function
__do_simulate_clone:
	movq %rcx, %r10
	movl $X86_64_SYS_CLONE, %eax
	syscall
	testq %rax, %rax
	jne clone_parent

	movq 0(%rsp), %r11
	movq 8(%rsp), %rdi
	movq 16(%rsp), %rsi
	call *%r11

	movq %rax, %rdi
	movl $X86_64_SYS_EXIT, %eax
	syscall

clone_parent:
	ret
.size __do_simulate_clone, .-__do_simulate_clone

.globl x86_ud_exception
.type x86_ud_exception, @function
x86_ud_exception:
	pushq $0
	pushq $6
	jmp x86_exception_common
.size x86_ud_exception, .-x86_ud_exception

.globl x86_gp_exception
.type x86_gp_exception, @function
x86_gp_exception:
	pushq $13
	jmp x86_exception_common
.size x86_gp_exception, .-x86_gp_exception

.globl x86_pf_exception
.type x86_pf_exception, @function
x86_pf_exception:
	pushq $14
	jmp x86_exception_common
.size x86_pf_exception, .-x86_pf_exception

x86_exception_common:
	swapgs
	movq (%rsp), %rax
	movq %rax, %gs:DUNE_SYSCALL_ARG0
	movq 8(%rsp), %rax
	movq %rax, %gs:DUNE_SYSCALL_ARG1
	movq 16(%rsp), %rax
	movq %rax, %gs:DUNE_SYSCALL_ARG2
	movq $-1, %rax
	movq %rax, %gs:DUNE_SYSCALL_NR
	movl $DUNE_SYSCALL_PORT, %edx
	inl %dx, %eax
1:
	ud2
	jmp 1b

.globl get_fpu_regs
.type get_fpu_regs, @function
get_fpu_regs:
	fxsave64 (%rdi)
	ret
.size get_fpu_regs, .-get_fpu_regs

.section .note.GNU-stack,"",@progbits
