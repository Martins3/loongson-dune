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
#include <stdio.h>
#include <signal.h>

#include "../dune/dune.h"

void handle_sigint(int sig)
{
  __asm__(".word 0x42000828");
	printf("Caught signal %d\n", sig);
}

int main(int argc, char *argv[])
{
  DUNE_ENTER;

	signal(SIGINT, handle_sigint);

	while (true) {
		sleep(4);
		printf("sleep\n");
	}

	return 0;
}
