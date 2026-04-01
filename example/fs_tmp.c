#include <stdint.h>
#include "../dune/dune.h"
int main(){ uintptr_t x; DUNE_ENTER; __asm__("mov %%fs:0x28,%0":"=r"(x)); return x == 0; }
