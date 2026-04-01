#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include "../dune/dune.h"
int main(){void *p; DUNE_ENTER; p = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); return p==MAP_FAILED;}
