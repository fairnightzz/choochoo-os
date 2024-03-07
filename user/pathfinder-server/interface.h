#ifndef __PATHFINDER_INTERFACE_H__
#define __PATHFINDER_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>

#define PathFinderAddress "¯|_pathfinder_/¯"

typedef struct
{
  int train;
  int speed;
  int offset; /* offset in millimeters from sensor */
  char *destination;
} PathFinderRequest;

typedef struct
{
  bool success;
} PathFinderResponse;

int FindPath(int pathfinder_server, int train, int speed, int offset, char *destination);

#endif // __PATHFINDER_INTERFACE_H__