#include <stdio.h>


long long int asm_test(long long int x);


int main(int argc, char *argv[])
{
  long long int x = asm_test(0x123456789abcdef);
  printf("%llx", x);
  return 0;
}

/*
00000001200006b8 <asm_test>:
   1200006b8:   02ff4063        addi.d  $r3,$r3,-48(0xfd0) // sp
   1200006bc:   29c0a061        st.d    $r1,$r3,40(0x28) // return address
   1200006c0:   29c08076        st.d    $r22,$r3,32(0x20) // frame pointer
   1200006c4:   02c0c076        addi.d  $r22,$r3,48(0x30)
   1200006c8:   29ff62c4        st.d    $r4,$r22,-40(0xfd8) // 保存参数
   1200006cc:   29ffa2c0        st.d    $r0,$r22,-24(0xfe8) 
   1200006d0:   28ffa2c5        ld.d    $r5,$r22,-24(0xfe8) // 赋值
   1200006d4:   1c000004        pcaddu12i       $r4,0
   1200006d8:   02c47084        addi.d  $r4,$r4,284(0x11c)
   1200006dc:   57fe97ff        bl      -364(0xffffe94) # 120000570 <printf@plt>
   1200006e0:   03400000        andi    $r0,$r0,0x0
   1200006e4:   28c0a061        ld.d    $r1,$r3,40(0x28)
   1200006e8:   28c08076        ld.d    $r22,$r3,32(0x20)
   1200006ec:   02c0c063        addi.d  $r3,$r3,48(0x30)
   1200006f0:   4c000020        jirl    $r0,$r1,0

   目前 $r21 预留，流片回来测试性能后，根据性能情况定义为 $t9 或预留以便以后扩展ABI.
*/
