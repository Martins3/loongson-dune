#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/bootmem.h>
#include <linux/mod_devicetable.h>

#include <asm/fpu.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/mmu_context.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/cpufeature.h>

#include "vz.h"

void dump_handler(const char *symbol, void *start, void *end)
{
	u32 *p;

	pr_debug("LEAF(%s)\n", symbol);

	pr_debug("\t.set push\n");
	pr_debug("\t.set noreorder\n");

	for (p = start; p < (u32 *)end; ++p)
		pr_debug("\t.word\t0x%08x\t\t# %p\n", *p, p);

	pr_debug("\t.set\tpop\n");

	pr_debug("\tEND(%s)\n", symbol);
}

// TODO what a complex way to assign a vcpu->cop0
void dune_mips_commpage_init(struct vz_vcpu *vcpu)
{
	struct kvm_mips_commpage *page = vcpu->kseg0_commpage;

	/* Specific init values for fields */
	vcpu->cop0 = &page->cop0;
}
