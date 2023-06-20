#include "common.h"

#include <stdio.h>
#include <stdlib.h>

void print_state(automata a) {
  double range = 272.0 * 2 / 5;

  for (int i = 0; i < a.rows * a.columns; i++) {
    if (!(i % a.columns)) {
      printf("\n");
    }
    double temp = a.old_state[i].temperature + 272;
    if (temp < range) {
      printf("\033[34m■ \033[0m");
    } else if (temp < 2 * range) {
      printf("\033[96m■ \033[0m");
    } else if (temp < 3 * range) {
      printf("\033[90m■ \033[0m");
    } else if (temp < 4 * range) {
      printf("\033[31m■ \033[0m");
    } else {
      printf("\033[93m■ \033[0m");
    }
  }
  printf("\n");
}

void print_conduc(automata a) {
  // TODO
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

void restric_temp(double *temp) {
  if (*temp > 272.0) {
    *temp = 272.0;
  }
  if (*temp < -272.0) {
    *temp = -272.0;
  }
}
