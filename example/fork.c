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
#include <signal.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>

#include <sys/syscall.h>

int dune_enter();

void dune_procmap_dump();

int guest_fork(int num)
{
	pid_t child;
	while (num--) {
		double a = 12.0;

		switch (child = fork()) {
		case -1:
			printf("fork failed");
			break;
		case 0:
			a = a + 1;
			printf("child %lf\n", a);
			exit(0);
			break;
		default:
			printf("parent %lf\n", a);
			kill(child, SIGALRM);
			// sleep(1);
			// printf("-------------\n");
			break;
		}
	}
	return 0;
}

static inline int k_sigaction(int sig, struct sigaction *sa,
			      struct sigaction *osa)
{
	int ret;
	if ((ret = sigaction(sig, sa, osa)) == -1) {
		printf("set action failed\n");
		exit(1);
	}
	return ret;
}

int create_procs_flag = 0;
void set_create_procs(int sig)
{
	create_procs_flag++;
	struct timespec tstart = { 0, 0 };
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	printf("signal : %ld : %ld\n", tstart.tv_sec, tstart.tv_nsec);
	printf("child [%d] get the signal\n", getpid());
	return;
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	if (dune_enter()) {
		return 1;
	}

	sa.sa_handler = set_create_procs;
	k_sigaction(SIGALRM, &sa, NULL);

	// dune_procmap_dump();

	guest_fork(2);

	return 0;
}

#define PROCMAP_TYPE_UNKNOWN 0x00
#define PROCMAP_TYPE_FILE 0x01
#define PROCMAP_TYPE_ANONYMOUS 0x02
#define PROCMAP_TYPE_HEAP 0x03
#define PROCMAP_TYPE_STACK 0x04
#define PROCMAP_TYPE_VSYSCALL 0x05
#define PROCMAP_TYPE_VDSO 0x06
#define PROCMAP_TYPE_VVAR 0x07

struct dune_procmap_entry {
	unsigned long begin;
	unsigned long end;
	long int offset;
	bool r; // Readable
	bool w; // Writable
	bool x; // Executable
	bool p; // Private (or shared)
	char *path;
	int type;
};

typedef void (*dune_procmap_cb)(const struct dune_procmap_entry *);

extern void dune_procmap_iterate(dune_procmap_cb cb);
extern void dune_procmap_dump();

static int get_type(const char *path)
{
	if (path[0] != '[' && path[0] != '\0')
		return PROCMAP_TYPE_FILE;
	if (path[0] == '\0')
		return PROCMAP_TYPE_ANONYMOUS;
	if (strcmp(path, "[heap]") == 0)
		return PROCMAP_TYPE_HEAP;
	if (strncmp(path, "[stack", 6) == 0)
		return PROCMAP_TYPE_STACK;
	if (strcmp(path, "[vsyscall]") == 0)
		return PROCMAP_TYPE_VSYSCALL;
	if (strcmp(path, "[vdso]") == 0)
		return PROCMAP_TYPE_VDSO;
	if (strcmp(path, "[vvar]") == 0)
		return PROCMAP_TYPE_VVAR;
	return PROCMAP_TYPE_UNKNOWN;
}

void dune_procmap_iterate(dune_procmap_cb cb)
{
	struct dune_procmap_entry e;
	FILE *map;
	unsigned int dev1, dev2, inode;
	char read, write, execute, private;
	char line[512];
	char path[256];

	printf("heee\n");
	map = fopen("/proc/self/maps", "r");
	if (map == NULL) {
		printf("Could not open /proc/self/maps!\n");
		abort();
	}
	printf("after open\n");

	fread(line, sizeof(char), 10, map);

	printf("after fread\n");

	setvbuf(map, NULL, _IOFBF, 8192);
	printf("after setvbuf\n");

	while (!feof(map)) {
		path[0] = '\0';
		if (fgets(line, 512, map) == NULL)
			break;
		sscanf((char *)&line, "%lx-%lx %c%c%c%c %lx %x:%x %d %s",
		       &e.begin, &e.end, &read, &write, &execute, &private,
		       &e.offset, &dev1, &dev2, &inode, path);
		e.r = (read == 'r');
		e.w = (write == 'w');
		e.x = (execute == 'x');
		e.p = (private == 'p');
		e.path = path;
		e.type = get_type(path);
		cb(&e);
	}
	fclose(map);
}

static void __dune_procmap_dump_helper(const struct dune_procmap_entry *e)
{
	printf("0x%016lx-0x%016lx %c%c%c%c %08lx %s\n", e->begin, e->end,
	       e->r ? 'R' : '-', e->w ? 'W' : '-', e->x ? 'X' : '-',
	       e->p ? 'P' : 'S', e->offset, e->path);
}

void dune_procmap_dump()
{
	printf("--- Process Map Dump ---\n");
	dune_procmap_iterate(&__dune_procmap_dump_helper);
}
