/*
 * entry.c - Handles transition into dune mode
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/prctl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <err.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <sched.h>

#include "dune.h"
#include "mmu-mips.h"

#define BUILD_ASSERT(cond) do { (void) sizeof(char [1 - 2*!(cond)]); } while(0)

int dune_init(bool map_full){
	int ret, i;

	BUILD_ASSERT(IOCTL_DUNE_ENTER == DUNE_ENTER);
}
