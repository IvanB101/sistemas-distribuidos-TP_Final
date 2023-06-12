#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void print_state(automata a) {
  for (int i = 0; i < a.rows * a.columns; i++) {
    cell c = a.old_state[i];
    printf("S: %d\tC: %.2f\tT: %.2f\n", c.sign, c.conductivity, c.temperature);
  }
}

void print_part_state(automata_part a) {
  for (int i = a.first; i < a.first + a.size; i++) {
    if (!(i % a.rows))
      printf("\n");
    printf(
        "%.2f ",
        (a.old_state[i].conductivity <= 1 ? a.old_state[i].conductivity : 0));
  }
  printf("\n");
}

void print_full_part_state(automata_part a) {
  for (int i = 0; i < a.full_size; i++) {
    if (!(i % a.rows))
      printf("\n");
    printf("%.2f ", a.old_state[i].conductivity);
  }
  printf("\n");
}

void print_usage() {
  printf("Uso:\n");
  printf("<nombre programa> <lado_matriz> <pasos>\n");
  printf("\t-lado_matriz: cantidad de celdas por lado de la matriz\n");
  printf("\t-pasos: cantidad de pasos a simular (100 por defecto)\n");
  exit(-1);
}
