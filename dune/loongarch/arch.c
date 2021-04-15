#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "arch.h"
#include "internal.h"
#include "../interface.h"

void arch_dump_regs(int debug_fd, struct kvm_regs regs)
{
	dprintf(debug_fd, "\n Registers:\n");
	dprintf(debug_fd, " ----------\n");
	dprintf(debug_fd, "$0   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[0],
		(unsigned long long)regs.gpr[1],
		(unsigned long long)regs.gpr[2],
		(unsigned long long)regs.gpr[3]);
	dprintf(debug_fd, "$4   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[4],
		(unsigned long long)regs.gpr[5],
		(unsigned long long)regs.gpr[6],
		(unsigned long long)regs.gpr[7]);
	dprintf(debug_fd, "$8   : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[8],
		(unsigned long long)regs.gpr[9],
		(unsigned long long)regs.gpr[10],
		(unsigned long long)regs.gpr[11]);
	dprintf(debug_fd, "$12  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[12],
		(unsigned long long)regs.gpr[13],
		(unsigned long long)regs.gpr[14],
		(unsigned long long)regs.gpr[15]);
	dprintf(debug_fd, "$16  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[16],
		(unsigned long long)regs.gpr[17],
		(unsigned long long)regs.gpr[18],
		(unsigned long long)regs.gpr[19]);
	dprintf(debug_fd, "$20  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[20],
		(unsigned long long)regs.gpr[21],
		(unsigned long long)regs.gpr[22],
		(unsigned long long)regs.gpr[23]);
	dprintf(debug_fd, "$24  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[24],
		(unsigned long long)regs.gpr[25],
		(unsigned long long)regs.gpr[26],
		(unsigned long long)regs.gpr[27]);
	dprintf(debug_fd, "$28  : %016llx %016llx %016llx %016llx\n",
		(unsigned long long)regs.gpr[28],
		(unsigned long long)regs.gpr[29],
		(unsigned long long)regs.gpr[30],
		(unsigned long long)regs.gpr[31]);

	dprintf(debug_fd, "pc  : %016llx\n", (unsigned long long)regs.pc);

	dprintf(debug_fd, "\n");
}

static void init_ebase(struct kvm_cpu *cpu)
{
	// 6.2.1
	// CSR.TLBRENTRY
	// CSR.MERRENTRY
	// CSR.EENTRY
	//
	// 可以利用 save 寄存器是曾经的 scratch 寄存器
	//
	// 直接映射窗口
	//
	// 中断信号被采样到 CSR.ESTA.IS 和 CSR.ECFG.LIE, 得到 13 bit 的中断向量
	//
	// - [ ] CS.ECFG.VS 是什么东西
  //
  // - [ ] 例外前模式信息，包括中断吗? 为什么以前不需要保存? 如果嵌套例外，比如 syscall 中间调用 syscall，那么怎么搞
  //
  // - [ ] 
}

struct csr_reg {
	struct kvm_one_reg reg;
	char name[100];
	u64 v;
};

#define CSR_INIT_REG(X)                                                        \
	{                                                                      \
		.reg = { .id = KVM_CSR_##X }, .name = #X, .v = INIT_VALUE_##X  \
	}

// TODO 验证这个数值
static const u64 DIRECT_MAP_BASE = 0x9800000000000000;
static void init_csr(struct kvm_cpu *cpu)
{
	if (!cpu->info.ebase)
		die("You forget to init ebase");
  
  u64 INIT_VALUE_KSCRATCH0 = (u64)cpu->info.ebase + DIRECT_MAP_BASE;

	u64 gg = KVM_CSR_CRMD;
	struct csr_reg one_regs[] = {
		CSR_INIT_REG(CRMD),
	};

	for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
		one_regs[i].reg.addr = (u64) & (one_regs[i].v);
	}

	for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
		if (ioctl(cpu->vcpu_fd, KVM_SET_ONE_REG, &(one_regs[i].reg)) <
		    0) {
			die("KVM_SET_ONE_REG %s", one_regs[i].name);
		} else {
			// pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
			// one_regs[i].v);
		}
	}
}

