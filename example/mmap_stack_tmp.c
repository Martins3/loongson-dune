#define _GNU_SOURCE
#include <sys/mman.h>
#include "../dune/dune.h"
int main(){void *p; DUNE_ENTER; p = mmap(0, 1<<14, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0); return p==MAP_FAILED;}
