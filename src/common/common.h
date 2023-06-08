#include <stdint.h>

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))
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
