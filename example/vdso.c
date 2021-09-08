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

#include "../dune/dune.h"

int main(int argc, char *argv[])
{
	DUNE_ENTER;
	printf("hello\n");
	int a = getpid();
	int b;
	while ((b = getpid()) == a) {
		printf("continue\n");
	}
	printf("we failed  %d %d\n", a, b);
	return 0;
}
