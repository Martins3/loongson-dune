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

  unsigned x = 0xfff;
  printf("%x", x & (-16));
  
  return 0;
}
