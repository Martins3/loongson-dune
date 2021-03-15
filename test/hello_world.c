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

int dune_enter();

#include <mqueue.h>
int main(int argc, char *argv[])
{
	if (dune_enter()) {
		return 1;
	}

	int flags, opt;
	mqd_t mqd;
	unsigned int prio;

	flags = O_WRONLY;
	while ((opt = getopt(argc, argv, "n")) != -1) {
		switch (opt) {
		case 'n':
			flags |= O_NONBLOCK;
			break;
		default:
			printf("check tlpi page 1075\n");
		}
	}

	if (optind + 1 >= argc) {
		printf("check tlpi page 1075\n");
	}

	mqd = mq_open(argv[optind], flags);
	if (mqd == (mqd_t)-1) {
		printf("mq_open\n");
		exit(1);
	}

	prio = (argc > optind + 2) ? atoi(argv[optind + 2]) : 0;

	if (mq_send(mqd, argv[optind + 1], strlen(argv[optind + 1]), prio) ==
	    -1) {
		printf("mq_send\n");
		exit(1);
	}
	exit(EXIT_SUCCESS);
	return 0;
}
