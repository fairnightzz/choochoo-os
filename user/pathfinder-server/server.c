#include "server.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/sensor-server/interface.h"
#include "user/switch-server/interface.h"
#include "user/clock-server/interface.h"
#include "user/traintrack/track_data.h"
#include "user/traindata/train_data.h"
#include "user/ui/render.h"
#include "user/trainsys-server/interface.h"
#include "user/trainsys/trainsys.h"
#include "user/zone-server/interface.h"
#include "user/pathfinder-server/helpers.h"
#define INF 2147483647
#define NONE 2147483646

void setSwitchesInZone(int switch_server, track_node *track, int zone, SwitchMode *desired_switches)
{
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    int switch_idx = 80 + i * 2;
    if (track[switch_idx].zone == zone && desired_switches[i] != SWITCH_MODE_UNKNOWN)
    {
      int switch_id = (0 <= i && i <= 17) ? i + 1 : i + 135;
      SwitchSet(switch_server, switch_id, desired_switches[i]);
    }
  }
}

void PatherSimplePath(track_node *track, track_edge **simple_path, int edge_count, int train, int speed, int offset)
{
  int clock_server = WhoIs(ClockAddress);
  int sensor_server = WhoIs(SensorAddress);
  int switch_server = WhoIs(SwitchAddress);
  int trainsys_server = WhoIs(TrainSystemAddress);
  int reserve_server = WhoIs(ZoneAddress);

  int stopping_distance = 0;
  int train_vel = 0;

  track_node *waiting_sensor = 0;
  int train_speed = speed;
  for (int idx = get_speed_index(speed); idx >= 0; --idx)
  {
    train_speed = TRAIN_DATA_SPEEDS[idx];
    stopping_distance = train_data_stop_dist(train, train_speed) - offset;
    // render_command("stopping distance: %d train_speed: %d", stopping_distance, train_speed);
    train_vel = train_data_vel(train, train_speed);

    waiting_sensor = 0;
    int last_edge_idx = (edge_count > 1) ? edge_count - 1 : 0;
    while (last_edge_idx >= 0)
    {
      track_edge *edge = simple_path[last_edge_idx];
      // render_command("edge node %s", edge->src->name);
      stopping_distance -= edge->dist;
      if (stopping_distance <= 0 && edge->src->type == NODE_SENSOR)
      {
        waiting_sensor = edge->src;
        break;
      }
      last_edge_idx -= 1;
    }

    if (waiting_sensor != 0 && simple_path[0]->src != waiting_sensor)
    {
      break;
    }
  }
  // render_command("Waiting Sensor %s", waiting_sensor->name);


  // compute desired switches
  SwitchMode desired_switch_modes[SWITCH_COUNT];
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    desired_switch_modes[i] = SWITCH_MODE_UNKNOWN;
  }

  for (int i = 0; i < edge_count; i++)
  {
    track_edge *edge = simple_path[i];
    if (edge->src->type == NODE_BRANCH)
    {
      int switch_num = edge->src->num;
      switch_num = (1 <= switch_num && switch_num <= 18) ? switch_num - 1 : switch_num - 135;
      if (track_edge_cmp(edge->src->edge[DIR_STRAIGHT], *edge))
      {
        desired_switch_modes[switch_num] = SWITCH_MODE_S;
      }
      else
      {        
        desired_switch_modes[switch_num] = SWITCH_MODE_C;
      }
    }
  }

  int distance_from_sensor = -stopping_distance;
  track_edge *first_edge = simple_path[0];
  int my_zone = first_edge->src->reverse->zone;
  int next_zone = track_next_sensor(switch_server, first_edge->src)->reverse->zone;
  int immediate_zones[2] = {my_zone, next_zone};
  for (int i = 0; i < 2; i++)
  {
    int zone = immediate_zones[i];
    setSwitchesInZone(switch_server, track, zone, desired_switch_modes);
  }

  if (waiting_sensor == 0 || simple_path[0]->src == waiting_sensor)
  {
    int distance_to_dest = 0;
    for (int i = 0; i < edge_count; ++i)
    {
      track_edge *edge = simple_path[i];
      distance_to_dest += edge->dist;
    }
    TrainSystemSetSpeed(trainsys_server, train, TRAIN_DATA_SHORT_MOVE_SPEED);
    Delay(clock_server, train_data_short_move_time(train, distance_to_dest) / 10);
    TrainSystemSetSpeed(trainsys_server, train, 0);
    Delay(clock_server, train_data_stop_time(train, TRAIN_DATA_SHORT_MOVE_SPEED) / 10 + 100);
  }
  else
  {
    TrainSystemSetSpeed(trainsys_server, train, train_speed);
    for (int i = 1; i < edge_count; i++)
    {
      track_edge *edge = simple_path[i];
      if (edge->src->type == NODE_SENSOR)
      {
        int new_pos = WaitOnSensor(sensor_server, edge->src->num);
        track_node *node = track + new_pos;
        track_node *next_node = track_next_sensor(switch_server, node);

        int next_zone = next_node->reverse->zone;
        setSwitchesInZone(switch_server, track, next_zone, desired_switch_modes);

        if (node->zone != -1)
        {
          zone_unreserve(reserve_server, train, node->zone);
        }

        if (edge->src->num == waiting_sensor->num)
          break;
      }
    }

    Delay(clock_server, distance_from_sensor * 100 / train_vel);
    TrainSystemSetSpeed(trainsys_server, train, 0);
    Delay(clock_server, train_data_stop_time(train, TRAIN_DATA_SHORT_MOVE_SPEED) / 10 + 100);
  }

  zone_unreserve_all(reserve_server, train);
  int dest_zone = simple_path[edge_count - 1]->dest->reverse->zone;
  zone_reserve(reserve_server, train, dest_zone);
}
void PatherComplexPath(int trainsys_server, track_node *track, track_edge **path, int edge_count, int train, int speed, int offset)
{
  render_command("Starting Complex Path...");
  // no work to do
  if (path[0] == 0)
    return;

  // break path into simple paths (no reversals)
  track_edge *simple_path[TRACK_MAX + 1] = {0};
  int sind = 0;
  for (int i = 0; i < edge_count; ++i)
  {

    track_edge *cur_edge = path[i];
    simple_path[sind] = cur_edge;
    sind += 1;

    // check for reversal
    if (cur_edge->type == EDGE_REVERSE)
    {
      render_command("reversal edge detected");
      if (sind > 1)
      {
        PatherSimplePath(track, simple_path, sind, train, speed, offset);
      }
      TrainSystemReverse(trainsys_server, train);
      for (int j = 0; j < sind; j++)
      {
        simple_path[j] = 0;
      }
      sind = 0;
    }
  }

  if (sind > 0)
  {
    PatherSimplePath(track, simple_path, sind, train, speed, offset);
  }
}

