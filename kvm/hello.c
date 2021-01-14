#include <stdio.h>
#include <assert.h>

typedef unsigned long long u64;


struct kvm_regs {
	/* out (KVM_GET_REGS) / in (KVM_SET_REGS) */
	u64 gpr[32];
	u64 hi;
	u64 lo;
	u64 pc;
};

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER)	__compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)
#endif

#define BUILD_ASSERT(cond) do { (void) sizeof(char [1 - 2*!(cond)]); } while(0)

void compiler_check(){
  BUILD_ASSERT(8 * 31 == offsetof(struct kvm_regs, gpr[31]));
  BUILD_ASSERT(256 == offsetof(struct kvm_regs, hi));
  BUILD_ASSERT(264 == offsetof(struct kvm_regs, lo));
  BUILD_ASSERT(272 == offsetof(struct kvm_regs, pc));
}

// how to get current pc : https://stackoverflow.com/questions/15331033/how-to-get-current-pc-register-value-on-mips-arch
// but I don't need it

void save_kvm_regs(struct kvm_regs * p){
  asm(
  ".set noat\n\t"
  ".set noreorder\n\t"
  "sd $0, 0(%0)\n\t"
  "sd $1, 8(%0)\n\t"
  "sd $2, 16(%0)\n\t"
  "sd $3, 24(%0)\n\t"
  "sd $4, 32(%0)\n\t"
  "sd $5, 40(%0)\n\t"
  "sd $6, 48(%0)\n\t"
  "sd $7, 56(%0)\n\t"
  "sd $8, 64(%0)\n\t"
  "sd $9, 72(%0)\n\t"
  "sd $10, 80(%0)\n\t"
  "sd $11, 88(%0)\n\t"
  "sd $12, 96(%0)\n\t"
  "sd $13, 104(%0)\n\t"
  "sd $14, 112(%0)\n\t"
  "sd $15, 120(%0)\n\t"
  "sd $16, 128(%0)\n\t"
  "sd $17, 136(%0)\n\t"
  "sd $18, 144(%0)\n\t"
  "sd $19, 152(%0)\n\t"
  "sd $20, 160(%0)\n\t"
  "sd $21, 168(%0)\n\t"
  "sd $22, 176(%0)\n\t"
  "sd $23, 184(%0)\n\t"
  "sd $24, 192(%0)\n\t"
  "sd $25, 200(%0)\n\t"
  "sd $26, 208(%0)\n\t"
  "sd $27, 216(%0)\n\t"
  "sd $28, 224(%0)\n\t"
  "sd $29, 232(%0)\n\t"
  "sd $30, 240(%0)\n\t"
  "sd $31, 248(%0)\n\t"

  "mfhi $8\n\t"
  "sd $8, 256(%0)\n\t"

  "mflo $8\n\t"
  "sd $8, 264(%0)\n\t"

  "dla $8, label\n\t"
  "sd $8, 272(%0)\n\t"

  "ld $8, 64(%0)\n\t" // restore $8
  ".set at"
   :
   : "r" (p) 
   : "memory"
   );

   asm("beq $0, $0, label");

   printf("fuck\n");

   asm("label:");
}

void load_kvm_regs(struct kvm_regs * p){
  asm(
  "ld $0, 0(%0)\n\t"
   :
   : "r" (p) 
   :
   );

}

int main(int argc, char *argv[])
{
  int i;
	printf("hello\n");
  struct kvm_regs kvm_regs;
  kvm_regs.gpr[0] = 1;
  save_kvm_regs(&kvm_regs);
  for (i = 0; i < 32; ++i) {
    printf("%d : %llx\n", i, kvm_regs.gpr[i]);
  }

  printf("hi : %llx\n", kvm_regs.hi);
  printf("lo : %llx\n", kvm_regs.lo);
  printf("pc : %llx\n", kvm_regs.pc);
	return 0;
}

