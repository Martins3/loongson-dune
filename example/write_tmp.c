#include <unistd.h>
#include "../dune/dune.h"
int main(){ DUNE_ENTER; return write(1, "x\n", 2) != 2; }
