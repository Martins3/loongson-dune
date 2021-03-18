#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
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

#include <sched.h>

static void setup(void);
static void cleanup(void);
static int do_child();

/*
 * ltp_clone: wrapper for clone to hide the architecture dependencies.
 *   1. hppa takes bottom of stack and no stacksize (stack grows up)
 *   2. __ia64__ takes bottom of stack and uses clone2
 *   3. all others take top of stack (stack grows down)
 */
static int ltp_clone_(unsigned long flags, int (*fn)(void *arg), void *arg,
		      size_t stack_size, void *stack, pid_t *ptid, void *tls,
		      pid_t *ctid)
{
	int ret;

#if defined(__ia64__)
	ret = __clone2(fn, stack, stack_size, flags, arg, ptid, tls, ctid);
#else
#if defined(__hppa__) || defined(__metag__)
	/*
	 * These arches grow their stack up, so don't need to adjust the base.
	 * XXX: This should be made into a runtime test.
	 */
#else
	/*
	 * For archs where stack grows downwards, stack points to the topmost
	 * address of the memory space set up for the child stack.
	 */
	if (stack)
		stack += stack_size;
#endif

	ret = clone(fn, stack, flags, arg, ptid, tls, ctid);
#endif

	return ret;
}

int ltp_clone(unsigned long flags, int (*fn)(void *arg), void *arg,
	      size_t stack_size, void *stack)
{
	return ltp_clone_(flags, fn, arg, stack_size, stack, NULL, NULL, NULL);
}

int dune_enter();

int main(int ac, char **av)
{
	if (dune_enter()) {
		return 1;
	}
	void *child_stack;
	int status, child_pid;

#define CHILD_STACK_SIZE (1 << 14)

	child_stack = malloc(CHILD_STACK_SIZE);
	if (child_stack == NULL) {
		return 1;
	}
  printf("child_stack is %p\n", child_stack);

	ltp_clone(SIGCHLD, do_child, NULL, CHILD_STACK_SIZE, child_stack);

	printf("parent\n");
	exit(0);
}

static int do_child(void)
{
	printf("child\n");
	exit(0);
}
