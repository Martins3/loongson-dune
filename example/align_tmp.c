#include <stdint.h>
#include "../dune/dune.h"
int main(){ uintptr_t sp; DUNE_ENTER; __asm__("mov %%rsp,%0":"=r"(sp)); return (int)(sp & 0xf); }
