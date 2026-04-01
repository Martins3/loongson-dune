#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../dune/dune.h"

int main(void)
{
	pid_t pid;
	int status = 0;

	DUNE_ENTER;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	if (pid == 0) {
		printf("child pid=%ld ppid=%ld\n", (long)getpid(),
		       (long)getppid());
		fflush(stdout);
		_exit(0);
	}

	if (waitpid(pid, &status, 0) != pid) {
		perror("waitpid");
		return 2;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return 3;

	printf("parent waited pid=%ld\n", (long)pid);
	return 0;
}
