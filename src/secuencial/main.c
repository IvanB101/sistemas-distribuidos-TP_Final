#include <stdint.h>
#include <stdio.h>

typedef struct {
  double temperature;
  double conductivity;
  uint8_t sign;
} cell;

int main(int argc, char **argv) {
  cell matrix[100][100];

  return 0;
}

void calculate(cell *old, cell *new, uint32_t rows, uint32_t columns) {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < columns; col++) {

      for (int vec_row = -2; vec_row <= 2; vec_row++) {
        if (row + vec_row < 0) {
          continue;
        }

        for (int vec_col = -2; vec_col <= 2; vec_col++) {
          if (col + vec_col < 0) {
            continue;
          }
        }
      }
    }
  }
}
