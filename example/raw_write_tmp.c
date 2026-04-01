#include "../dune/dune.h"
static long raw_write(int fd, const void *buf, unsigned long len){ register long rax asm("rax") = 1; register long rdi asm("rdi") = fd; register long rsi asm("rsi") = (long)buf; register long rdx asm("rdx") = len; asm volatile("syscall" : "+a"(rax) : "D"(rdi), "S"(rsi), "d"(rdx) : "rcx", "r11", "memory"); return rax; }
int main(){ DUNE_ENTER; return raw_write(1, "y\n", 2) != 2; }
