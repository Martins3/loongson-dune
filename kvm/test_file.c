#include <stdio.h>
#include <stdlib.h>  // malloc 
#include <string.h>  // strcmp ..
#include <stdbool.h> // bool false true
#include <stdlib.h>  // sort
#include <limits.h>  // INT_MAX
#include <math.h>    // sqrt
#include <unistd.h>  // sleep
#include <assert.h>  // assert
#include <fcntl.h>   // open

int main(int argc, char *argv[]){

  int fd;
  char *name = "record_syscall.txt";
  fd = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if (fd == -1) {
      perror("open failed");
      exit(1);
  }

  dprintf(fd, "\n");
  dprintf(fd, "adf");
  
  return 0;
}