void PartialPathFinderTask()
{
  int from_tid;
  PathFinderResponse response;
  PartialPathFinderRequest request;
  int req_len = Receive(&from_tid, (char *)&request, sizeof(PartialPathFinderRequest));

  if (req_len < 0)
  {
    render_command("[PartialPathFinderTask] error on receive");
    response = (PathFinderResponse){.success = false};
    Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));
    Exit();
    return;
  }

  PatherComplexPath(request.trainsys_server, request.track, request.path, request.edge_count, request.train, request.speed, request.offset);
  response = (PathFinderResponse){.success = true};
  Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));
  Exit();
}

void PathFinderTask()
{
  int trainsys_server = WhoIs(TrainSystemAddress);
  int reserve_server = WhoIs(ZoneAddress);

  track_node *track = get_track();
  int from_tid;
  PathFinderRequest request;
  PathFinderResponse response;
  int req_len = Receive(&from_tid, (char *)&request, sizeof(PathFinderRequest));

  if (req_len < 0)
  {
    render_command("[PathFinderTask]: error when receiving");
    response = (PathFinderResponse){.success = false};
    Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));
    Exit();
    return;
  }

  response = (PathFinderResponse){.success = true};

  int src = request.source;
  int dest = request.destination;
  int train = request.train;
  int speed = request.speed;
  int offset = request.offset;
  bool allow_reversal = request.allow_reversal;


  Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));

  // render_command("PathFinderTask: pathing from %d to %d on train %d", src, dest, train);
  if (src == dest || src == track[dest].reverse - track)
  {
    render_command("[PathFinderTask] Source Equals Destination");
    Exit();
  }
  
  track_edge *route_edges[TRACK_MAX + 1];
  // render_command("doing djikstra reversal: %d", allow_reversal);
  int edge_count = do_djikstra(track, train, src, dest, allow_reversal, true, route_edges);
  render_command("edge count %d", edge_count);
  if (edge_count == -1)
  {
    render_command("[PATHER] djikstra cannot find path, recompute a blocking path");
    edge_count = do_djikstra(track, train, src, dest, allow_reversal, false, route_edges);
  }

  // render_command("starting complex pathing :((()))");

  track_edge *complex_path[TRACK_MAX + 1] = {0};
  int cind = 0;
  for (int i = 0; i < edge_count; i++)
  {
    track_edge *edge = route_edges[i];
  
    int zone = edge->dest->reverse->zone;
    if (zone != -1)
    {
      if (!zone_reserve(reserve_server, train, zone))
      {
        int partialPathTask = Create(5, &PartialPathFinderTask);
        PathFinderResponse pp_response;
        PartialPathFinderRequest pp_request = (PartialPathFinderRequest){
            .trainsys_server = trainsys_server,
            .track = track,
            .path = complex_path,
            .edge_count = cind,
            .train = train,
            .speed = speed,
            .offset = offset};
        Send(partialPathTask, (const char *)&pp_request, sizeof(PartialPathFinderRequest), (char *)&pp_response, sizeof(PathFinderResponse));

        zone_wait(reserve_server, train, zone);
        if (!zone_reserve(reserve_server, train, zone))
        {
          render_command("[ERROR] should have claimed zone");
        }

        for (int i = 0; i < TRACK_MAX + 1; i++)
        {
          complex_path[i] = 0;
        }
        cind = 0;
      }
    }

    complex_path[cind] = edge;
    cind += 1;
  }

  PatherComplexPath(trainsys_server, track, complex_path, cind, train, speed, offset);

  Exit();
}
