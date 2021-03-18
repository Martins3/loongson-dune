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
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stack>
#include <sstream>
#include <climits>
#include <deque>
#include <set>
#include <utility>
#include <queue>
#include <map>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <string>
#include <cassert>
#include <unordered_set>
#include <unordered_map>

using namespace std;
typedef unsigned long long int u64;

void test_file()
{
	int fd;
	char name[] = "record_syscall.txt";
	fd = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}

	dprintf(fd, "\n");
	dprintf(fd, "adf");
	printf("%x\n", 0x42000028 + (0x37 << 11));
	printf("%x\n", 0x42000028 + (0x38 << 11));
	printf("%x\n", 0x42000028 + (0x39 << 11));
}

int test_guest_fork()
{
	printf("fork you\n");
	long len = printf("liyawei\n");
	printf("ret : %ld\n", len);

	pid_t pid = fork();

	switch (pid) {
	case -1:
		printf("fork failed");
		break;
	case 0:
		printf("child\n");
		exit(1);
		break;
	default:
		printf("parent\n");
		break;
	}
	return 0;
}

long syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		return -1;
	}
	return r;
}

void test_neg_16()
{
	unsigned x = 0xfff;
	printf("%x", x & (-16));
}

void test_endian()
{
	long int a = 0x123456789abcdeff;

	int *x = (int *)(&a);
	printf("%x", *x);
}

void gen_ass()
{
	for (int i = 0; i < 32; ++i) {
		printf("\"sd $%d, %d(\%%0)\\n\\t\"\n", i, i * 8);
	}
}

void gen_cp0()
{
	std::ifstream infile(
		"/home/maritns3/core/loongson-dune/3day/kvm/cp0.h");
	string line;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		string x;
		iss >> x;
		iss >> x;

		if (x.substr(0, 3) == "KVM") {
			string g = x.substr(strlen("KVM_REG_MIPS_CP0_"));
			printf("CPO_INIT_REG(%s),\n", g.c_str());
		}
	}
}

u64 show_bits(const u64 d)
{
	int i;
	for (i = 0; i < sizeof(u64) * 8; ++i) {
		if (((u64)1 << i) & d) {
			printf("[%d]", i);
		}
	}
	return d;
}

int main(int argc, char *argv[])
{
	show_bits(0x740000a2);
  printf("%x\n", 1 << 21);
  printf("%x\n", 1 << 24);
  printf("%x", 0x42000028 | (0x1 << 11));
	return 0;
}
