#include "../common/common.h"

#include <mpi/mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(automata_part *a, int id, int n_proc);

void apply_rules(automata_part *a, int id, int n_proc);

void sinc_parts(automata_part *a, int id, int n_proc);

void join_parts(automata_part *a, int id, int n_proc);

void calc_time(int id, int n_proc, clock_t start);

void update(automata_part *a);

void free_automata(automata_part *a);

int main(int argc, char **argv) {
  uint32_t rows, columns, steps = 100;
  clock_t start;
  int id, n_proc;

  start = clock();

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

  automata_part a;
  a.rows = rows;
  a.columns = rows;

  init(&a, id, n_proc);
  sinc_parts(&a, id, n_proc);
  update(&a);

  for (int i = 0; i < steps; i++) {
    apply_rules(&a, id, n_proc);
    sinc_parts(&a, id, n_proc);
    update(&a);
  }

  free_automata(&a);

  calc_time(id, n_proc, start);

  MPI_Finalize();

  return 0;
}

void init(automata_part *a, int id, int n_proc) {
  srand(time(NULL));

  uint32_t rows_per_proc = ceil_div(a->rows, n_proc),
           my_rows = (id == n_proc - 1 ? a->rows - (n_proc - 1) * rows_per_proc
                                       : rows_per_proc),
           neighbor_rows = 2, neighbors = (id == 0 || id == n_proc - 1 ? 1 : 2);

  a->first = (id == 0 ? 0 : a->columns * neighbor_rows);
  a->size = my_rows * a->columns;
  a->full_size = (a->size + neighbor_rows * neighbors * a->columns);
  if (id == n_proc - 2) {
    a->full_size -= (a->rows - (n_proc - 1) * rows_per_proc) * a->columns;
  }

  a->new_state = (cell *)malloc(sizeof(cell) * a->full_size);
  a->old_state = (cell *)malloc(sizeof(cell) * a->full_size);

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

    cell *new_cell = &a->new_state[row * my_rows + col];
    new_cell->temperature =
        curr_cell.temperature + curr_cell.sign / (double)sqr(24) * sum;

    if (rand() <= RAND_MAX / 10) {
      new_cell->sign = curr_cell.sign * (-1);
    }
  }
}

void sinc_parts(automata_part *a, int id, int n_proc) {
  cell *p_recv_buff = a->new_state, *p_send_buff = a->new_state + a->first,
       *n_recv_buff = p_send_buff + a->size,
       *n_send_buff = n_recv_buff - (a->full_size - a->first - a->size);
  uint32_t p_recv_size = a->first * sizeof(cell),
           p_send_size = min(a->first, a->size) * sizeof(cell),
           n_recv_size = (a->full_size - a->first - a->size) * sizeof(cell),
           n_send_size =
               (id == 0 ? a->full_size - a->size : a->first) * sizeof(cell);

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

void join_parts(automata_part *a, int id, int n_proc) {
  // TODO
  if (id == 0) {

  } else {
  }
}

void calc_time(int id, int n_proc, clock_t start) {
  // TODO
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
