#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
struct A{
  int a;
  int b;
};

void but(int a, int b){
  printf("how");
}

void a(int a, int b){
  struct A good;
  good.a = 10;
  good.b = 1;

  while(good.a > good.b){
    good.a --;
    printf("fuck you\n");
  }

  but(a, b);
}

int main(int argc, char *argv[]){
  a(1, 2);
  return 0;
}
