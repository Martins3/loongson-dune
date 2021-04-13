#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "../dune/dune.h"

int main(int argc, char *argv[])
{
  dune_enter();
  
  return 0;
}
