#ifndef __PATHFINDER_INTERFACE_H__
#define __PATHFINDER_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include "user/traintrack/track_node.h"

#define PathFinderAddress "¯|_pathfinder_/¯"

typedef struct
{
  int source;
  int train;
  int speed;
  int offset; /* offset in millimeters from sensor */
  int destination;
  bool allow_reversal;
} PathFinderRequest;

typedef struct
{
  int trainsys_server;
  track_node* track;
  track_edge** path;
  int edge_count;
  int train;
  int speed;
  int offset;
} PartialPathFinderRequest;

typedef struct
{
  bool success;
} PathFinderResponse;

typedef struct {
    int train;
    int speed;
    int offset;
    char* dest;
    bool allow_reversal;
} Path;

int PlanPath(Path path);

#endif // __PATHFINDER_INTERFACE_H__