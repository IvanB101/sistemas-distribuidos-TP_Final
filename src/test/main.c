#include "../common/common.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  cell *ptr = NULL;
  cell *sec = NULL;

  sec += 5;
  ptr += 10;

  printf("Cell %ld Diff %ld", ptr, ptr - sec);

  return 0;
}
