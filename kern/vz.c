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

#include <asm/traps.h>
#include <asm/bitops.h>

#include "dune.h"
#include "vz.h"

static void vz_get_cpu(struct vz_vcpu *vcpu);
static void vz_put_cpu(struct vz_vcpu *vcpu);

#define dune_err(fmt, ...) \
	pr_err("dune [%i]: " fmt, task_pid_nr(current), ## __VA_ARGS__)
#define dune_info(fmt, ...) \
	pr_info("dune [%i]: " fmt, task_pid_nr(current), ## __VA_ARGS__)
#define dune_debug(fmt, ...) \
	pr_debug("dune [%i]: " fmt, task_pid_nr(current), ## __VA_ARGS__)

static LIST_HEAD(vcpus);

/**
 * vmx_init - the main initialization routine for this driver
 */
__init int vz_init(void)
{
  // TODO this is all the thing we should init ?
  kvm_mips_entry_setup();
}

void vz_exit(void)
{
  // TODO
}

/**
 * vmx_setup_registers - setup general purpose registers
 */
static void vz_setup_registers(struct vz_vcpu *vcpu, struct dune_config *conf)
{
  // TODO
}

/**
 * vmx_copy_registers_to_conf - copy registers to dune_config
 */
static void vz_copy_registers_to_conf(struct vz_vcpu *vcpu, struct dune_config *conf){

}



/**
 * vmx_get_cpu - called before using a cpu
 * @vcpu: VCPU that will be loaded.
 *
 * Disables preemption. Call vmx_put_cpu() when finished.
 */
static void vz_get_cpu(struct vz_vcpu *vcpu)
{
  // TODO

}

/**
 * vmx_put_cpu - called after using a cpu
 * @vcpu: VCPU that was loaded.
 */
static void vz_put_cpu(struct vz_vcpu *vcpu)
{
  // TODO
}

/**
 * vz_run_vcpu - launches the CPU into non-root mode
 * @vcpu: the vmx instance to launch
 */
static int __noclone vz_run_vcpu(struct vz_vcpu *vcpu)
{
  // TODO
}



/**
 * vz_launch - the main loop for a VMX Dune process
 * @conf: the launch configuration
 */
int vz_launch(struct dune_config *conf, int64_t *ret_code)
{
	int ret, done = 0;
	u32 exit_intr_info;
	bool rescheduled = false;
	struct vz_vcpu *vcpu = vz_create_vcpu(conf);
	if (!vcpu)
		return -ENOMEM;

  // TODO do we have vpid supported ?
	// printk(KERN_ERR "vmx: created VCPU (VPID %d)\n",
				 // vcpu->vpid);

	while (1) {
	  pr_err("vmx: prepare run vcpu\n");
		vz_get_cpu(vcpu);
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
			vz_put_cpu(vcpu);
			cond_resched();
			continue;
		}

		if (signal_pending(current)) {

			local_irq_enable();
			vz_put_cpu(vcpu);


			vcpu->ret_code = DUNE_RET_SIGNAL;
			break;
		}

		// setup_perf_msrs(vcpu);
		// vmx_handle_queued_interrupts(vcpu);

		ret = vz_run_vcpu(vcpu);

    // TODO
		/* We need to handle NMIs before interrupts are enabled */
		// exit_intr_info = vmcs_read32(VM_EXIT_INTR_INFO);
		// if ((exit_intr_info & INTR_INFO_INTR_TYPE_MASK) == INTR_TYPE_NMI_INTR &&
				// (exit_intr_info & INTR_INFO_VALID_MASK)) {
			// asm("int $2");
		// }

		// vmx_handle_external_interrupt(vcpu, exit_intr_info);

		local_irq_enable();

		if (ret == EXIT_REASON_VMCALL ||
			ret == EXIT_REASON_CPUID ||
			ret == EXIT_REASON_MSR_WRITE) {
      // problem fixed, so move to next instruction
			vmx_step_instruction();
		}

		vz_put_cpu(vcpu);

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

	vz_copy_registers_to_conf(vcpu, conf);

	return 0;
}

/**
 * vmx_create_vcpu - allocates and initializes a new virtual cpu
 *
 * Returns: A new VCPU structure
 */
static struct vz_vcpu * vz_create_vcpu(struct dune_config *conf)
{ 
  // struct kvm_vcpu *kvm_arch_vcpu_create(struct kvm *kvm, unsigned int id)
  struct vz_vcpu * vcpu;

	if (conf->vcpu) {
		/* This Dune configuration already has a VCPU. */
		vcpu = (struct vz_vcpu *) conf->vcpu;
		vz_get_cpu(vcpu);
		vz_setup_registers(vcpu, conf);
		vz_put_cpu(vcpu);
		return vcpu;
	}

	vcpu = kmalloc(sizeof(struct vz_vcpu), GFP_KERNEL);
	if (!vcpu)
		return NULL;

	memset(vcpu, 0, sizeof(*vcpu));

	list_add(&vcpu->list, &vcpus);

	vcpu->conf = conf;
	conf->vcpu = (u64) vcpu;

  // TODO
  // 1. allocate gebase
  // 2. tlb refill : gebase + 2000
  // 3. normal exception
  // 4. run
  
  return NULL;
}


static struct kvm_vcpu *vz_create_vcpu(struct dune_config * conf)
{
	int err, size;
	void *gebase, *p, *handler, *refill_start, *refill_end;
	int i;
	struct vz_vcpu * vcpu;


  vcpu = kzalloc(sizeof(struct vz_vcpu), GFP_KERNEL);
	if (!vcpu) {
		err = -ENOMEM;
		goto out;
	}

	/*
	 * Allocate space for host mode exception handlers that handle
	 * guest mode exits
	 */
	if (cpu_has_veic || cpu_has_vint)
		size = 0x200 + VECTORSPACING * 64;
	else
		size = 0x4000;

	gebase = kzalloc(ALIGN(size, PAGE_SIZE), GFP_KERNEL);

	if (!gebase) {
		err = -ENOMEM;
		goto out_uninit_cpu;
	}
	dune_debug("Allocated %d bytes for KVM Exception Handlers @ %p\n",
		  ALIGN(size, PAGE_SIZE), gebase);

	/*
	 * Check new ebase actually fits in CP0_EBase. The lack of a write gate
	 * limits us to the low 512MB of physical address space. If the memory
	 * we allocate is out of range, just give up now.
	 */
	if (!cpu_has_ebase_wg && virt_to_phys(gebase) >= 0x20000000) {
		dune_err("CP0_EBase.WG required for guest exception base %pK\n",
			gebase);
		err = -ENOMEM;
		goto out_free_gebase;
	}

	/* Save new ebase */
	vcpu->guest_ebase = gebase;

	/* Build guest exception vectors dynamically in unmapped memory */
	handler = gebase + 0x2000;

	/* TLB refill (or XTLB refill on 64-bit VZ where KX=1) */
	refill_start = gebase;
	// IS_ENABLED(CONFIG_KVM_MIPS_VZ) && IS_ENABLED(CONFIG_64BIT)
  refill_start += 0x080;
	refill_end = kvm_mips_build_tlb_refill_exception(refill_start, handler);

	/* General Exception Entry point */
	dune_mips_build_exception(gebase + 0x180, handler);

	/* For vectored interrupts poke the exception code @ all offsets 0-7 */
	for (i = 0; i < 8; i++) {
		dune_debug("L1 Vectored handler @ %p\n",
			  gebase + 0x200 + (i * VECTORSPACING));
		kvm_mips_build_exception(gebase + 0x200 + i * VECTORSPACING,
					 handler);
	}

  // TODO what's difference between this and host normal exit handler ?
  // - what does it contains ?
	/* General exit handler */
	p = handler;
	p = kvm_mips_build_exit(p);

	/* Guest entry routine */
	vcpu->vcpu_run = p;
	p = kvm_mips_build_vcpu_run(p);

	/* Dump the generated code */
	pr_debug("#include <asm/asm.h>\n");
	pr_debug("#include <asm/regdef.h>\n");
	pr_debug("\n");
	dump_handler("kvm_vcpu_run", vcpu->vcpu_run, p);
	dump_handler("kvm_tlb_refill", refill_start, refill_end);
	dump_handler("kvm_gen_exc", gebase + 0x180, gebase + 0x200);
	dump_handler("kvm_exit", gebase + 0x2000, vcpu->vcpu_run);
	dune_info("start %lx end %lx handler %lx\n",(unsigned long)refill_start,(unsigned long)refill_end,(unsigned long)handler);

	/* Invalidate the icache for these ranges */
	flush_icache_range((unsigned long)gebase,
			   (unsigned long)gebase + ALIGN(size, PAGE_SIZE));

	/*
	 * Allocate comm page for guest kernel, a TLB will be reserved for
	 * mapping GVA @ 0xFFFF8000 to this page
	 */
	vcpu->arch.kseg0_commpage = kzalloc(PAGE_SIZE << 1, GFP_KERNEL);

	if (!vcpu->arch.kseg0_commpage) {
		err = -ENOMEM;
		goto out_free_gebase;
	}

	dune_debug("Allocated COMM page @ %p\n", vcpu->arch.kseg0_commpage);
	kvm_mips_commpage_init(vcpu);

	dune_info("guest cop0 page @ %lx gprs @ %lx tlb @ %lx pc @ %lx\n",
		  (unsigned long)vcpu->arch.cop0, (unsigned long)vcpu->arch.gprs,
		  (unsigned long)vcpu->arch.guest_tlb, (unsigned long)&vcpu->arch.pc);
	dune_info("pending exception @ %lx\n", (ulong)&vcpu->arch.pending_exceptions);
	dune_info("fcr31 @ %lx\n", (ulong)&vcpu->arch.fpu.fcr31);
	dune_info("count_bias @ %lx period @ %lx\n", (ulong)&vcpu->arch.count_bias, (ulong)&vcpu->arch.count_period);
	dune_info("vzguestid @ %lx\n", (unsigned long)vcpu->arch.vzguestid);
	dune_info("exit_reason @ %lx\n", (ulong)&vcpu->run->exit_reason);
	dune_info("run @ %lx\n", (ulong)vcpu->run);
	dune_info("wait @ %lx\n", (ulong)&vcpu->arch.wait);
	dune_info("stable timer @ %lx\n", (ulong)&vcpu->arch.stable_timer_tick);
	dune_info("use stable timer %x\n", vcpu->kvm->arch.use_stable_timer);
	dune_info("\n\n");
	/* Init */
	vcpu->arch.last_sched_cpu = -1;
	vcpu->arch.last_exec_cpu = -1;

	return vcpu;

out_free_gebase:
	kfree(gebase);

out_uninit_cpu:
	kvm_vcpu_uninit(vcpu);

out_free_cpu:
	kfree(vcpu);

out:
	return ERR_PTR(err);
}
