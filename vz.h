#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/kvm_types.h>
#include <asm/syscall.h>
#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/kvm_types.h>

#include "dune.h"

extern __init int vz_init(void);
extern __init int vz_exit(void);

extern int vmx_launch(struct dune_config *conf, int64_t *ret_code);
 

enum vmx_reg {
  // TODO
	NR_VCPU_REGS
};
 
struct vmx_vcpu {
	struct list_head list;
	int cpu;
	int vpid;
	int launched;

	struct mmu_notifier mmu_notifier;

	u64 regs[NR_VCPU_REGS];

	int shutdown;
	int ret_code;

  // TODO
	// sys_call_ptr_t *syscall_tbl;
	struct dune_config *conf;
};
