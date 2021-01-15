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
		u64 hwrena;
		u64 status;
    u64 intctl;
    u64 cause;
    u64 prid;
    u64 ebase;
	} p;
  memset(&p, 0 , sizeof(struct cp0));

  if(sizeof(struct cp0) != 8 * 7){
    pr_err("memory alignment error");
    return 1;
  }

	asm(".set	push\n\t"
	    ".set	mips64\n\t"
	    "mfc0	$t0, $5, 1\n\t"
	    "sw $t0, 0(%0)\n\t"
	    "mfc0	$t0, $7, 0\n\t"
	    "sw $t0, 8(%0)\n\t"
	    "mfc0	$t0, $12, 0\n\t"
	    "sw $t0, 16(%0)\n\t"
	    "mfc0	$t0, $12, 1\n\t"
	    "sw $t0, 24(%0)\n\t"
	    "mfc0	$t0, $13, 0\n\t"
	    "sw $t0, 32(%0)\n\t"
	    "mfc0	$t0, $15, 0\n\t"
	    "sw $t0, 40(%0)\n\t"
	    "dmfc0	$t0, $15, 1\n\t"
	    "sd $t0, 48(%0)"
	    :
	    : "r"(&p)
	    : "memory");
  pr_debug("value : 0x%llx\n", bits(p.ebase));
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
