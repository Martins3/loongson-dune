#include <stdio.h>
typedef unsigned long long int u64;

u64 bits(const u64 d)
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
	u64 x = 0x40008010;
	bits(x);
	return 0;
}
