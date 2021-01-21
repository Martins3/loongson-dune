#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
#include "syscall_arch.h"

int main(int argc, char *argv[]){
	char a [] ="fork you\n";
  __syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3);
  __syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3);
  __syscall6(5001, STDOUT_FILENO, (long)a, sizeof(a) -1, 1, 2, 3);
  return 0;
}
