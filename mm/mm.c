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
      // pr_debug("[%d]", i);
    }
  }
  return d;
}

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER)	__compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)
#endif

#define BUILD_ASSERT(cond) do { (void) sizeof(char [1 - 2*!(cond)]); } while(0)


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
    u64 config;
    u64 config1;
    u64 config2;
    u64 config3;
    u64 config4;
    u64 config5;
    u64 config6;
    u64 config7;
	} p;
  memset(&p, 0 , sizeof(struct cp0));

  BUILD_ASSERT(56 == offsetof(struct cp0, config));
  BUILD_ASSERT(112 == offsetof(struct cp0, config7));

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
	    "sd $t0, 48(%0)\n\t"
	    "mfc0	$t0, $16, 0\n\t"
	    "sw $t0, 56(%0)\n\t"
	    "mfc0	$t0, $16, 1\n\t"
	    "sw $t0, 64(%0)\n\t"
	    "mfc0	$t0, $16, 2\n\t"
	    "sw $t0, 72(%0)\n\t"
	    "mfc0	$t0, $16, 3\n\t"
	    "sw $t0, 80(%0)\n\t"
	    "mfc0	$t0, $16, 4\n\t"
	    "sw $t0, 88(%0)\n\t"
	    "mfc0	$t0, $16, 5\n\t"
	    "sw $t0, 96(%0)\n\t"
	    "mfc0	$t0, $16, 6\n\t"
	    "sw $t0, 104(%0)\n\t"
	    "mfc0	$t0, $16, 7\n\t"
	    "sw $t0, 112(%0)\n\t"
	    :
	    : "r"(&p)
	    : "memory");
  pr_debug("value : 0x%llx\n", bits(p.config));
  pr_debug("value : 0x%llx\n", bits(p.config1));
  pr_debug("value : 0x%llx\n", bits(p.config2));
  pr_debug("value : 0x%llx\n", bits(p.config3));
  pr_debug("value : 0x%llx\n", bits(p.config4));
  pr_debug("value : 0x%llx\n", bits(p.config5));
  pr_debug("value : 0x%llx\n", bits(p.config6));
  pr_debug("value : 0x%llx\n", bits(p.config7));
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
