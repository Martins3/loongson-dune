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

#include <sys/time.h>
#include <sys/resource.h>

typedef unsigned long long u64;

extern void dune_procmap_dump();

#define die(x) printf("%s", x);

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

void get_writesize()
{
	printf("get_writesize\n");
	char read[4096];
	printf("read : %p\n", read);

	char buf[0x3d000];
	printf("buf location %p", buf);
}

static u64 expand_stack_getrlimit()
{
	struct rlimit rlim;
	if (getrlimit(RLIMIT_STACK, &rlim) == -1)
		die("get rlimit failed");
	return rlim.rlim_cur;
}

static u64 expand_stack_get_stack_top()
{
	struct dune_procmap_entry e;

	char line[512];
	char path[256];
	unsigned int dev1, dev2, inode;
	char read, write, execute, private;

	FILE *map = fopen("/proc/self/maps", "r");
	if (map == NULL)
		die("Could not open /proc/self/maps!\n");

	if (setvbuf(map, NULL, _IOFBF, 8192))
		die("setvbuf");

	while (!feof(map)) {
		path[0] = '\0';
		if (fgets(line, 512, map) == NULL)
			break;
		sscanf((char *)&line, "%lx-%lx %c%c%c%c %lx %x:%x %d %s",
		       &e.begin, &e.end, &read, &write, &execute, &private,
		       &e.offset, &dev1, &dev2, &inode, path);
		if (strncmp(path, "[stack", 6) == 0) {
			return e.end;
		}
	}

	if (fclose(map))
		die("close file");

	die("stack entry not found in /proc/self/maps");
  return -1;
}


#include "../dune/dune.h"
#ifndef DUNE_ENTER
static void expand_stack()
{
	u64 limit = expand_stack_getrlimit();
	u64 top = expand_stack_get_stack_top();
	printf("top = %llx, limit = %llx\n", top, limit);
	*(int *)(top - limit) = 0;
}
#else
void expand_stack();
#endif

int main(int argc, char *argv[])
{
	dune_procmap_dump();
  DUNE_ENTER;
  expand_stack();

	dune_procmap_dump();

	get_writesize();

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

	map = fopen("/proc/self/maps", "r");
	if (map == NULL) {
		printf("Could not open /proc/self/maps!\n");
		abort();
	}

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
