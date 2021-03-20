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
static const int max_thread_cnt = 32;
static const char fname[] = "tst_ofd_locks.txt";
const int writes_num = 100;
const int write_size = 4096;

static void setup(void)
{
	thread_cnt = 12;
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

	rval = pthread_join(thread_id, retval);

	if (rval) {
		printf("failed\n");
		exit(1);
	}

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

	for (i = 0; i < thread_cnt; ++i)
		safe_pthread_join(id[i], NULL);
}

void *thread_fn_01(void *arg)
{
	int i;
	unsigned char buf[write_size];
	int fd = open(fname, O_RDWR);

	printf("%ld stack = %p\n", (intptr_t)arg, buf);
	memset(buf, (intptr_t)arg, write_size);
	printf("after memset %ld stack = %p\n", (intptr_t)arg, buf);

	struct flock lck = {
		.l_whence = SEEK_SET,
		.l_start = 0,
		.l_len = 1,
	};

	for (i = 0; i < writes_num; ++i) {
		lck.l_type = F_WRLCK;
		fcntl(fd, F_OFD_SETLKW, &lck);

		lseek(fd, 0, SEEK_END);
		write(fd, buf, write_size);

		lck.l_type = F_UNLCK;
		fcntl(fd, F_OFD_SETLKW, &lck);

		sched_yield();
	}

	close(fd);

	return NULL;
}

static void test01(void)
{
	intptr_t i;
	int k;
	pthread_t id[thread_cnt];
	int res[thread_cnt];
	unsigned char buf[write_size];

	int fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, 0600);

	memset(res, 0, sizeof(res));

	spawn_threads(id, thread_fn_01);
	wait_threads(id);
	printf("all child returned\n");

	lseek(fd, 0, SEEK_SET);
	for (i = 0; i < writes_num * thread_cnt; ++i) {
		read(fd, buf, write_size);

		if (buf[0] >= thread_cnt) {
			printf("unexpected data read");
			exit(1);
			return;
		}

		++res[buf[0]];

		for (k = 1; k < write_size; ++k) {
			if (buf[0] != buf[k]) {
				printf("failed unexpected data read");
				exit(1);
				return;
			}
		}
	}

	for (i = 0; i < thread_cnt; ++i) {
		if (res[i] != writes_num) {
			printf("fail : corrupted data found");
			exit(1);
			return;
		}
	}
	close(fd);

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
