#ifndef __PATHFINDER_HELPERS_H__
#define __PATHFINDER_HELPERS_H__

#include "user/traintrack/track_node.h"
#include "user/traintrack/track_data.h"
#include "lib/stdlib.h"

int do_djikstra(track_node *track, int train, int source_node, int dest_node, bool allow_reversal, bool check_reserve, track_edge **edge_graph);

#endif // __PATHFINDER_HELPERS_H__