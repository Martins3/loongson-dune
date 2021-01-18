#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h> // sort
#include <limits.h> // INT_MAX
#include <math.h> // sqrt
#include <unistd.h> // sleep
#include <assert.h> // assert
#include "cp0.h"


typedef unsigned long long int u64;

void get(u64 hi)
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

int main(int argc, char *argv[])
{
  printf("%x\n", PAGEMASK_1G_MASK | PAGEMASK_1G_MASK_LOW_BITS);
	get(0xfff5f90000);
	return 0;
}
