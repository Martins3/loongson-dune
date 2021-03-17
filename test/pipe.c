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

int main(int argc, char *argv[])
{
	if (dune_enter()) {
		return 1;
	}

	if ((pipe(pfd)) == -1) {
		return 1;
	}

	printf("%x %x\n", pfd[0], pfd[1]);

	return 0;
}
