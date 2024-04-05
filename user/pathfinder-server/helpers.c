#include "helpers.h"
#include "user/ui/render.h"
#include "user/zone-server/interface.h"
#include "user/nameserver.h"
#include "user/pactrain-server/interface.h"

#define INF 2147483647
#define NONE 2147483646


int do_edge_trace(int cur_node, int src_node, int iter_count, uint32_t *prev, track_edge** edges, track_edge **edge_graph)
{
  if (cur_node == src_node)
  {
    return iter_count - 1;
  }
  else if (iter_count > 130)
  {
    render_command("[EdgeTrace ERROR] cannot form edge graph through trace");
    return -1;
  }
  int tot_iterations = do_edge_trace(prev[cur_node], src_node, iter_count + 1, prev, edges, edge_graph);
  // render_command("[Edge Trace]: %s -> %s, dist %d, type %d", edges[cur_node]->src->name, edges[cur_node]->dest->name, edges[cur_node]->dist,edges[cur_node]->type);
  edge_graph[tot_iterations - iter_count] = edges[cur_node];
  // string render_string = string_format("edge trace: tot_iterations - iter_count: %d, src: %s, dest: %s, dist: %d", tot_iterations - iter_count, edges[cur_node]->src->name, edges[cur_node]->dest->name, edges[cur_node]->dist);
  // render_command(&render_string);
  return tot_iterations;
}

int do_djikstra(track_node *track, int train, int source_node, int dest_node, bool allow_reversal, bool check_reserve, track_edge **edge_graph) {
  int reserve_server = WhoIs(ZoneAddress);
  int pac_server = WhoIs(PacTrainAddress);

  PacTrainType train_type = WhoTrain(pac_server, train);
  int *dest_sensors = GetFoodSensors(pac_server);
  
  uint32_t dist[TRACK_MAX];
  uint32_t prev[TRACK_MAX];
  track_edge* edges[TRACK_MAX];
  bool visited[TRACK_MAX];

  for (int i = 0; i < TRACK_MAX; i++) {
    dist[i] = INF;
    prev[i] = NONE;
    edges[i] = 0;
    visited[i] = false;
  }
  dist[source_node] = 0;

  while (1) {
    int cur_node = NONE;
    for (int i = 0; i < TRACK_MAX; i++) {
      if (dist[i] != INF && visited[i] == false) {
        if (cur_node == NONE) {
          cur_node = i;
        } else {
          cur_node = dist[cur_node] < dist[i] ? cur_node : i;
        }
      }
    }
    if (cur_node == NONE) {
      return -1;
    }
    visited[cur_node] = true;
    if (train_type == PAC_TRAIN) {
      if (check_reserve) {
        int dest_rev = track[cur_node].reverse - track;
        if (dest_sensors[cur_node] == 1) {
          dest_node = cur_node;
          break;
        }
        if (dest_sensors[dest_rev] == 1) {
          dest_node = dest_rev;
          break;
        }
      } else {
        if (dist[cur_node] > 0) {
          dest_node = cur_node;
          break;
        }
      }
    } else {
      if (cur_node == dest_node) {
        break;
      }
      int dest_rev = track[dest_node].reverse - track;
      if (cur_node == dest_rev) {
        dest_node = dest_rev;
        break;
      }
  
      if (check_reserve) {
        int cur_zone = track[cur_node].reverse->zone;
        if (cur_zone != -1 && zone_is_reserved(reserve_server, train, cur_zone)) {
          continue;
        }
      }
    }
    if (track[cur_node].type == NODE_SENSOR || track[cur_node].type == NODE_MERGE)
    {
      track_edge *next_edge = &track[cur_node].edge[DIR_AHEAD];
      int next_node_idx = next_edge->dest - track;
      if (dist[cur_node] + next_edge->dist < dist[next_node_idx])
      {
        dist[next_node_idx] = dist[cur_node] + next_edge->dist;
        prev[next_node_idx] = cur_node;
        edges[next_node_idx] = next_edge;
      }
    }
    else if (track[cur_node].type == NODE_BRANCH)
    {
      track_edge *edge_straight = &track[cur_node].edge[DIR_STRAIGHT];
      int next_node_idx = edge_straight->dest - track;
      if (dist[cur_node] + edge_straight->dist < dist[next_node_idx])
      {
        dist[next_node_idx] = dist[cur_node] + edge_straight->dist;
        prev[next_node_idx] = cur_node;
        edges[next_node_idx] = edge_straight;
      }

      track_edge *edge_curved = &track[cur_node].edge[DIR_CURVED];
      next_node_idx = edge_curved->dest - track;
      if (dist[cur_node] + edge_curved->dist < dist[next_node_idx])
      {
        dist[next_node_idx] = dist[cur_node] + edge_curved->dist;
        prev[next_node_idx] = cur_node;
        edges[next_node_idx] = edge_curved;
      }
    }

    if (allow_reversal && track[cur_node].type == NODE_SENSOR) {
      track_edge* edge_rev = &track[cur_node].edge[DIR_REVERSE];
      int next_rev_node_idx = track[cur_node].reverse - track;
      if (dist[cur_node] + edge_rev->dist < dist[next_rev_node_idx]) {
          dist[next_rev_node_idx] = dist[cur_node] + edge_rev->dist;
          prev[next_rev_node_idx] = cur_node;
          edges[next_rev_node_idx] = edge_rev;
      }
    }
  }

  return do_edge_trace(dest_node, source_node, 0, prev, edges, edge_graph) + 1;
}