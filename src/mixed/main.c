#include "../common/common.h"

#include <mpi/mpi.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Los estados solamente pueden ser mostrados para matrices con lados multiplo
// de 4
#define SHOW_INICIAL 0
#define SHOW_FINAL 0
#define SHOW_STEPS 0
#define STEP 1
#define SIGN_FLIP 1
#define GROWTH_FACTOR 50

void master(automata_part *a, int id, int n_proc, int steps);

void slave(automata_part *a, int id, int n_proc, int steps);

void init(automata_part *a, int id, int n_proc);

void apply_rules(automata_part *a, int id, int n_proc);

void sinc_parts(automata_part *a, int id, int n_proc);

void update(automata_part *a);

void free_automata(automata_part *a);

int main(int argc, char **argv) {
  uint32_t rows, columns, steps = 100;
  int id, n_proc;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &n_proc);

  if (argc != 3 || !(rows = atoi(argv[1])) || !(steps = atoi(argv[2]))) {
    if (id == 0)
      print_usage();
    else
      // Blocks all proccesses but 0 to print_usage just once
      MPI_Recv(NULL, 0, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  omp_set_num_threads(omp_get_num_procs());

  automata_part a;
  a.rows = rows;
  a.columns = rows;

  init(&a, id, n_proc);
  sinc_parts(&a, id, n_proc);
  update(&a);

  if (id == 0) {
    master(&a, id, n_proc, steps);
  } else {
    slave(&a, id, n_proc, steps);
  }

  free_automata(&a);

  MPI_Finalize();

  return 0;
}

void master(automata_part *a, int id, int n_proc, int steps) {
  uint64_t start_t, end_t;
  clock_t start, end;
  int count = a->size * sizeof(cell);

  start_t = time(NULL);
  start = clock();

  automata full = {
      .rows = a->rows,
      .columns = a->rows,
      .old_state = malloc(a->rows * a->columns * sizeof(cell)),
      .new_state = NULL,
  };

#if SHOW_INICIAL
  MPI_Gather(a->old_state + a->first, count, MPI_BYTE, full.old_state, count,
             MPI_BYTE, 0, MPI_COMM_WORLD);
  print_state(full);
#endif

#pragma omp parallel
  for (int i = 0; i < steps; i++) {
    apply_rules(a, id, n_proc);
#pragma omp barrier
#pragma omp single
    {
      sinc_parts(a, id, n_proc);
      update(a);
#if SHOW_STEPS
      if (!(i % STEP)) {
        MPI_Gather(&a->old_state[a->first], count, MPI_BYTE, &full.old_state[0],
                   count, MPI_BYTE, 0, MPI_COMM_WORLD);
        print_state(full);
      }
#endif
    }
  }
#if SHOW_FINAL
  MPI_Gather(a->old_state + a->first, count, MPI_BYTE, full.old_state, count,
             MPI_BYTE, 0, MPI_COMM_WORLD);
  print_state(full);
#endif
  free(full.old_state);

  end = clock();
  end_t = time(NULL);
  printf("Tiempo de CPU: %lf\n", ((double)end - start) / CLOCKS_PER_SEC);
  printf("Tiempo de ejecucion: %ld\n", end_t - start_t);
}

void slave(automata_part *a, int id, int n_proc, int steps) {
#if SHOW_INICIAL
  MPI_Gather(a->old_state + a->first, a->size * sizeof(cell), MPI_BYTE, NULL, 0,
             MPI_BYTE, 0, MPI_COMM_WORLD);
#endif

#pragma omp parallel
  for (int i = 0; i < steps; i++) {
    apply_rules(a, id, n_proc);
#pragma omp barrier
#pragma omp single
    {
      sinc_parts(a, id, n_proc);
      update(a);
#if SHOW_STEPS
      if (!(i % STEP)) {
        MPI_Gather(&a->old_state[a->first], a->size * sizeof(cell), MPI_BYTE,
                   NULL, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
      }
#endif
    }
  }

#if SHOW_FINAL
  MPI_Gather(a->old_state + a->first, a->size * sizeof(cell), MPI_BYTE, NULL, 0,
             MPI_BYTE, 0, MPI_COMM_WORLD);
#endif
}

void init(automata_part *a, int id, int n_proc) {
  srand(time(NULL));

  uint32_t rows_per_proc = ceil_div(a->rows, n_proc), my_rows = rows_per_proc,
           neighbor_rows = 2, neighbors = 2;

  a->first = a->columns * neighbor_rows;

  uint32_t rem_rows = a->rows - (rows_per_proc * (n_proc - 1));

  if (id == 0) {
    neighbors = 1;
    a->first = 0;
  }
  if (id == n_proc - 1) {
    neighbors = 1;
    my_rows = rem_rows;
  }

  a->size = my_rows * a->columns;

  if (id == n_proc - 2) {
    a->full_size =
        (a->size + (neighbor_rows + min(neighbor_rows, rem_rows)) * a->columns);
  } else {
    a->full_size = (a->size + neighbor_rows * neighbors * a->columns);
  }

  a->new_state = (cell *)malloc(sizeof(cell) * a->full_size);
  a->old_state = (cell *)malloc(sizeof(cell) * a->full_size);

#pragma omp parallel for
  for (uint64_t c = a->first; c < a->first + a->size; c++) {
    double temp = rand() / (double)RAND_MAX * 272 * 2 - 272;
    double conduc = rand() / 2.0 / RAND_MAX;
    int8_t sign = (rand() > RAND_MAX / 2) ? 1 : -1;

    a->new_state[c].temperature = temp;
    a->new_state[c].conductivity = conduc;
    a->new_state[c].sign = sign;

    a->old_state[c].conductivity = conduc;
    a->old_state[c].sign = sign;
  }
}

void apply_rules(automata_part *a, int id, int n_proc) {
  uint32_t my_rows = a->full_size / a->columns;

#pragma omp for
  for (int i = a->first; i < a->first + a->size; i++) {
    double sum = 0;
    uint32_t row = i / a->columns, col = i % a->columns;
    cell curr_cell = a->old_state[i];

    int from_row = (row > 1 ? -2 : -row),
        to_row = (row < my_rows - 2 ? 2 : my_rows - row - 1),
        from_col = (col > 1 ? -2 : -col),
        to_col = (col < a->columns - 2 ? 2 : a->columns - col - 1);

    for (int dist_row = from_row; dist_row <= to_row; dist_row++) {
      uint32_t disp = (row + dist_row) * my_rows;
      double partial_sum = 0;

      for (int dist_col = from_col; dist_col <= to_col; dist_col++) {
        uint32_t vec_index = disp + col + dist_col;
        if (vec_index == i) {
          continue;
        }

        cell neighbor = a->old_state[vec_index];
        uint32_t dist = max(abs(dist_col), abs(dist_row));

        partial_sum += neighbor.temperature *
                       (curr_cell.conductivity + neighbor.conductivity) /
                       sqr(dist + 2);
      }

      sum += partial_sum;
    }

    cell *new_cell = &a->new_state[i];
    new_cell->temperature = curr_cell.temperature + curr_cell.sign /
                                                        (double)sqr(24) * sum *
                                                        GROWTH_FACTOR;
    restric_temp(&new_cell->temperature);

#if SIGN_FLIP
    if (rand() <= RAND_MAX / 10) {
      new_cell->sign = curr_cell.sign * (-1);
    }
#endif
  }
}

void sinc_parts(automata_part *a, int id, int n_proc) {
  cell *p_recv_buff = a->new_state, *p_send_buff = a->new_state + a->first,
       *n_recv_buff = p_send_buff + a->size,
       *n_send_buff = n_recv_buff - (a->full_size - a->first - a->size);
  uint64_t p_recv_size = a->first * sizeof(cell),
           p_send_size = min(a->first, a->size) * sizeof(cell),
           n_recv_size = (a->full_size - a->first - a->size) * sizeof(cell),
           n_send_size = a->first * sizeof(cell);

  if (id == 0) {
    p_recv_buff = NULL;
    n_send_size = (a->full_size - a->size) * sizeof(cell);
  }
  if (id == n_proc - 1) {
    n_send_buff = NULL;
    n_send_size = 0;
  }

  /*
  printf("thread: %d\tfull size: %d\tfirst: %d\tsize: %d\t", id, a->full_size,
         a->first, a->size);

  printf("pr: %ld\tps: %ld\tnr: %ld\tns: %ld\n", p_recv_size, p_send_size,
         n_recv_size, n_send_size);
  */

  if (id == 0) {
    MPI_Send(n_send_buff, n_send_size, MPI_BYTE, id + 1, 0, MPI_COMM_WORLD);
    MPI_Recv(n_recv_buff, n_recv_size, MPI_BYTE, id + 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  } else if (id == n_proc - 1) {
    MPI_Recv(p_recv_buff, p_recv_size, MPI_BYTE, id - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Send(p_send_buff, p_send_size, MPI_BYTE, id - 1, 0, MPI_COMM_WORLD);
  } else {
    MPI_Recv(p_recv_buff, p_recv_size, MPI_BYTE, id - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Send(p_send_buff, p_send_size, MPI_BYTE, id - 1, 0, MPI_COMM_WORLD);
    MPI_Send(n_send_buff, n_send_size, MPI_BYTE, id + 1, 0, MPI_COMM_WORLD);
    MPI_Recv(n_recv_buff, n_recv_size, MPI_BYTE, id + 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  }
}

void update(automata_part *a) {
  cell *temp = a->old_state;
  a->old_state = a->new_state;
  a->new_state = temp;
}

void free_automata(automata_part *a) {
  free((void *)a->new_state);
  free((void *)a->old_state);
}
