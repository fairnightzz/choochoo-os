#ifndef __TRACK_DATA_H__
#define __TRACK_DATA_H__
/* THIS FILE IS GENERATED CODE -- DO NOT EDIT */

#include "track_node.h"
#include "lib/stdlib.h"

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144
#define TRACK_A_SIZE 144
#define TRACK_B_SIZE 140

#define ZONE_MAX_SENSORS 8
#define ZONE_MAX_SWITCHES 6
#define NUM_ZONES 30

typedef struct
{
  char *sensors[ZONE_MAX_SENSORS];
  int switches[ZONE_MAX_SWITCHES];
} Zones;

void init_tracka(track_node *track, HashMap *nodeMap);
void init_trackb(track_node *track, HashMap *nodeMap);
bool track_edge_cmp(track_edge a, track_edge b);
track_edge *track_next_edge(int switch_server, track_node *node);
track_node *track_next_node(int switch_server, track_node *node);
track_node *track_next_sensor(int switch_server, track_node *node);
track_node *track_prev_sensor(int switch_server, track_node *node);
track_node *track_node_by_name(track_node *track, char *name);
#endif // __TRACK_DATA_H__