#ifndef __PATHFINDER_SERVER_H__
#define __PATHFINDER_SERVER_H__

#include "interface.h"
#include "lib/stdlib.h"

typedef struct {
  int train;
  int speed;
  int offset;
  char *dest;
  bool allow_reversal;
} Route;

int AllAboard(Route path);

#endif // __PATHFINDER_SERVER_H__