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
  /* t0 是 __SYSCALL_CLOBBERS 寄存器，可以随意修改 */
  ld.d t0, sp, 0 /* child_entry */
  ld.d a0, sp, 8 /* vcpu */
  jirl zero, t0, 0 /* child_entry 接下来调用 host_loop，无需返回 */
END (dune_clone)

.global host_loop
.global switch_stack
switch_stack:
	move sp, a1
	b host_loop /* host_loop 绝对不会返回 */
