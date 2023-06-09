#include "../common/common.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int thread_cant = omp_get_num_procs();
  omp_set_num_threads(thread_cant);

  printf("Threads: %d\n", thread_cant);

#pragma omp parallel
  {
    int id = omp_get_thread_num();
    printf("Thread: %d\n", id);
  }

  return 0;
}
