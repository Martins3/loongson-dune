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
  char a [] = "fuckasdfasdfafa\n";
  long ret = __syscall6(5001,  STDOUT_FILENO, (long)a, 12, 0, 0, 0);
  printf("\n\n%ld", ret);
  return 0;
}
