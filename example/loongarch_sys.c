
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h> // sort
#include <limits.h> // INT_MAX
#include <math.h> // sqrt
#include <unistd.h> // sleep
#include <assert.h> // assert
#include <fcntl.h> // open

bool arch_do_syscall(long int syscall_parameter[8]);
int main(int argc, char *argv[])
{
	long int syscall_parameter[8];
	syscall_parameter[0] = 17; // SIGCHLD
	syscall_parameter[1] =
		(long int)malloc(1 << 14) + (1 << 14) - 100; // child_stack
	syscall_parameter[7] = 0xdc; // clone

	if(arch_do_syscall(syscall_parameter)){
    printf("child pid = %d\n", getpid());
  }else{
    printf("parent pid = %d\n", getpid());
  }

	return 0;
}

#define __SYSCALL_CLOBBERS                                                     \
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"

bool arch_do_syscall(long int syscall_parameter[8])
{
	register long int __a7 asm("$a7") = syscall_parameter[7];
	register long int __a0 asm("$a0") = syscall_parameter[0];
	register long int __a1 asm("$a1") = syscall_parameter[1];
	register long int __a2 asm("$a2") = syscall_parameter[2];
	register long int __a3 asm("$a3") = syscall_parameter[3];
	register long int __a4 asm("$a4") = syscall_parameter[4];
	register long int __a5 asm("$a5") = syscall_parameter[5];
	register long int __a6 asm("$a6") = syscall_parameter[6];

	__asm__ volatile("syscall	0\n\t"
			 : "+r"(__a0)
			 : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3),
			   "r"(__a4), "r"(__a5), "r"(__a6)
			 : __SYSCALL_CLOBBERS);
  if(__a0 == 0){
    return true;
  }

	syscall_parameter[0] = __a0;
	return false;
}
