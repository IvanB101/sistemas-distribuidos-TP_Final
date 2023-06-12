#include "../common/common.h"

#include <math.h>
#include <mpi/mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  cell *old_state;
  cell *new_state;
  uint32_t rows;
  uint32_t columns;
  int8_t neighbors[8];
  cell *buffers[8];
} automata_part;

void init(automata_part *a, int id, int n_proc, uint32_t total_rows);

void apply_rules(automata_part *a, int id, int n_proc);

void sinc(automata_part *a, int id, int n_proc);

void update(automata_part *a, int id, int n_proc);

void free_automata(automata_part *a);

int main(int argc, char **argv) {
  uint32_t rows, columns, steps = 100;
  clock_t start, end;
  double exec_time;
  int id, n_proc;

  start = clock();

  if (argc != 3 || !(rows = atoi(argv[1])) || !(steps = atoi(argv[2]))) {
    print_usage();
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &n_proc);

  automata_part a;

  init(&a, id, n_proc, rows);
  sinc(&a, id, n_proc);
  update(&a, id, n_proc);

  for (int i = 0; i < steps; i++) {
    apply_rules(&a, id, n_proc);
    sinc(&a, id, n_proc);
    update(&a, id, n_proc);
  }

  free_automata(&a);

  if (id == 0) {
    end = clock();
    exec_time = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecucion: %lf\n", exec_time);
  } else {
  }

  MPI_Finalize();

  return 0;
}

void init(automata_part *a, int id, int n_proc, uint32_t total_rows) {
  srand(time(NULL));

  for (int i = 0; i < 8; i++) {
    a->neighbors[i] = -1;
    a->buffers[i] = NULL;
  }
  switch (id) {
  case 0:
    a->neighbors[3] = 1;
    a->neighbors[4] = 3;
    a->neighbors[5] = 2;
    break;
  case 1:
    a->neighbors[5] = 3;
    a->neighbors[6] = 2;
    a->neighbors[7] = 0;
    break;
  case 2:
    a->neighbors[1] = 0;
    a->neighbors[2] = 1;
    a->neighbors[3] = 3;
    break;
  case 3:
    a->neighbors[0] = 0;
    a->neighbors[1] = 1;
    a->neighbors[7] = 2;
  }

  uint32_t rows = total_rows / 2, cols = rows;

  // Rows and columns of the matris that the process is going to calculate
  a->rows = rows;
  a->columns = rows;

  // Rows and columns of the matris that the process need in order to calculate
  if (a->neighbors[1] != -1) {
    cols += 2;
  }
  if (a->neighbors[3] != -1) {
    rows += 2;
  }
  if (a->neighbors[5] != -1) {
    cols += 2;
  }
  if (a->neighbors[7] != -1) {
    rows += 2;
  }

  switch (id) {
  case 0:
    a->buffers[3] = (cell *)malloc(sizeof(cell) * rows * 2);
    a->buffers[4] = (cell *)malloc(sizeof(cell) * 2 * 2);
    a->buffers[5] = (cell *)malloc(sizeof(cell) * cols * 2);
    break;
  case 1:
    a->buffers[5] = (cell *)malloc(sizeof(cell) * cols * 2);
    a->buffers[6] = (cell *)malloc(sizeof(cell) * 2 * 2);
    a->buffers[7] = (cell *)malloc(sizeof(cell) * rows * 2);
    break;
  case 2:
    a->buffers[5] = (cell *)malloc(sizeof(cell) * cols * 2);
    a->buffers[6] = (cell *)malloc(sizeof(cell) * 2 * 2);
    a->buffers[7] = (cell *)malloc(sizeof(cell) * rows * 2);
    a->neighbors[1] = 0;
    a->neighbors[2] = 1;
    a->neighbors[3] = 3;
    break;
  case 3:
    a->buffers[5] = (cell *)malloc(sizeof(cell) * cols * 2);
    a->buffers[6] = (cell *)malloc(sizeof(cell) * 2 * 2);
    a->buffers[7] = (cell *)malloc(sizeof(cell) * rows * 2);
    a->neighbors[0] = 0;
    a->neighbors[1] = 1;
    a->neighbors[7] = 2;
  }

  uint32_t cells = cols * rows;

  a->new_state = (cell *)malloc(sizeof(cell) * cells);
  a->old_state = (cell *)malloc(sizeof(cell) * cells);

  for (uint64_t c = 0; c < cells; c++) {
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
  int from_row = 0, to_row = a->rows, from_col = 0, to_col = a->columns;
  if (a->neighbors[7] != -1) {
    from_col++;
    to_col++;
  }
  if (a->neighbors[1] != -1) {
    from_row++;
    to_row++;
  }

  for (int row = 0; row < a->rows; row++) {
    for (int col = 0; col < a->columns; col++) {
      double sum = 0;
      uint32_t index = row * a->rows + col;
      cell curr_cell = a->old_state[index];

      int up_off = (row > 1 ? -2 : -row),
          down_off = (a->neighbors[5] != -1
                          ? 2
                          : (row < to_row - 2 ? 2 : to_row - row - 1)),
          left_off = (col > 1 ? -2 : -col),
          right_off = (a->neighbors[3] != -1
                           ? 2
                           : (col < to_col - 2 ? 2 : to_col - col - 1));

      for (int hor_off = up_off; hor_off <= down_off; hor_off++) {
        uint32_t disp = (row + hor_off) * a->rows;
        double partial_sum = 0;

        for (int ver_off = left_off; ver_off <= right_off; ver_off++) {
          uint32_t vec_index = disp + col + ver_off;
          if (vec_index == index) {
            continue;
          }

          cell neighbor = a->old_state[vec_index];
          uint32_t dist = max(abs(ver_off), abs(hor_off));

          partial_sum += neighbor.temperature *
                         (curr_cell.conductivity + neighbor.conductivity) /
                         sqr(dist + 2);
        }

        sum += partial_sum;
      }

      cell *new_cell = &a->new_state[row * a->rows + col];
      new_cell->temperature =
          curr_cell.temperature + curr_cell.sign / (double)sqr(24) * sum;

      if (rand() <= RAND_MAX / 10) {
        new_cell->sign = curr_cell.sign * (-1);
      }
    }
  }
}

void sinc(automata_part *a, int id, int n_proc) {
  for (int i = 0; i < 8; i++) {
    if (a->neighbors[i] != -1) {
      // TODO Enviar celdas
    }
  }
  for (int i = 0; i < 8; i++) {
    if (a->neighbors[i] != -1) {
      // TODO Recibir celdas
    }
  }
}

void update(automata_part *a, int id, int n_proc) {
  cell *temp = a->old_state;
  a->old_state = a->new_state;
  a->new_state = temp;
}

void free_automata(automata_part *a) {
  free((void *)a->new_state);
  free((void *)a->old_state);
}
