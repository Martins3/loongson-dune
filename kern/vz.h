#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/kvm_types.h> // TODO remove anything about kvm ?
#include <asm/syscall.h>
#include <linux/mmu_notifier.h>
#include <linux/types.h>

#include "dune.h"

extern __init int vz_init(void);
extern void vz_exit(void);
extern int vz_launch(struct dune_config *conf, int64_t *ret_code);

#define N_MIPS_COPROC_REGS	32
#define N_MIPS_COPROC_SEL	8
struct mips_coproc {
	unsigned long reg[N_MIPS_COPROC_REGS][N_MIPS_COPROC_SEL];
};

struct dune_mips_tlb {
	long tlb_mask;
	long tlb_hi;
	long tlb_lo[2];
};
// TODO why guest TLB size is arch unrelated ?
#define KVM_MIPS_GUEST_TLB_SIZE	64

// TODO do we need the name of all the GPRs, why x86 dune need this ?
enum vz_reg {
  // TODO
	NR_VCPU_REGS
};


// copied from struct kvm_arch
// TODO shared by all vcpu belonging to one cpu
struct dune_tdp {
	/* Mask of CPUs needing GPA ASID flush */
	cpumask_t asid_flush_mask;

	/* Guest physical mm */
	struct mm_struct gpa_mm;
};

struct vz_vcpu {

	struct list_head list;
	int cpu;
	int vpid; // TODO mips doesn't use vpid ?
	int launched; // TODO launched and conf != NULL

	struct mmu_notifier mmu_notifier;


	int shutdown;
  // TODO seems two ret_code ?
	int ret_code;

  // TODO
	// sys_call_ptr_t *syscall_tbl;
	struct dune_config *conf;
  
	void *guest_ebase;

  // TODO this is what to do next !
	int (*vcpu_run)(struct vz_vcpu *vcpu);

	/* Host KSEG0 address of the EI/DI offset */
	void *kseg0_commpage;

	/* COP0 State */
	struct mips_coproc *cop0;

	/* GPRS */
	unsigned long gprs[NR_VCPU_REGS];
	unsigned long hi;
	unsigned long lo;
  // ?? pc
	unsigned long pc;

	/* S/W Based TLB for guest */
	struct dune_mips_tlb guest_tlb[KVM_MIPS_GUEST_TLB_SIZE];

	/* FPU State */
  // TODO what's fcr31 ?
	struct mips_fpu_struct fpu;

	/* vcpu's vzguestid is different on each host cpu in an smp system */
	u64 vzguestid[NR_CPUS];
  
  // TODO this is originally in kvm_run::exit_reason
	/* out */
	u32 exit_reason;

  // TODO why I need this ?
	/* Last CPU the VCPU state was loaded on */
	int last_sched_cpu;
	/* Last CPU the VCPU actually executed guest code on */
	int last_exec_cpu;

	/* wired guest TLB entries */
	struct dune_mips_tlb *wired_tlb;
	unsigned int wired_tlb_limit;
	unsigned int wired_tlb_used;
  
  struct dune_tdp * tbp;
};



/* entry.c */
int dune_mips_entry_setup(void);
void *dune_mips_build_vcpu_run(void *addr);
void *dune_mips_build_tlb_refill_exception(void *addr, void *handler);
void *dune_mips_build_exception(void *addr, void *handler);
void *dune_mips_build_exit(void *addr);

/* mips.c */
void dump_handler(const char *symbol, void *start, void *end);

/* commpage.h */
struct kvm_mips_commpage {
	/* COP0 state is mapped into Guest kernel via commpage */
	struct mips_coproc cop0;
};

#define KVM_MIPS_COMM_EIDI_OFFSET       0x0

void dune_mips_commpage_init(struct vz_vcpu *vcpu);

/* tlb.h */

extern unsigned long GUESTID_MASK;
extern unsigned long GUESTID_FIRST_VERSION;
extern unsigned long GUESTID_VERSION_MASK;

void dune_vz_load_guesttlb(const struct dune_mips_tlb *buf, unsigned int index,
			  unsigned int count);
void dune_vz_save_guesttlb(struct dune_mips_tlb *buf, unsigned int index,
			  unsigned int count);

#define dune_err(fmt, ...)                                                     \
	pr_err("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)
#define dune_info(fmt, ...)                                                    \
	pr_info("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)
#define dune_debug(fmt, ...)                                                   \
	pr_debug("dune [%i]: " fmt, task_pid_nr(current), ##__VA_ARGS__)


