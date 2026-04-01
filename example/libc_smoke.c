#include <stdio.h>
#include <unistd.h>

#include "../dune/dune.h"

int main(void)
{
	DUNE_ENTER;

	puts("libc puts ok");
	if (write(STDOUT_FILENO, "libc write ok\n", 14) != 14)
		return 1;

	printf("getpid=%ld\n", (long)getpid());
	return 0;
}
