#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/types.h>
// #include <asm/uaccess.h>

#include "vz.h"
#include "dune.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A driver for Dune");

static int dune_enter(struct dune_config *conf, int64_t *ret)
{
	return vz_launch(conf, ret);
}

static long dune_dev_ioctl(struct file *filp, unsigned int ioctl,
                           unsigned long arg) {
	long r = -EINVAL;
	struct dune_config conf;
	struct dune_layout layout;

	switch (ioctl) {
	case DUNE_ENTER:
		r = copy_from_user(&conf, (int __user *) arg,
				   sizeof(struct dune_config));
		if (r) {
			r = -EIO;
			goto out;
		}

		r = dune_enter(&conf, &conf.ret);
		if (r)
			break;

		r = copy_to_user((void __user *)arg, &conf,
				 sizeof(struct dune_config));
		if (r) {
			r = -EIO;
			goto out;
		}
		break;

	case DUNE_GET_SYSCALL:
		// rdmsrl(MSR_LSTAR, r);
		printk(KERN_INFO "R %lx\n", (unsigned long) r);
		break;

	case DUNE_GET_LAYOUT:
		// layout.phys_limit = (1UL << boot_cpu_data.x86_phys_bits);
		// layout.base_map = LG_ALIGN(current->mm->mmap_base) - GPA_MAP_SIZE;
		// layout.base_stack = LG_ALIGN(current->mm->start_stack) - GPA_STACK_SIZE;
		r = copy_to_user((void __user *)arg, &layout,
				 sizeof(struct dune_layout));
		if (r) {
			r = -EIO;
			goto out;
		}
		break;

	case DUNE_TRAP_ENABLE:
    // TODO
		// r = dune_trap_enable(arg);
		break;

	case DUNE_TRAP_DISABLE:
    // TODO
		// r = dune_trap_disable(arg);
		break;

	default:
		return -ENOTTY;
	}

out:
	return r;
}

static int dune_dev_release(struct inode *inode, struct file *file) {
  return 0;
}

static const struct file_operations dune_chardev_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = dune_dev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = dune_dev_ioctl,
#endif
    .llseek = noop_llseek,
    .release = dune_dev_release,
};

static struct miscdevice dune_dev = {
    DUNE_MINOR,
    "dune",
    &dune_chardev_ops,
};

static int __init dune_init(void) {
  int r;

  printk(KERN_ERR "Dune module loaded\n");

  if ((r = vz_init())) {
    printk(KERN_ERR "dune: failed to initialize vmx\n");
    return r;
  }

  r = misc_register(&dune_dev);
  if (r) {
    printk(KERN_ERR "dune: misc device register failed\n");
    vz_exit();
  }

  return r;
}

static void __exit dune_exit(void) {
  misc_deregister(&dune_dev);
  vz_exit();
}

module_init(dune_init);
module_exit(dune_exit);
