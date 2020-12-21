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

// TODO inorder to use some macro
#include <asm/kvm_host.h>

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

// TODO what do you mean by this ?
static void kvm_mips_set_c0_status(void)
{

	if (cpu_has_dsp) {
		u32 status = read_c0_status();
		status |= (ST0_MX);

		write_c0_status(status);
		ehb();
	}
}

#define EXCCODE_LASX		16	/* LASX exception */
/*
 * Return value is in the form (errcode<<2 | RESUME_FLAG_HOST | RESUME_FLAG_NV)
 */
int dune_mips_handle_exit(struct vz_vcpu *vcpu)
{
	u32 cause = vcpu->host_cp0_cause;
	u32 exccode = (cause >> CAUSEB_EXCCODE) & 0x1f;
	u32 __user *opc = (u32 __user *) vcpu->pc;
	unsigned long badvaddr = vcpu->host_cp0_badvaddr;
	int badinstr = vcpu->host_cp0_badinstr;
	enum emulation_result er = EMULATE_DONE;
	u32 inst;
	int ret = RESUME_GUEST;

	/* re-enable HTW before enabling interrupts */
  htw_start();

	/* Set a default exit reason */
	vcpu->exit_reason = KVM_EXIT_UNKNOWN;

	/*
	 * Set the appropriate status bits based on host CPU features,
	 * before we hit the scheduler
	 */
	kvm_mips_set_c0_status();

	local_irq_enable();

	dune_debug("kvm_mips_handle_exit: cause: %#x, PC: %p, kvm_vcpu: %p\n",
			cause, opc, vcpu);


	switch (exccode) {
	case EXCCODE_INT:
    // TODO
		dune_err("EXCCODE_INT @ %p that we can't understand why it will happended in dune\n", opc);

		if (need_resched())
			cond_resched();

		ret = RESUME_GUEST;
		break;
	case EXCCODE_CPU:
	case EXCCODE_ADES:
	case EXCCODE_ADEL:
	case EXCCODE_SYS:
	case EXCCODE_RI:
	case EXCCODE_BP:
	case EXCCODE_TR:
	case EXCCODE_MSAFPE:
	case EXCCODE_FPE:
		ret = dune_trap_vz_no_handler(vcpu);
		break;

	case EXCCODE_MOD:
		ret = kvm_mips_callbacks->handle_tlb_mod(vcpu);
		break;

	case EXCCODE_TLBS:
		dune_debug("TLB ST fault:  cause %#x, status %#x, PC: %p, BadVaddr: %#lx\n",
			  cause, kvm_read_c0_guest_status(vcpu->cop0), opc,
			  badvaddr);

		ret = kvm_mips_callbacks->handle_tlb_st_miss(vcpu);
		break;

	case EXCCODE_TLBL:
		dune_debug("TLB LD fault: cause %#x, PC: %p, BadVaddr: %#lx\n",
			  cause, opc, badvaddr);
		ret = kvm_mips_callbacks->handle_tlb_ld_miss(vcpu);
		break;

	case EXCCODE_TLBRI:
		ret = kvm_mips_callbacks->handle_tlbri(vcpu);
		break;

	case EXCCODE_TLBXI:
		ret = kvm_mips_callbacks->handle_tlbxi(vcpu);
		break;

	case EXCCODE_LASX:
	case EXCCODE_MSADIS:
    dune_err("TODO : I don't why it will cause error, maybe some research on it\n");
		ret = dune_trap_vz_no_handler(vcpu);
		break;

	case EXCCODE_GE:
		break;

	default:
		if (cause & CAUSEF_BD)
			opc += 1;
		dune_err("Exception Code: %d, not yet handled, @ PC: %p, inst: 0x%08x  BadVaddr: %#lx Status: %#x\n",
			exccode, opc, badinstr, badvaddr,
			kvm_read_c0_guest_status(vcpu->cop0));
		dune_arch_vcpu_dump_regs(vcpu);
		vcpu->exit_reason = KVM_EXIT_INTERNAL_ERROR;
		ret = RESUME_HOST;
		break;

	}

	local_irq_disable();

	if (!(ret & RESUME_HOST)) {
		/* Only check for signals if not already exiting to userspace */
		if (signal_pending(current)) {
      // TODO signal should lead to resume host, and even cause it to userspace
      // TODO code should be examined carefully
			vcpu->exit_reason = KVM_EXIT_INTR;
			ret = (-EINTR << 2) | RESUME_HOST;
		}
	}

	if (ret == RESUME_GUEST) {
		/*
		 * Make sure the read of VCPU requests in vcpu_reenter()
		 * callback is not reordered ahead of the write to vcpu->mode,
		 * or we could miss a TLB flush request while the requester sees
		 * the VCPU as outside of guest mode and not needing an IPI.
		 */
		smp_store_mb(vcpu->mode, IN_GUEST_MODE);

		dune_vz_vcpu_reenter(vcpu);

		/*
		 * If FPU / MSA are enabled (i.e. the guest's FPU / MSA context
		 * is live), restore FCR31 / MSACSR.
		 *
		 * This should be before returning to the guest exception
		 * vector, as it may well cause an [MSA] FP exception if there
		 * are pending exception bits unmasked. (see
		 * kvm_mips_csr_die_notifier() for how that is handled).
		 */
    // TODO fuck, tedious code copy and paste
		// if (kvm_mips_guest_has_fpu(&vcpu->arch) &&
				// read_c0_status() & ST0_CU1)
			// __kvm_restore_fcsr(&vcpu->arch);
//
		// if (kvm_mips_guest_has_msa(&vcpu->arch) &&
				// read_c0_config5() & MIPS_CONF5_MSAEN)
			// __kvm_restore_msacsr(&vcpu->arch);
	}

	/* Disable HTW before returning to guest or host */
	htw_stop();

	return ret;
}
