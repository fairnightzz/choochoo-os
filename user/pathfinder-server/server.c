#include "server.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/sensor-server/interface.h"
#include "user/switch-server/interface.h"
#include "user/clock-server/interface.h"
#include "user/traintrack/track_data.h"
#include "user/io-server/io_marklin.h"
#include "user/traindata/train_data.h"
#include "user/ui/render.h"

#define INF 2147483647
#define NONE 2147483646

// Statically allocated arrays used in Dijkstra algorithm
uint32_t dist[TRACK_MAX];
uint32_t prev[TRACK_MAX];
track_edge *edges[TRACK_MAX];
bool visited[TRACK_MAX];
track_edge *route_edges[TRACK_MAX];
track_node track[TRACK_MAX];

int do_djikstra(track_node *track, int source_node, int dest_node);
void do_train_course(track_node *track, int io_server, int sensor_server, int switch_server, int clock_server, int src, int dest, int train, int train_speed, int offset);

void PathFinderServer()
{
  RegisterAs(PathFinderAddress);

  int sensor_server = WhoIs(SensorAddress);
  int switch_server = WhoIs(SwitchAddress);
  int io_server = WhoIs(MarklinIOAddress);
  int clock_server = WhoIs(ClockAddress);

  HashMap *NodeIndexMap = hashmap_new();

  init_tracka(track, NodeIndexMap);

  PathFinderRequest request;
  PathFinderResponse response;
  int from_tid;

  while (1)
  {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(PathFinderRequest));
    if (req_len < 0)
    {
      render_command("[PathFinderServer ERROR]: error on receive: %d", req_len);
      continue;
    }
    response = (PathFinderResponse){.success = true};
    Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));

    io_marklin_set_train(io_server, request.train, request.speed);

    int start_sensor = WaitOnSensor(sensor_server, -1);

    // io_marklin_set_train(io_server, request.train, 0);

    if (start_sensor < 0)
    {
      render_command("[PathFinderServer ERROR]: error on getting starting sensor: %d", start_sensor);
      continue;
    }

    char letter[2] = {'A' + start_sensor / 16, '\0'};
    string start_str = string_format("%s%d", letter, (start_sensor % 16) + 1);

    bool success, success2;
    int start_node_index = (int)(intptr_t)hashmap_get(NodeIndexMap, start_str.data, &success);
    int end_node_index = (int)(intptr_t)hashmap_get(NodeIndexMap, request.destination, &success2);
    if (!success || !success2)
    {
      render_command("[PathFinderServer ERROR]hashmap error: start node = %s, dest node = %s", start_str.data, request.destination);
      continue;
    }

    // string new_string_life = string_format("start node %s, index = %d, len = %d, dest node %s, index = %d", start_str.data, start_node_index, start_str.length, request.destination, end_node_index);
    // render_command(&new_string_life);

    do_train_course(track, io_server, sensor_server, switch_server, clock_server, start_node_index, end_node_index, request.train, request.speed, request.offset);
  }

  Exit();
}

int do_edge_trace(int cur_node, int src_node, int src_rev_node, int iter_count)
{
  if (cur_node == src_node || cur_node == src_rev_node)
  {
    return iter_count - 1;
  }
  else if (iter_count > 130)
  {
    render_command("[PathfinderServer ERROR] cannot form edge graph through trace");
    return -1;
  }
  int tot_iterations = do_edge_trace(prev[cur_node], src_node, src_rev_node, iter_count + 1);
  route_edges[tot_iterations - iter_count] = edges[cur_node];
  // string render_string = string_format("edge trace: tot_iterations - iter_count: %d, src: %s, dest: %s, dist: %d", tot_iterations - iter_count, edges[cur_node]->src->name, edges[cur_node]->dest->name, edges[cur_node]->dist);
  // render_command(&render_string);
  return tot_iterations;
}

