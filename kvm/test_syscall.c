#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
#include <errno.h>
#include "syscall_arch.h"

long __syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}

int main(int argc, char *argv[]){
	char a [] ="fork you\n";

  __syscall_ret(__syscall6(5001, 100, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
  __syscall_ret(__syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3));
  return 0;
}