static int __attribute__((noinline))
kvm_launch(struct kvm_cpu *cpu, struct kvm_regs *regs, u64 is_guest)
{
	asm goto("\n\t"
		 "st.d $r0,  $r5, 0\n\t"
		 "st.d $r1,  $r5, 8\n\t"
		 "st.d $r2,  $r5, 16\n\t"
		 "st.d $r3,  $r5, 24\n\t"
		 "st.d $r4,  $r5, 32\n\t"
		 "st.d $r5,  $r5, 40\n\t"
		 "st.d $r6,  $r5, 48\n\t"
		 "st.d $r7,  $r5, 56\n\t"
		 "st.d $r8,  $r5, 64\n\t"
		 "st.d $r9,  $r5, 72\n\t"
		 "st.d $r10, $r5, 80\n\t"
		 "st.d $r11, $r5, 88\n\t"
		 "st.d $r12, $r5, 96\n\t"
		 "st.d $r13, $r5, 104\n\t"
		 "st.d $r14, $r5, 112\n\t"
		 "st.d $r15, $r5, 120\n\t"
		 "st.d $r16, $r5, 128\n\t"
		 "st.d $r17, $r5, 136\n\t"
		 "st.d $r18, $r5, 144\n\t"
		 "st.d $r19, $r5, 152\n\t"
		 "st.d $r20, $r5, 160\n\t"
		 "st.d $r21, $r5, 168\n\t"
		 "st.d $r22, $r5, 176\n\t"
		 "st.d $r23, $r5, 184\n\t"
		 "st.d $r24, $r5, 192\n\t"
		 "st.d $r25, $r5, 200\n\t"
		 "st.d $r26, $r5, 208\n\t"
		 "st.d $r27, $r5, 216\n\t"
		 "st.d $r28, $r5, 224\n\t"
		 "st.d $r29, $r5, 232\n\t"
		 "st.d $r30, $r5, 240\n\t"
		 "st.d $r31, $r5, 248\n\t"

		 "beqz $r6, %l[guest_entry]\n\t"
		 "pcaddi $r8, 0\n\t"
		 "st.d $r8, $r5, 256\n\t"
		 "ld.d $r8, $r5, 64\n\t" // restore $8
		 :
		 :
		 : "memory"
		 : guest_entry);

	regs->pc -= 4; // let guest jump guest_entry directly
	arch_dump_regs(STDOUT_FILENO, *regs);

	init_ebase(cpu);
	init_csr(cpu);
	// init_fpu(cpu);

	if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, regs) < 0) {
		die("KVM_SET_REGS failed");
	}

	// TODO vacate_current_stack(cpu);
	die("host never reach here\n");
guest_entry:
	// return expression is needed by guest_entry, otherwise gcc inline asm would
	// complaint
	return 0;
}

void arch_dune_enter(struct kvm_cpu *cpu)
{
	struct kvm_regs regs;
	BUILD_ASSERT(256 == offsetof(struct kvm_regs, pc));
	kvm_launch(cpu, &regs, false);
}

void switch_stack(struct kvm_cpu *cpu, u64 host_stack)
{
	// 解决 wip 的问题
	die("unimp");
}

bool do_syscall6(struct kvm_cpu *cpu, bool is_fork)
{
	die("unimp");
	return false;
}
void child_entry(struct kvm_cpu *cpu)
{
	// easy
	die("unimp");
}
void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
	die("unimp");
}
void init_child_thread_info(struct kvm_cpu *child_cpu,
			    const struct kvm_cpu *parent_cpu, int sysno)
{
	die("unimp");
}
void arch_handle_tls(struct kvm_cpu *vcpu)
{
	// easy
	die("unimp");
}
bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno)
{
	// should be zero
	die("unimp");
	return false;
}
void escape()
{
	die("unimp");
}

u64 dune_clone(u64 r4, u64 r5, u64 r6, u64 r7, u64 r8, u64 r9)
{
	die("unimp");
	// reference the glibc
	return 0;
}
