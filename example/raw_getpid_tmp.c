#include "../dune/dune.h"
static long raw_getpid(void){ register long rax asm("rax") = 39; asm volatile("syscall" : "+a"(rax) : : "rcx", "r11", "memory"); return rax; }
int main(){ DUNE_ENTER; return raw_getpid() <= 0; }
