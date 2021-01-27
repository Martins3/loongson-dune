#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h> // sort
#include <limits.h> // INT_MAX
#include <math.h> // sqrt
#include <unistd.h> // sleep
#include <assert.h> // assert
#include <fcntl.h> // open

#include "syscall_arch.h"
#include "cp0.h"

void test_file()
{
	int fd;
	char *name = "record_syscall.txt";
	fd = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}

	dprintf(fd, "\n");
	dprintf(fd, "adf");
	printf("%x\n", 0x42000028 + (0x37 << 11));
	printf("%x\n", 0x42000028 + (0x38 << 11));
	printf("%x\n", 0x42000028 + (0x39 << 11));
}

int test_guest_fork()
{
	printf("fork you\n");
	long len = printf("liyawei\n");
	printf("ret : %ld\n", len);

	pid_t pid = fork();

	switch (pid) {
	case -1:
	  printf("fork failed");
		break;
	case 0:
		printf("child\n");
    exit(1);
		break;
	default:
		printf("parent\n");
		break;
	}
	return 0;
}

long syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		return -1;
	}
	return r;
}

void test_syscall(){
	char a [] ="fork you\n";

  __syscall_ret(__syscall6(5001, 100, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
}


typedef unsigned long long int u64;

void tlb_hi(u64 hi)
{
  int pgsize = 14;
  int shift = 6;

  u64 m = 0x7fffffff;
	u64 mask = ~m;
	u64 vpn = (hi & mask) >> (pgsize - shift);
	printf("%llx\n", vpn);
	printf("%llx\n", vpn | EntryLo1_1G_OFFSET);

  u64 res = ((vpn >> shift) << pgsize) | (hi & m);
  printf("%llx\n",res); 
}

int test_TLB_bit(int argc, char *argv[])
{
  printf("%x\n", PAGEMASK_1G_MASK | PAGEMASK_1G_MASK_LOW_BITS);
	tlb_hi(0xfff5f90000);
	return 0;
}

int test_neg_16(){
  unsigned x = 0xfff;
  printf("%x", x & (-16));
}

int main(int argc, char *argv[])
{
	return 0;
}
