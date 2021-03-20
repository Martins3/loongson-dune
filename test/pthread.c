#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

static int thread_cnt;

static void setup(void)
{
	thread_cnt = 1;
}

int safe_pthread_create(pthread_t *thread_id, const pthread_attr_t *attr,
			void *(*thread_fn)(void *), void *arg)
{
	int rval;

	rval = pthread_create(thread_id, attr, thread_fn, arg);

	if (rval) {
		printf("failed\n");
		exit(1);
	}

	return rval;
}

int safe_pthread_join(pthread_t thread_id, void **retval)
{
	int rval;

	printf("just before pthread join %lx\n", thread_id);
	rval = pthread_join(thread_id, retval);

	if (rval) {
		printf("failed\n");
		exit(1);
	}
	printf("pthread succeed\n");

	return rval;
}

static void spawn_threads(pthread_t *id, void *(*thread_fn)(void *))
{
	intptr_t i;

	for (i = 0; i < thread_cnt; ++i)
		safe_pthread_create(id + i, NULL, thread_fn, (void *)i);
}

static void wait_threads(pthread_t *id)
{
	int i;

	for (i = 0; i < thread_cnt; ++i) {
		printf("pthread join never works %d\n", i);
		safe_pthread_join(id[i], NULL);
	}
}

void *thread_fn_01(void *arg)
{
	int i;

	printf("%ld stack = %p\n", (intptr_t)arg, &i);
	sleep(1);
	return NULL;
}

static void test01(void)
{
	intptr_t i;
	int k;
	pthread_t id[thread_cnt];
	int res[thread_cnt];

	spawn_threads(id, thread_fn_01);
	wait_threads(id);
	printf("all child returned\n");

	printf("OFD locks synchronized access between threads succeed\n");
}

#include "../dune/dune.h"
int main(int argc, char *argv[])
{
	// DUNE_ENTER;

	setup();
	test01();
	return 0;
}
