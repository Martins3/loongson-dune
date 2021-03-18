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

int pfd[2];
int dune_enter();

int host_loop_pipe(int pdf[2]);

int main(int argc, char *argv[])
{
  if (dune_enter()) {
    return 1;
  }

	// FILE *f = fopen("/home/loonson/dune/Readme.md", "r");

	printf("pfd = %p\n", pfd);

	if ((pipe(pfd)) != 0) {
		return 1;
	}

	// if ((host_loop_pipe(pfd)) != 0) {
		// return 1;
	// }

	printf("%x %x\n", pfd[0], pfd[1]);

	return 0;
}
