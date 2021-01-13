#include "minunit.h"
#include "mmu-mips.h"
#include "types.h"
#include "kvm.h"

// https://github.com/siu/minunit/blob/master/minunit_example.c

void test_setup(void) {
  printf("test setup\n"); 
}

void test_teardown(void) {
  printf("test teardown\n"); 
}

int count_bits(physaddr_t x){
  int counter = 0;
  for (int i = 0; i < 64; ++i) {
    if (x | (1 << i))
      counter ++;
  }
  return counter;
}

void mips_page_table_walker(){
  // TODO build a page table walker to
}

void test_PTE_ADDR(void){
  physaddr_t pte = UINT64(0xffffffffffffffff);
  physaddr_t addr = PTE_ADDR(pte);
  mu_check(count_bits(addr) == 33);
  physaddr_t flags = PTE_FLAGS(pte);
  mu_check(count_bits(flags) == 14);
  mu_check(count_bits(addr | flags) == 47);
}

void test_BIG_PGSIZE(void){
  mu_check(BIG_PGSIZE == (physaddr_t)(16 * 2048 * 1024));
}



MU_TEST_SUITE(test_mmu_mips) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_PTE_ADDR);
	MU_RUN_TEST(test_BIG_PGSIZE);
}

int main(int argc, char *argv[]) {
	// MU_RUN_SUITE(test_mmu_mips);
	MU_REPORT();
	return MU_EXIT_CODE;
}
