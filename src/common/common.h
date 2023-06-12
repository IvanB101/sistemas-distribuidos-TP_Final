#ifndef COMMON
#define COMMON

#include <stdint.h>

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ceil_div(X, Y) (((X) % (Y)) ? (X / Y + 1) : (X / Y))
#define sqr(X) (X * X)

typedef struct {
  double temperature;
  double conductivity;
  int8_t sign;
} cell;

typedef struct {
  uint32_t rows;
  uint32_t columns;
  cell *old_state;
  cell *new_state;
} automata;

typedef struct {
  cell *old_state;
  cell *new_state;
  uint32_t first;
  uint32_t size;
  uint32_t full_size;
  uint32_t rows;
  uint32_t columns;
} automata_part;

void print_state(automata a);

void print_part_state(automata_part a);

void print_full_part_state(automata_part a);

void print_usage();

#endif // !COMMON
