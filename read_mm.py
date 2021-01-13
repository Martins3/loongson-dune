#!/usr/bin/python3

# copied from https://github.com/iovisor/bpftrace/issues/901

from bcc import BPF

bpf_text = """
#include <linux/sched.h>

int trace_sys_open(struct pt_regs *ctx, long code) {
    struct task_struct *t = (struct task_struct *)bpf_get_current_task();
    struct mm_struct *mm = t->mm;

    bpf_trace_printk("Some random fields: %lx %lx\\n",
		     mm->mmap_base, mm->start_stack);

    return 0;
}
"""

b = BPF(text=bpf_text)
b.attach_kprobe(event="do_sys_open", fn_name="trace_sys_open")

while 1:
    try:
        (task, pid, cpu, flags, ts, msg) = b.trace_fields()
    except ValueError:
        continue
    print("%-12s %-6d %s" % (task, pid, msg))
