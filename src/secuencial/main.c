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
  uint32_t rows = 1000, columns = 1000, steps = 100;
  clock_t start, end;
  double exec_time;

  start = clock();

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

  // TODO almacenar resultado

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
    double conduc = rand() / 2.0 * RAND_MAX;
    int8_t sign = (rand() > RAND_MAX / 2) ? 1 : -1;

    a->new_state[c].temperature = temp;
    a->new_state[c].conductivity = conduc;
    a->new_state[c].sign = sign;

    a->old_state[c].temperature = temp;
    a->old_state[c].conductivity = conduc;
    a->old_state[c].sign = sign;
  }
}

void apply_rules(automata *a) {
  for (uint32_t row = 0; row < a->rows; row++) {
    for (uint32_t col = 0; col < a->columns; col++) {
      double sum = 0;
      cell curr_cell = a->old_state[row * a->rows + col];

      for (int dist_row = -2; dist_row <= 2; dist_row++) {
        uint32_t vec_row = row + dist_row;
        if (vec_row >= a->rows) {
          continue;
        }
        double partial_sum = 0;

        for (int dist_col = -2; dist_col <= 2; dist_col++) {
          uint32_t vec_col = col + dist_col;
          if (vec_col >= a->columns) {
            continue;
          }
          cell neighbor = a->old_state[vec_row * row + vec_col];
          uint32_t dist = max(abs(dist_col), abs(dist_row));

          partial_sum += neighbor.temperature *
                         (curr_cell.conductivity + neighbor.conductivity) /
                         sqr(dist + 2);
        }

        sum += partial_sum;
      }

      cell new_cell = a->new_state[row * a->rows + col];
      new_cell.temperature += curr_cell.sign / (double)sqr(24) * sum;

      if (rand() < RAND_MAX / 10) {
        new_cell.sign = curr_cell.sign * (-1);
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
