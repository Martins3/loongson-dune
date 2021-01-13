/*
 * Module to create a proc entry. The user can read and write to the proc entry.
 */

// https://lkw.readthedocs.io/en/latest/doc/05_proc_interface.html
#include <linux/module.h>
#include <linux/sched/signal.h>
#include <linux//uaccess.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

int proc_init(void)
{
	struct task_struct *g;
	rcu_read_lock();
	for_each_process (g) { 
    if(g->mm)
      pr_debug("%s ---> %lx %lx\n", g->comm, g->mm->mmap_base, g->mm->start_stack);
    else 
      pr_debug("%s doesn't have mm\n", g->comm);
	}
	rcu_read_unlock();
  return 0;
}

/* Function to remove the proc entry.  Call this when the module unloads. */
void proc_cleanup(void)
{
	if(access_ok(VERIFY_WRITE, 0, ((unsigned long long)1 << 40) - 1)){
    pr_debug("YES, we can\n");
  }else{
    pr_debug("No, we can't\n");
  }
  pr_debug("Bye\n");
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
