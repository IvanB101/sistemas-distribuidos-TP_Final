#include "../common/common.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init(automata *a);

void apply_rules(automata *a);

void update(automata *a);

void free_automata(automata *a);

int main(int argc, char **argv) {
  uint32_t rows, columns, steps = 100;
  clock_t start, end;
  double exec_time;

  start = clock();

  if (argc != 3 || !(rows = atoi(argv[1])) || !(steps = atoi(argv[2]))) {
    print_usage();
  }
  columns = rows;

  automata a = {
      .rows = rows,
      .columns = columns,
      .old_state = NULL,
      .new_state = NULL,
  };

  init(&a);

  for (int i = 0; i < steps; i++) {
    apply_rules(&a);
    update(&a);
  }

  free_automata(&a);

  end = clock();
  exec_time = ((double)end - start) / CLOCKS_PER_SEC;
  printf("Tiempo de ejecucion: %lf\n", exec_time);

  return 0;
}

void init(automata *a) {
  srand(time(NULL));

  a->new_state = (cell *)malloc(sizeof(cell) * a->rows * a->columns);
  a->old_state = (cell *)malloc(sizeof(cell) * a->rows * a->columns);

  for (uint64_t c = 0; c < a->rows * a->columns; c++) {
    double temp = rand() / (double)RAND_MAX * 272 * 2 - 272;
    double conduc = rand() / 2.0 / RAND_MAX;
    int8_t sign = (rand() > RAND_MAX / 2) ? 1 : -1;

    a->old_state[c].temperature = temp;
    a->old_state[c].conductivity = conduc;
    a->old_state[c].sign = sign;

    a->new_state[c].conductivity = conduc;
    a->new_state[c].sign = sign;
  }
}

void apply_rules(automata *a) {
  for (int row = 0; row < a->rows; row++) {
    for (int col = 0; col < a->columns; col++) {
      double sum = 0;
      uint32_t index = row * a->rows + col;
      cell curr_cell = a->old_state[index];

      int from_row = (row > 1 ? -2 : -row),
          to_row = (row < a->rows - 2 ? 2 : a->rows - row - 1),
          from_col = (col > 1 ? -2 : -col),
          to_col = (col < a->columns - 2 ? 2 : a->columns - col - 1);

      for (int dist_row = from_row; dist_row <= to_row; dist_row++) {
        uint32_t disp = (row + dist_row) * a->rows;
        double partial_sum = 0;

        for (int dist_col = from_col; dist_col <= to_col; dist_col++) {
          uint32_t vec_index = disp + col + dist_col;
          if (vec_index == index) {
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

      cell *new_cell = &a->new_state[row * a->rows + col];
      new_cell->temperature =
          curr_cell.temperature + curr_cell.sign / (double)sqr(24) * sum;

      if (rand() <= RAND_MAX / 10) {
        new_cell->sign = curr_cell.sign * (-1);
      }
    }
  }
}

void update(automata *a) {
  cell *temp = a->old_state;
  a->old_state = a->new_state;
  a->new_state = temp;
}

void free_automata(automata *a) {
  free((void *)a->new_state);
  free((void *)a->old_state);
}
