/*
 * Module to create a proc entry. The user can read and write to the proc entry.
 */

// https://lkw.readthedocs.io/en/latest/doc/05_proc_interface.html
#include <linux/module.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

#define CP0_PAGEGRAIN $5, 1

u64 bits(const u64 d){
  int i;
  for (i = 0; i < sizeof(u64) * 8; ++i) {
    if(((u64)1 << i) & d){
      pr_debug("[%d]", i);
    }
  }
  return d;
}

int proc_init(void)
{
	struct cp0 {
		u64 pagegrain;
	} p = { .pagegrain = 0 };

	asm(".set	push\n\t"
	    ".set	mips64\n\t"
	    "mfc0	$t0, $5, 1\n\t"
	    "sw $t0, 0(%0)"
	    :
	    : "r"(&p)
	    : "memory");

  pr_debug("pagegrain : %llx\n", bits(p.pagegrain));

	return 0;
}

/* Function to remove the proc entry.  Call this when the module unloads. */
void proc_cleanup(void)
{
	pr_debug("Bye\n");
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
