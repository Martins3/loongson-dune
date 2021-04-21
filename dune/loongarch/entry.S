#include "internal.h"
.global err_entry_begin
.global err_entry_end

err_entry_begin:
INVALID_CODEFLOW_1
err_entry_end:

.global tlb_refill_entry_begin
.global tlb_refill_entry_end

tlb_refill_entry_begin:
csrrd TLBRSAVE, LOONGARCH_CSR_TLBREHI

// TODO 初始化 Hi 的 PS
// 将 Hi 的 low bit 清空
// 
tlbfill
tlb_refill_entry_end:

.global syscall_entry_begin
.global syscall_entry_end

syscall_entry_begin:
// TODO
syscall_entry_end:
