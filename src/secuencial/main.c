#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct {
  double temperature;
  double conductivity;
  uint8_t sign;
} cell;

typedef struct {
  uint32_t rows;
  uint32_t columns;
  cell *old_state;
  cell *new_state;
} automata;

void init(automata *a);

void apply(automata *a);

void update(automata *a);

int main(int argc, char **argv) {
  uint32_t rows = 100, columns = 100, steps = 100;

  automata a = {
      .rows = rows,
      .columns = columns,
      .old_state = NULL,
      .new_state = NULL,
  };

  init(&a);

  for (int i = 0; i < steps; i++) {
    apply(&a);
    update(&a);
  }

  // TODO almacenar resultado

  return 0;
}

void init(automata *a) {
  // TODO inicializacion de matriz
}

void apply(automata *a) {
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
                         pow(dist + 2, 2);
        }

        sum += partial_sum;
      }

      cell new_cell = a->new_state[row * a->rows + col];
      new_cell.temperature += curr_cell.sign / pow(24, 2) * sum;

      // TODO cambio espontaneo de signo de la celda
    }
  }
}

void update(automata *a) {
  cell *temp = a->old_state;
  a->old_state = a->new_state;
  a->new_state = temp;
}
