#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
#include <fcntl.h>   // open


int dune_enter();
int main(int argc, char *argv[]){
  if(dune_enter()){
    return 1;
  }
  printf("hello\n");
  return 0;
}
