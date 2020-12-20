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
#include <asm/fpu.h>
#include <asm/mipsregs.h>
#include <asm/tlb.h>

#include "dune.h"
#include "vz.h"

static struct vz_vcpu * dune_vz_create_vcpu(struct dune_config *conf);


#ifndef VECTORSPACING
#define VECTORSPACING 0x100 /* for EI/VI mode */
#endif

static LIST_HEAD(vcpus);

/* Pointers to last VCPU loaded on each physical CPU */
static struct vz_vcpu *last_vcpu[NR_CPUS];
/* Pointers to last VCPU executed on each physical CPU */
static struct vz_vcpu *last_exec_vcpu[NR_CPUS];


#define guestid_cache(cpu)	(cpu_data[cpu].guestid_cache)

/**
 * vmx_init - the main initialization routine for this driver
 */
__init int vz_init(void)
{
	// TODO this is all the thing we should init ?
	dune_mips_entry_setup();
	return 0;
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
static void vz_copy_registers_to_conf(struct vz_vcpu *vcpu,
				      struct dune_config *conf)
{
}

static void dune_vcpu_load(struct vz_vcpu *vcpu)
{
	// TODO 	vcpu_load in kvm
}

static void dune_vcpu_put(struct vz_vcpu *vcpu)
{
	// TODO
}

static void dune_vz_get_new_guestid(unsigned long cpu)
{
	unsigned long guestid = guestid_cache(cpu);

	if (!(++guestid & GUESTID_MASK)) {
		if (cpu_has_vtag_icache)
			flush_icache_all();

		if (!guestid)		/* fix version if needed */
			guestid = GUESTID_FIRST_VERSION;

		++guestid;		/* guestid 0 reserved for root */

		/* start new guestid cycle */
		dune_vz_local_flush_roottlb_all_guests();
		dune_vz_local_flush_guesttlb_all();
	}

	guestid_cache(cpu) = guestid;
}


static void dune_vz_vcpu_load_tlb(struct vz_vcpu *vcpu, int cpu)
{
	bool migrated;
	/*
	 * Are we entering guest context on a different CPU to last time?
	 * If so, the VCPU's guest TLB state on this CPU may be stale.
	 */
	migrated = (vcpu->last_exec_cpu != cpu);
	vcpu->last_exec_cpu = cpu;

	/*
	 * A vcpu's GuestID is set in GuestCtl1.ID when the vcpu is loaded and
	 * remains set until another vcpu is loaded in.  As a rule GuestRID
	 * remains zeroed when in root context unless the kernel is busy
	 * manipulating guest tlb entries.
	 */
	if (cpu_has_guestid) {
		/*
		 * Check if our GuestID is of an older version and thus invalid.
		 *
		 * We also discard the stored GuestID if we've executed on
		 * another CPU, as the guest mappings may have changed without
		 * hypervisor knowledge.
		 */
		if (migrated ||
		    (vcpu->vzguestid[cpu] ^ guestid_cache(cpu)) &
					GUESTID_VERSION_MASK) {
			dune_vz_get_new_guestid(cpu);
			vcpu->vzguestid[cpu] = guestid_cache(cpu);
		}

		/* Restore GuestID */
		change_c0_guestctl1(GUESTID_MASK, vcpu->vzguestid[cpu]);
	} else {
    dune_err("MIPS VZ and Loongson manual implys 3a4000 cpu has guestid, but in fact not\n");
  }
}

static void dune_vz_vcpu_load_wired(struct vz_vcpu *vcpu)
{
	/* Load wired entries into the guest TLB */
	if (vcpu->wired_tlb)
		dune_vz_load_guesttlb(vcpu->wired_tlb, 0,
				     vcpu->wired_tlb_used);
}

static void dune_vz_vcpu_save_wired(struct vz_vcpu *vcpu)
{
	unsigned int wired = read_gc0_wired();
	struct dune_mips_tlb *tlbs;
	int i;

	/* Expand the wired TLB array if necessary */
	wired &= MIPSR6_WIRED_WIRED;
	if (wired > vcpu->wired_tlb_limit) {
		tlbs = krealloc(vcpu->wired_tlb, wired *
				sizeof(*vcpu->wired_tlb), GFP_ATOMIC);
		if (WARN_ON(!tlbs)) {
			/* Save whatever we can */
			wired = vcpu->wired_tlb_limit;
		} else {
			vcpu->wired_tlb = tlbs;
			vcpu->wired_tlb_limit = wired;
		}
	}

	if (wired)
		/* Save wired entries from the guest TLB */
		dune_vz_save_guesttlb(vcpu->wired_tlb, 0, wired);
	/* Invalidate any dropped entries since last time */
	for (i = wired; i < vcpu->wired_tlb_used; ++i) {
		vcpu->wired_tlb[i].tlb_hi = UNIQUE_GUEST_ENTRYHI(i);
		vcpu->wired_tlb[i].tlb_lo[0] = 0;
		vcpu->wired_tlb[i].tlb_lo[1] = 0;
		vcpu->wired_tlb[i].tlb_mask = 0;
	}
	vcpu->wired_tlb_used = wired;
}

static int dune_vz_vcpu_run(struct vz_vcpu *vcpu)
{
	int cpu = smp_processor_id();
	int r;

  // TODO kvm_vz_check_requests used for flush remote TLB, maybe it's useless 
	// kvm_vz_check_requests(vcpu, cpu);

  dune_vz_vcpu_load_tlb(vcpu, cpu);
  dune_vz_vcpu_load_wired(vcpu);

	r = vcpu->vcpu_run(vcpu);
  // TODO kvm_mips_build_ret_to_host should return to here. ensure it.

  // TODO where is dune_vz_vcpu_save_tlb, when entering the CPU
  // TODO dune_vz_load_tlb doesn't work as it's name implys, it handle something like guestid
  // TODO vz_vcpu::guesttlb is unused
  dune_vz_vcpu_save_wired(vcpu);

	return r;
}

/**
 * vz_launch - the main loop for a VMX Dune process
 * @conf: the launch configuration
 */
int vz_launch(struct dune_config *conf, int64_t *ret_code)
{
	int r = -EINTR;
	struct vz_vcpu *vcpu = dune_vz_create_vcpu(conf);
	if (!vcpu)
		return -ENOMEM;
	printk(KERN_ERR "vmx: stopping VCPU (VPID %d)\n", vcpu->vpid);

	dune_vcpu_load(vcpu);

  lose_fpu(1);

	local_irq_disable();

  r = dune_vz_vcpu_run(vcpu);
	local_irq_enable();

	dune_vcpu_put(vcpu);
	*ret_code = vcpu->ret_code;

	vz_copy_registers_to_conf(vcpu, conf);

	return 0;
}

static int dune_vz_vcpu_init(struct vz_vcpu *vcpu)
{
	int i;

	for_each_possible_cpu (i)
		vcpu->vzguestid[i] = 0;

	return 0;
}

static void dune_vz_vcpu_uninit(struct vz_vcpu *vcpu)
{
	int cpu;

	/*
	 * If the VCPU is freed and reused as another VCPU, we don't want the
	 * matching pointer wrongly hanging around in last_vcpu[] or
	 * last_exec_vcpu[].
	 */
	for_each_possible_cpu (cpu) {
		if (last_vcpu[cpu] == vcpu)
			last_vcpu[cpu] = NULL;
		if (last_exec_vcpu[cpu] == vcpu)
			last_exec_vcpu[cpu] = NULL;
	}
}

// TODO remove these two redundant function later
void dune_vcpu_uninit(struct vz_vcpu *vcpu)
{
	dune_vz_vcpu_uninit(vcpu);
}

int dune_vcpu_init(struct vz_vcpu *vcpu)
{
	// TODO we will check the preempted / vcpu_load related stuff later
	// vcpu->preempted = false;
	return dune_vz_vcpu_init(vcpu);
}

// TODO don't include kvm_host.h
static struct vz_vcpu * dune_vz_create_vcpu(struct dune_config *conf)
{
	int err, size;
	void *gebase, *p, *handler, *refill_start, *refill_end;
	int i;
	struct vz_vcpu *vcpu;

	if (conf->vcpu) {
		/* This Dune configuration already has a VCPU. */
		vcpu = (struct vz_vcpu *)conf->vcpu;
    // TODO
		vz_setup_registers(vcpu, conf);
		return vcpu;
	}

	vcpu = kzalloc(sizeof(struct vz_vcpu), GFP_KERNEL);
	if (!vcpu) {
		err = -ENOMEM;
		goto out;
	}

  // TODO merge following 4 statement to dune_vcpu_init ?
	memset(vcpu, 0, sizeof(*vcpu));
	list_add(&vcpu->list, &vcpus);
	vcpu->conf = conf;
	conf->vcpu = (u64)vcpu;
	err = dune_vcpu_init(vcpu);

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
	refill_end =
		dune_mips_build_tlb_refill_exception(refill_start, handler);

	/* General Exception Entry point */
	dune_mips_build_exception(gebase + 0x180, handler);

	/* For vectored interrupts poke the exception code @ all offsets 0-7 */
	for (i = 0; i < 8; i++) {
		dune_debug("L1 Vectored handler @ %p\n",
			   gebase + 0x200 + (i * VECTORSPACING));
		dune_mips_build_exception(gebase + 0x200 + i * VECTORSPACING,
					  handler);
	}

	/* General exit handler */
	p = handler;
	p = dune_mips_build_exit(p);

	/* Guest entry routine */
	vcpu->vcpu_run = p;
	p = dune_mips_build_vcpu_run(p);

	/* Dump the generated code */
	pr_debug("#include <asm/asm.h>\n");
	pr_debug("#include <asm/regdef.h>\n");
	pr_debug("\n");
	dump_handler("kvm_vcpu_run", vcpu->vcpu_run, p);
	dump_handler("kvm_tlb_refill", refill_start, refill_end);
	dump_handler("kvm_gen_exc", gebase + 0x180, gebase + 0x200);
	dump_handler("kvm_exit", gebase + 0x2000, vcpu->vcpu_run);
	dune_info("start %lx end %lx handler %lx\n",
		  (unsigned long)refill_start, (unsigned long)refill_end,
		  (unsigned long)handler);

	/* Invalidate the icache for these ranges */
	flush_icache_range((unsigned long)gebase,
			   (unsigned long)gebase + ALIGN(size, PAGE_SIZE));

	/*
	 * Allocate comm page for guest kernel, a TLB will be reserved for
	 * mapping GVA @ 0xFFFF8000 to this page
	 */
	vcpu->kseg0_commpage = kzalloc(PAGE_SIZE << 1, GFP_KERNEL);

	if (!vcpu->kseg0_commpage) {
		err = -ENOMEM;
		goto out_free_gebase;
	}

	dune_debug("Allocated COMM page @ %p\n", vcpu->kseg0_commpage);
	dune_mips_commpage_init(vcpu);

	dune_info("guest cop0 page @ %lx gprs @ %lx tlb @ %lx pc @ %lx\n",
		  (unsigned long)vcpu->cop0, (unsigned long)vcpu->gprs,
		  (unsigned long)vcpu->guest_tlb, (unsigned long)&vcpu->pc);
	dune_info("fcr31 @ %lx\n", (ulong)&vcpu->fpu.fcr31);

	dune_info("vzguestid @ %lx\n", (unsigned long)vcpu->vzguestid);

	dune_info("exit_reason @ %lx\n", (ulong)&vcpu->exit_reason);
	// TODO examine arch->wait
	// dune_info("wait @ %lx\n", (ulong)&vcpu->arch.wait);

	dune_info("\n\n");
	/* Init */
	vcpu->last_sched_cpu = -1;
	vcpu->last_exec_cpu = -1;

	return vcpu;

	// TODO check labels below
out_free_gebase:
	kfree(gebase);

out_uninit_cpu:
	dune_vcpu_uninit(vcpu);

out_free_cpu:
	kfree(vcpu);

out:
	return ERR_PTR(err);
}