int do_djikstra(track_node *track, int source_node, int dest_node)
{
  for (int i = 0; i < TRACK_MAX; i++)
  {
    dist[i] = INF;
    prev[i] = NONE;
    edges[i] = 0;
    visited[i] = false;
  }

  dist[source_node] = 0;

  while (1)
  {
    // Find next closest node
    int cur_node = NONE;
    for (int i = 0; i < TRACK_MAX; i++)
    {
      if (dist[i] != INF && visited[i] == false)
      {
        if (cur_node == NONE)
        {
          cur_node = i;
        }
        else
        {
          cur_node = dist[cur_node] < dist[i] ? cur_node : i;
        }
      }
    }

    visited[cur_node] = true;
    if (cur_node == dest_node)
      break;

    int dest_rev = track[dest_node].reverse - track;
    if (cur_node == dest_rev)
    {
      dest_node = dest_rev;
      break;
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
  }

  int source_rev_node = track[source_node].reverse - track;
  return do_edge_trace(dest_node, source_node, source_rev_node, 0);
}

void do_train_course(track_node *track, int io_server, int sensor_server, int switch_server, int clock_server, int src, int dest, int train, int train_speed, int offset)
{
  int edges_in_path = do_djikstra(track, src, dest);
  if (edges_in_path == -1)
  {
    render_command("[PathfinderServer]: could not find path in do_train_course");
    return;
  }
  track_node dest_node = track[dest];
  if (offset != 0 && dest_node.type != NODE_SENSOR)
  {
    render_command("[PathfinderServer] can't use offset from node other than sensor");
    return;
  }
  int max_fwd_dist = dest_node.edge[DIR_AHEAD].dist;
  if (offset > 0 && offset > max_fwd_dist)
  {
    render_command("[PathfinderServer] forward offset too large (max value for node %s is %d)", dest_node.name, max_fwd_dist);
    return;
  }

  int max_bck_dist = dest_node.reverse->edge[DIR_AHEAD].dist;
  if (offset < 0 && -offset > max_bck_dist)
  {
    render_command("[PathfinderServer] backward offset too large (max value for node %s is %d)", dest_node.name, max_bck_dist);
    return;
  }

  if (get_train_index(train) == -1 || get_speed_index(train_speed) == -1)
  {
    render_command("[PathfinderServer] un calibrated train speed %d on train %d", train_speed, train);
    return;
  }
  int stopping_distance = train_data_stop_dist(train, train_speed) - offset;
  int train_vel = train_data_vel(train, train_speed);

  track_node *waiting_sensor = 0;
  for (int i = edges_in_path; i >= 0; i--)
  {
    stopping_distance -= route_edges[i]->dist;
    if (stopping_distance <= 0 && route_edges[i]->src->type == NODE_SENSOR)
    {
      waiting_sensor = route_edges[i]->src; // sensor that we should wait to trip
      break;
    }
  }

  int distance_from_sensor = -stopping_distance;
  if (waiting_sensor == 0)
  {
    render_command("[PathfinderServer] could not find usable sensor");
    return;
  }

  for (int i = 0; i <= edges_in_path; i++)
  {
    if (route_edges[i]->src->type == NODE_BRANCH)
    {
      int switch_num = route_edges[i]->src->num;
      if (&(route_edges[i]->src->edge[DIR_STRAIGHT]) == route_edges[i])
      {
        SwitchSet(switch_server, switch_num, SWITCH_MODE_S);
      }
      else
      {
        SwitchSet(switch_server, switch_num, SWITCH_MODE_C);
      }
    }
  }

  // io_marklin_set_train(io_server, train, train_speed);

  string command_life = string_format("[PathFinderServer INFO]: waiting on sensor %s", track[waiting_sensor->num].name);
  render_command(command_life.data);

  WaitOnSensor(sensor_server, waiting_sensor->num);
  string new_command_life = to_string("[PathFinderServer INFO]: hit target waiting sensor");
  render_command(new_command_life.data);

  int delay_ticks = distance_from_sensor * 100 / train_vel;
  Delay(clock_server, delay_ticks);

  io_marklin_set_train(io_server, train, 0);
}
