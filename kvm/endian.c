#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert

int main(int argc, char *argv[]){
  long int a = 0x123456789abcdeff;

  int * x = (int *)(&a);
  printf("%x", *x);
  return 0;
}
