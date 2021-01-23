#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
long syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		return -1;
	}
	return r;
}

int main(int argc, char *argv[]){
  
  long r = -10000;
  printf("%d", syscall_ret(r));
  return 0;
}
