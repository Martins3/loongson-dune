#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/tboot.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/percpu.h>
#include <linux/syscalls.h>
#include <linux/version.h>

#include "dune.h"
#include "vz.h"
/**
 * vmx_init - the main initialization routine for this driver
 */
__init int vz_init(void)
{
  // TODO
}

void vmx_exit(void)
{
  // TODO
}


/**
 * vmx_create_vcpu - allocates and initializes a new virtual cpu
 *
 * Returns: A new VCPU structure
 */
static struct vmx_vcpu * vmx_create_vcpu(struct dune_config *conf)
{
  // TODO
  return NULL;
}

/**
 * vmx_get_cpu - called before using a cpu
 * @vcpu: VCPU that will be loaded.
 *
 * Disables preemption. Call vmx_put_cpu() when finished.
 */
static void vmx_get_cpu(struct vmx_vcpu *vcpu)
{
  // TODO
}

/**
 * vmx_put_cpu - called after using a cpu
 * @vcpu: VCPU that was loaded.
 */
static void vmx_put_cpu(struct vmx_vcpu *vcpu)
{
  // TODO
}

/* vmx_handle_queued_interrupts - sometimes a posted interrupt is sent to a core
 * that is not currently in VMX non-root mode. In this case, the interrupt
 * is still pending in the posted interrupt descriptor, but it needs to be
 * inserted into the guest OS.
 *
 * Part of the algorithm here is from section 29.6 of the Intel manual.
 * According to the Intel manual (section 29.2.1), virtual interrupts are
 * delivered on VM entry, so we just need to set the vIRR and the RVI here,
 * and the interrupt will be delivered on VM entry.
 */
static void vmx_handle_queued_interrupts(struct vmx_vcpu *vcpu)
{
  // TODO
}

/**
 * vmx_run_vcpu - launches the CPU into non-root mode
 * @vcpu: the vmx instance to launch
 */
static int __noclone vmx_run_vcpu(struct vmx_vcpu *vcpu)
{
  // TODO
}

/**
 * vmx_copy_registers_to_conf - copy registers to dune_config
 */
static void vmx_copy_registers_to_conf(struct vmx_vcpu *vcpu, struct dune_config *conf){

}


/**
 * vmx_launch - the main loop for a VMX Dune process
 * @conf: the launch configuration
 */
int vmx_launch(struct dune_config *conf, int64_t *ret_code)
{
	int ret, done = 0;
	u32 exit_intr_info;
	bool rescheduled = false;
	struct vmx_vcpu *vcpu = vmx_create_vcpu(conf);
	if (!vcpu)
		return -ENOMEM;

  // TODO do we have vpid supported ?
	// printk(KERN_ERR "vmx: created VCPU (VPID %d)\n",
				 // vcpu->vpid);

	while (1) {
	  pr_err("vmx: prepare run vcpu\n");
		vmx_get_cpu(vcpu);
		if (rescheduled) {
      // TODO
			// update_vapic_addresses(vcpu);
			rescheduled = false;
		}

    // TODO FPU
		/*
		 * We assume that a Dune process will always use
		 * the FPU whenever it is entered, and thus we go
		 * ahead and load FPU state here. The reason is
		 * that we don't monitor or trap FPU usage inside
		 * a Dune process.
		 */
		// compat_fpu_restore();

		local_irq_disable();

		if (need_resched()) {
			rescheduled = true;
			local_irq_enable();
			vmx_put_cpu(vcpu);
			cond_resched();
			continue;
		}

		if (signal_pending(current)) {

			local_irq_enable();
			vmx_put_cpu(vcpu);


			vcpu->ret_code = DUNE_RET_SIGNAL;
			break;
		}

		// setup_perf_msrs(vcpu);
		vmx_handle_queued_interrupts(vcpu);

		ret = vmx_run_vcpu(vcpu);

    // TODO
		/* We need to handle NMIs before interrupts are enabled */
		// exit_intr_info = vmcs_read32(VM_EXIT_INTR_INFO);
		// if ((exit_intr_info & INTR_INFO_INTR_TYPE_MASK) == INTR_TYPE_NMI_INTR &&
				// (exit_intr_info & INTR_INFO_VALID_MASK)) {
			// asm("int $2");
		// }

		vmx_handle_external_interrupt(vcpu, exit_intr_info);

		local_irq_enable();

		if (ret == EXIT_REASON_VMCALL ||
			ret == EXIT_REASON_CPUID ||
			ret == EXIT_REASON_MSR_WRITE) {
      // problem fixed, so move to next instruction
			vmx_step_instruction();
		}

		vmx_put_cpu(vcpu);

		// if (ret == EXIT_REASON_VMCALL)
			// vmx_handle_syscall(vcpu);
    // else if (ret == EXIT_REASON_CPUID)
      // vmx_handle_cpuid(vcpu);
		// else if (ret == EXIT_REASON_EPT_VIOLATION)
			// done = vmx_handle_ept_violation(vcpu);
		// else if (ret == EXIT_REASON_EXCEPTION_NMI) {
      // if (vmx_handle_nmi_exception(vcpu))
        // done = 1;
		// } else if (ret == EXIT_REASON_MSR_WRITE) {
      // if (vmx_handle_msr_write(vcpu))
        // done = 1;
		// } else if (ret != EXIT_REASON_EXTERNAL_INTERRUPT) {
			// printk(KERN_INFO "unhandled exit: reason %d, exit qualification %x\n",
					// ret, vmcs_read32(EXIT_QUALIFICATION));
			// vcpu->ret_code = DUNE_RET_UNHANDLED_VMEXIT;
			// vmx_dump_cpu(vcpu);
			// done = 1;
		// }
//
		if (done || vcpu->shutdown)
			break;
	}

	printk(KERN_ERR "vmx: stopping VCPU (VPID %d)\n",
			vcpu->vpid);

	*ret_code = vcpu->ret_code;

	vmx_copy_registers_to_conf(vcpu, conf);

	return 0;
}
