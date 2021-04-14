#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "../dune/dune.h"


void asm_test(long long int x){
  long long int p = 0;
	asm(
	    "st.d $r4, %0, 0\n\t"
	    :
	    : "r"(&p)
	    : "memory");
  printf("%llx\n", p);
}

int main(int argc, char *argv[])
{
  // dune_enter();
  
  asm_test(0x123456789abcdef);
  
  return 0;
}

/**
00000001200006c8 <asm_test>:
   1200006c8:   02ff4063        addi.d  $r3,$r3,-48(0xfd0)
   1200006cc:   29c0a061        st.d    $r1,$r3,40(0x28)
   1200006d0:   29c08076        st.d    $r22,$r3,32(0x20)
   1200006d4:   02c0c076        addi.d  $r22,$r3,48(0x30)
   1200006d8:   29ff62c4        st.d    $r4,$r22,-40(0xfd8)
   1200006dc:   29ffa2c0        st.d    $r0,$r22,-24(0xfe8)
   1200006e0:   02ffa2cc        addi.d  $r12,$r22,-24(0xfe8)
   1200006e4:   29c00184        st.d    $r4,$r12,0
   1200006e8:   28ffa2cc        ld.d    $r12,$r22,-24(0xfe8)
   1200006ec:   00150185        move    $r5,$r12
   1200006f0:   1c000004        pcaddu12i       $r4,0
   1200006f4:   02c46084        addi.d  $r4,$r4,280(0x118)
   1200006f8:   57fe8bff        bl      -376(0xffffe88) # 120000580 <printf@plt>
   1200006fc:   03400000        andi    $r0,$r0,0x0
   120000700:   28c0a061        ld.d    $r1,$r3,40(0x28)
   120000704:   28c08076        ld.d    $r22,$r3,32(0x20)
   120000708:   02c0c063        addi.d  $r3,$r3,48(0x30)
   12000070c:   4c000020        jirl    $r0,$r1,0
*/
