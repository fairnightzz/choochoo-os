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
#include "user/trainsys-server/interface.h"
#include "user/trainsys/trainsys.h"
#include "user/pathfinder-server/helpers.h"
#define INF 2147483647
#define NONE 2147483646

// Statically allocated arrays used in Dijkstra algorithm
track_edge *route_edges[TRACK_MAX];
void do_train_course(track_node *track, int trainsys_server, int sensor_server, int switch_server, int clock_server, int src, int dest, int train, int train_speed, int offset);

void PathFinderServer()
{
  RegisterAs(PathFinderAddress);

  int sensor_server = WhoIs(SensorAddress);
  int switch_server = WhoIs(SwitchAddress);
  int clock_server = WhoIs(ClockAddress);
  // int io_server = WhoIs(MarklinIOAddress);
  int trainsys_tid = WhoIs(TrainSystemAddress);

  HashMap *NodeIndexMap = get_node_map();
  track_node *track = get_track();

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
    bool success2;
    int end_node_index = (int)(intptr_t)hashmap_get(NodeIndexMap, request.destination, &success2);
    Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));

    TrainSystemSetSpeed(trainsys_tid, request.train, request.speed);

    int start_sensor = WaitOnSensor(sensor_server, -1);
    if (start_sensor == track[start_sensor].num)
    {
      start_sensor = WaitOnSensor(sensor_server, -1);
    }

    // io_marklin_set_train(io_server, request.train, 0);

    if (start_sensor < 0)
    {
      render_command("[PathFinderServer ERROR]: error on getting starting sensor: %d", start_sensor);
      continue;
    }

    char letter[2] = {'A' + start_sensor / 16, '\0'};
    string start_str = string_format("%s%d", letter, (start_sensor % 16) + 1);

    bool success;
    int start_node_index = (int)(intptr_t)hashmap_get(NodeIndexMap, start_str.data, &success);
    if (!success || !success2)
    {
      render_command("[PathFinderServer ERROR] hashmap: src = %s, dest = %s", start_str.data, end_node_index);
      continue;
    }

    // string new_string_life = string_format("start node %s, index = %d, len = %d, dest node %s, index = %d", start_str.data, start_node_index, start_str.length, request.destination, end_node_index);
    // render_command(&new_string_life);

    do_train_course(track, trainsys_tid, sensor_server, switch_server, clock_server, start_node_index, end_node_index, request.train, request.speed, request.offset);
  }

  Exit();
}

void do_train_course(track_node *track, int trainsys_server, int sensor_server, int switch_server, int clock_server, int src, int dest, int train, int train_speed, int offset)
{
  track_edge *route_edges[150];
  int edges_in_path = do_djikstra(track, train, src, dest, true, false, route_edges);
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

  TrainSystemSetSpeed(trainsys_server, train, SPEED_STOP);
}
