#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/kvm_types.h> // TODO remove anything about kvm ?
#include <asm/syscall.h>
#include <linux/mmu_notifier.h>
#include <linux/types.h>

#include "dune.h"

/* entry.c : Building of entry/exception code */
int kvm_mips_entry_setup(void);
void *kvm_mips_build_vcpu_run(void *addr);
void *kvm_mips_build_tlb_refill_exception(void *addr, void *handler);
void *dune_mips_build_exception(void *addr, void *handler);
void *kvm_mips_build_exit(void *addr);

/* mips.c : */
void dump_handler(const char *symbol, void *start, void *end);

extern __init int vz_init(void);
extern void vz_exit(void);

extern int vmx_launch(struct dune_config *conf, int64_t *ret_code);
 

enum vz_reg {
  // TODO
	NR_VCPU_REGS
};
 
struct vz_vcpu {

	struct list_head list;
	int cpu;
	int vpid; // TODO mips doesn't use vpid ?
	int launched; // TODO launched and conf != NULL

	struct mmu_notifier mmu_notifier;

	unsigned long gprs[NR_VCPU_REGS];

	int shutdown;
	int ret_code;

	/* Last CPU the VCPU state was loaded on */
	int last_sched_cpu;
	/* Last CPU the VCPU actually executed guest code on */

  // TODO
	// sys_call_ptr_t *syscall_tbl;
	struct dune_config *conf;
  
	void *guest_ebase;

  // TODO this is what to do next !
	int (*vcpu_run)(struct kvm_run *run, struct kvm_vcpu *vcpu);
};
