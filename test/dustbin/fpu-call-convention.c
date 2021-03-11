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

void fuck(int a, int b)
{
	a = a + 100;
	b = b + 101;
}

void fuck_float(double a, double b)
{
	a = a + 100.0;
	b = b + 101.0;
}

void call_fuck_float()
{
	fuck_float(11.0, 22.0);
}

int main(int argc, char *argv[])
{
	double a = 12;
	a = a + 1;
	printf("%lf", a);

	return 0;
}
