#include "internal.h"
.global err_entry_begin
.global err_entry_end

err_entry_begin:
INVALID_CODEFLOW_1
err_entry_end:

.global tlb_refill_entry_begin
.global tlb_refill_entry_end

tlb_refill_entry_begin:

csrwr t0,  LOONGARCH_CSR_TLBRSAVE
csrwr t1,  LOONGARCH_CSR_KS0

csrrd t0, LOONGARCH_CSR_TLBREHI
// 因为 Loongarch 的 TLB 采用奇偶页, 所以从 EntryHi 的获取掩码为 1G 而不是 512M

// page fault 的 1G 对齐地址
slli.d t0, TLBRSAVE, (TLB_PS + 1)
srli.d t0, TLBRSAVE, (TLB_PS + 1)

// 整个计算过程中， t0 的数值始终保持不变
// 使用 t1 来对应的掩码
csrwr t0, LOONGARCH_CSR_TLBREHI
li t1, TLB_PS
csrxchg t0, t1, LOONGARCH_CSR_TLBREHI

csrwr t0, LOONGARCH_CSR_TLBRELO0
csrrd t1, LOONGARCH_CSR_KS7
csrxchg t0, t1, LOONGARCH_CSR_TLBRELO0

csrwr t0, LOONGARCH_CSR_TLBRELO0
csrrd t1, LOONGARCH_CSR_KS8
csrxchg t0, t1, LOONGARCH_CSR_TLBRELO1
tlbfill

csrrd t0,  LOONGARCH_CSR_TLBRSAVE
csrrd t1,  LOONGARCH_CSR_KS0
ertn
tlb_refill_entry_end:

/* t0 是 caller saved 寄存器 */
/* Syscall number held in a7 */
.global syscall_entry_begin
.global syscall_entry_end
syscall_entry_begin:
csrrd rd, LOONGARCH_CSR_KS7
st.d a0, t0, 0 
st.d a1, t0, 8 
st.d a2, t0, 16
st.d a3, t0, 24
st.d a4, t0, 32
st.d a5, t0, 40
st.d a6, t0, 48
st.d a7, t0, 56
HYPERCALL
ld.d v0, t0, 0
ertn
syscall_entry_end:

/* copied from /sysdeps/unix/sysv/linux/loongarch/clone.S */
// 参考 emulate_fork_by_two_vcpu 中在 stack 上设置的参数
ENTRY (__clone)
	/* Do the system call.  */
	dli	a7,__NR_clone
	syscall	0

	beqz	a0,L (child)

	/* Successful return from the parent.  */
	ret

L (child):
  // TODO loongarch 的 PIC register 在哪?
  ld.d 
  ld.d a0, 8(sp)
END (dune_clone)

//  
.global host_loop
.global switch_stack
switch_stack:
	move sp, a1
  // TODO t9 ?
	la.abs t9, host_loop
  // TODO bal ?
	bal host_loop 





ENTRY (__clone)
	/* Sanity check arguments.  */
	beqz	a0, L (invalid)	/* No NULL function pointers.  */
	beqz	a1, L (invalid)	/* No NULL stack pointers.  */

	addi.d	a1, a1, -16	/* Reserve argument save space.  */
	st.d	a0, a1, 0	/* Save function pointer.  */
	st.d	a3, a1, REG_SZ	/* Save argument pointer.  */

	/* The syscall expects the args to be in different slots.  */
	or	a0, a2, zero
	or	a2, a4, zero
	or	a3, a5, zero
	or	a4, a6, zero

	/* Do the system call.  */
	dli	a7,__NR_clone
	syscall	0

	blt	a0, zero ,L (error)
	beqz	a0,L (thread_start)

	/* Successful return from the parent.  */
	ret

L (invalid):
	dli	a0, -EINVAL
	/* Something bad happened -- no child created.  */
L (error):
	b	__syscall_error
	END (dune_clone)
