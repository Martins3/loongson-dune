#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert

#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
long syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		return -1;
	}
	return r;
}

int test_guest_fork()
{
	printf("fork you\n");
	long len = printf("liyawei\n");
	printf("ret : %ld\n", len);

	pid_t pid = fork();

	switch (pid) {
	case -1:
	  printf("fork failed");
		break;
	case 0:
		printf("child\n");
    exit(1);
		break;
	default:
		printf("parent\n");
		break;
	}
	return 0;
}

int main(int argc, char *argv[]){
  test_guest_fork();

  return 0;
}

