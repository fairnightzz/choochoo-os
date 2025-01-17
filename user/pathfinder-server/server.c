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
#include "user/pactrain-server/interface.h"

#define INF 2147483647
#define NONE 2147483646

void setSwitchesInZone(int switch_server, track_node *track, int zone, SwitchMode *desired_switches)
{
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    int switch_idx = 80 + i * 2;
    if (track[switch_idx].zone == zone && desired_switches[i] != SWITCH_MODE_UNKNOWN)
    {
      render_command("setting switch %d in zone %d", track[switch_idx].num, zone);
      int switch_id = (0 <= i && i <= 17) ? i + 1 : i + 135;
      SwitchSet(switch_server, switch_id, desired_switches[i]);
    }
  }
}

void PatherSimplePath(track_node *track, track_edge **simple_path, int edge_count, int train, int speed, int offset, bool final_destination, int *reservations, bool src_is_rev)
{
  // render_command("Simple Command: %s -> %s, edge_count %d", simple_path[0]->src->name, simple_path[edge_count - 1]->dest->name, edge_count);
  int sensor_server = WhoIs(SensorAddress);
  int clock_server = WhoIs(ClockAddress);
  int switch_server = WhoIs(SwitchAddress);
  int trainsys_server = WhoIs(TrainSystemAddress);
  int reserve_server = WhoIs(ZoneAddress);
  int stopping_distance = 0;
  // int train_vel = 0;

  track_node *waiting_sensor = 0;
  int train_speed = speed;
  for (int idx = get_speed_index(speed); idx >= 0; --idx)
  {
    train_speed = TRAIN_DATA_SPEEDS[idx];
    stopping_distance = train_data_stop_dist(train, train_speed) - offset;
    // render_command("stopping distance: %d train_speed: %d", stopping_distance, train_speed);
    // train_vel = train_data_vel(train, train_speed);

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
  // render_command("stopping distance %d on train speed %d", stopping_distance, train_speed);

  // render_command("Found Waiting Sensor: %s", waiting_sensor->name);

  // compute desired switches
  /*
  SwitchMode desired_switch_modes[SWITCH_COUNT];
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    desired_switch_modes[i] = SWITCH_MODE_UNKNOWN;
  }
*/

  for (int i = 0; i < edge_count; i++)
  {
    track_edge *edge = simple_path[i];
    if (edge->src->type == NODE_BRANCH)
    {
      int switch_num = edge->src->num;
      if (track_edge_cmp(edge->src->edge[DIR_STRAIGHT], *edge))
      {
        SwitchSet(switch_server, switch_num, SWITCH_MODE_S);
      }
      else
      {
        SwitchSet(switch_server, switch_num, SWITCH_MODE_C);
      }
    }
  }

  /*
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
*/

  int slow_speed = final_destination ? TRAIN_DATA_SHORT_MOVE_SPEED[get_train_index(train)] - 2 : TRAIN_DATA_SHORT_MOVE_SPEED[get_train_index(train)];
  int dest = simple_path[edge_count - 1]->type == EDGE_REVERSE ? simple_path[edge_count - 1]->src - track : simple_path[edge_count - 1]->dest - track;
  int edge_cutoff = 0;
  if (waiting_sensor == 0 || simple_path[0]->src == waiting_sensor)
  {
    // render_command("starting short move");
    TrainSystemSetSpeed(trainsys_server, train, slow_speed);
    render_command("[PFS INFO]: train %d waiting on destination sensor %s", train, get_sensor_string(dest));
    WaitOnSensor(sensor_server, dest);
    if (src_is_rev)
    {
      int src_zone = simple_path[0]->src->zone;
      if (reservations[src_zone] > 0 && reservations[src_zone] - 1 == 0)
      {
        zone_unreserve(reserve_server, train, src_zone);
      }
      reservations[src_zone] -= 1;
      src_is_rev = false;
    }
    for (int i = edge_cutoff; i < edge_count; i++)
    {
      track_edge *past_edge = simple_path[i];
      int res_zone = past_edge->dest->zone;
      if (res_zone != -1)
      {
        // render_command("reservations at zone %d: %d", res_zone, reservations[res_zone]);
        if (reservations[res_zone] > 0 && reservations[res_zone] - 1 == 0)
        {
          zone_unreserve(reserve_server, train, res_zone);
        }
        reservations[res_zone] -= 1;
      }
    }
    // TrainSystemSetSpeed(trainsys_server, train, 0);
    // Delay(clock_server, 100);
    if (final_destination)
    {
      TrainSystemStop(trainsys_server, train);
      Delay(clock_server, 100);
      TrainSystemStop(trainsys_server, train);
      Delay(clock_server, 100);
    }
    else
    {
      TrainSystemSetSpeed(trainsys_server, train, 0);
      Delay(clock_server, 100);
    }
  }
  else
  {
    TrainSystemSetSpeed(trainsys_server, train, train_speed);
    render_command("[PFS INFO]: train %d waiting on sensor %s", train, waiting_sensor->name);

    int last_triggered = -1;
    while (last_triggered != dest)
    {
      last_triggered = WaitOnSensor(sensor_server, -1);
      if (last_triggered == dest)
      {
        // TrainSystemSetSpeed(trainsys_server, train, 0);
        // Delay(clock_server, 100);
        if (final_destination)
        {
          TrainSystemStop(trainsys_server, train);
          Delay(clock_server, 100);
          TrainSystemStop(trainsys_server, train);
          Delay(clock_server, 100);
        }
        else
        {
          TrainSystemSetSpeed(trainsys_server, train, 0);
          Delay(clock_server, 100);
        }
      }
      else if (last_triggered == waiting_sensor->num)
      {
        TrainSystemSetSpeed(trainsys_server, train, slow_speed);
        render_command("[PFS INFO]: train %d waiting on destination sensor %s", train, get_sensor_string(dest));
      }
      for (int i = edge_cutoff; i < edge_count; i++)
      {
        track_edge *edge = simple_path[i];
        if (edge->dest->num == last_triggered)
        {
          if (src_is_rev)
          {
            int src_zone = simple_path[0]->src->zone;
            if (reservations[src_zone] > 0 && reservations[src_zone] - 1 == 0)
            {
              zone_unreserve(reserve_server, train, src_zone);
            }
            reservations[src_zone] -= 1;
            src_is_rev = false;
          }
          for (int j = edge_cutoff; j <= i; j++)
          {
            track_edge *past_edge = simple_path[j];
            int res_zone = past_edge->dest->zone;
            if (res_zone != -1)
            {
              if (reservations[res_zone] > 0 && reservations[res_zone] - 1 == 0)
              {
                zone_unreserve(reserve_server, train, res_zone);
              }
              reservations[res_zone] -= 1;
            }
          }
          edge_cutoff = i + 1;
        }
      }
    }
  }
}

void PatherComplexPath(int trainsys_server, track_node *track, track_edge **path, int edge_count, int train, int speed, int offset, int *reservations)
{
  int clock_server = WhoIs(ClockAddress);
  int reserve_server = WhoIs(ZoneAddress);

  // render_command("Starting Complex Path...");
  // no work to do
  if (path[0] == 0)
    return;

  // break path into simple paths (no reversals)
  bool src_is_rev = false;
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
      // render_command("reversal edge detected");
      if (sind > 1)
      {
        PatherSimplePath(track, simple_path, sind, train, speed, 0, false, reservations, src_is_rev);
      }
      TrainSystemReverse(trainsys_server, train);
      src_is_rev = true;
      Delay(clock_server, 60); // delay needed to make sure train reverses
      for (int j = 0; j < sind; j++)
      {
        simple_path[j] = 0;
      }
      sind = 0;
    }
  }

  if (sind > 0)
  {
    PatherSimplePath(track, simple_path, sind, train, speed, offset, true, reservations, src_is_rev);
  }

  int dest_zone = path[edge_count - 1]->dest->reverse->zone;
  zone_unreserve_all_except(reserve_server, train, dest_zone);
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

  PatherComplexPath(request.trainsys_server, request.track, request.path, request.edge_count, request.train, request.speed, request.offset, request.path_reservations);
  response = (PathFinderResponse){.success = true};
  Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));
  Exit();
}

void PathFinderTask()
{
  int trainsys_server = WhoIs(TrainSystemAddress);
  int reserve_server = WhoIs(ZoneAddress);
  int switch_server = WhoIs(SwitchAddress);
  int pacman_server = WhoIs(PacTrainAddress);

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

  int dest = request.destination;
  int train = request.train;
  int speed = request.speed;
  int offset = request.offset;
  bool allow_reversal = request.allow_reversal;

  Reply(from_tid, (char *)&response, sizeof(PathFinderResponse));

  PacTrainType train_type = WhoTrain(pacman_server, train);
  int deadlock_counter = 0;

deadlock_recompute:;

  if (deadlock_counter > 2) {
    render_command("[PathFinderTask INFO]: Hard deadlock detected. Aborting path on train %d.", train);
    Exit();
    return;
  }

  TrainSystemResponse resp = TrainSystemGetTrainPosition(trainsys_server, train);

  int start_position = track[resp.position].reverse->num == resp.next_sensor_id ? resp.next_sensor_id : resp.position;
  int src = (resp.train_state & TRAIN_SPEED_MASK) == 0 ? start_position : track_next_sensor(switch_server, track + resp.next_sensor_id)->num;
  render_command("[PathFinderTask INFO] Train: %d Source Sensor: %s Destination Sensor: %s", train, get_sensor_string(src), get_sensor_string(dest));
  if (src == -1)
  {
    render_command("[PathFinderTask INFO] Train %d has unknown current position, aborting PlanPath", train);
    Exit();
    return;
  }

  // render_command("PathFinderTask: pathing from %d to %d on train %d", src, dest, train);
  if (src == dest || src == track[dest].reverse - track)
  {
    render_command("[PathFinderTask] Source Equals Destination");
    Exit();
  }

  track_edge *route_edges[TRACK_MAX + 1];
  int *poss_dest_sensors = GetFoodSensors(pacman_server);
  
  int edge_count = -1;

  if (train_type == PAC_TRAIN) {
    for (int i = 0; i < 80; i++) {
      if (src != i && poss_dest_sensors[i] == 1) {
        edge_count = do_djikstra(track, train, src, i, allow_reversal, true, route_edges);
        if (edge_count != -1) {
          break;
        }
      }
    }
    if (edge_count == -1) {
      for (int i = 3; i < 80; i++) {
        if (src != i) {
          edge_count = do_djikstra(track, train, src, i, allow_reversal, true, route_edges);
          if (edge_count != -1) {
            break;
          }
        }
      }
    }
    if (edge_count == -1) {
      NotifyPacServerDeadlock(pacman_server);
    }
  } else {
    // render_command("doing djikstra reversal: %d", allow_reversal);
    edge_count = do_djikstra(track, train, src, dest, allow_reversal, true, route_edges);
    // render_command("edge count %d", edge_count);
    if (edge_count == -1)
    {
      render_command("[PATHER] djikstra cannot find path, recompute a blocking path");
      edge_count = do_djikstra(track, train, src, dest, allow_reversal, false, route_edges);
    }
  }
  render_train_destination(train, route_edges[edge_count - 1]->dest->num);

  // render_command("starting complex pathing :((()))");

  track_edge *complex_path[TRACK_MAX + 1] = {0};
  int zone_sem[ZONE_COUNT + 1] = {0};
  // render_command("[PATHER] source zone: %d", track[src].reverse->zone);
  zone_sem[track[src].reverse->zone] = 1;
  int cind = 0;
  int last_sensor_dest_edge = -1;
  for (int i = 0; i < edge_count; i++)
  {
    track_edge *edge = route_edges[i];

    int zone = edge->dest->reverse->zone;
    if (zone != -1)
    {
      // render_command("partial path edge_count: %d", last_sensor_dest_edge + 1);
      if (!zone_reserve(reserve_server, train, zone))
      {
        // render_command("could not reserve");
        if (last_sensor_dest_edge != -1)
        {
          int partialPathTask = Create(5, &PartialPathFinderTask);
          PathFinderResponse pp_response;
          PartialPathFinderRequest pp_request = (PartialPathFinderRequest){
              .trainsys_server = trainsys_server,
              .track = track,
              .path = complex_path,
              .edge_count = last_sensor_dest_edge + 1,
              .train = train,
              .speed = speed,
              .offset = offset,
              .path_reservations = zone_sem};
          Send(partialPathTask, (const char *)&pp_request, sizeof(PartialPathFinderRequest), (char *)&pp_response, sizeof(PathFinderResponse));

          for (int j = last_sensor_dest_edge + 1; j < TRACK_MAX + 1; j++)
          {
            complex_path[j - (last_sensor_dest_edge + 1)] = complex_path[j];
          }
          cind = cind - (last_sensor_dest_edge + 1);
          last_sensor_dest_edge = -1;
        }

        // render_command("Before zone wait %d ", zone);
        bool deadlock = zone_wait(reserve_server, train, zone);
        if (deadlock)
        {
          render_command("[PFS INFO]: deadlock detected -> recomputing path on train %d", train);
          deadlock_counter += 1;
          goto deadlock_recompute;
        }
        // render_command("after zone wait");
        if (!zone_reserve(reserve_server, train, zone))
        {
          render_command("[ERROR] nde have claimed zone");
        }
        else
        {
          zone_sem[zone] += 1;
        }
      }
      else
      {
        zone_sem[zone] += 1;
      }
    }
    // render_command("cind %d, last sensor %d", cind, last_sensor_dest_edge);
    complex_path[cind] = edge;
    if (edge->dest->type == NODE_SENSOR)
    {
      last_sensor_dest_edge = cind;
    }
    cind += 1;
  }
  // render_command("cind: %d", cind);
  // for (int j = 0; j < cind; j ++) {
  //  render_command("[EDGE]: %s -> %s", complex_path[j]->src->name, complex_path[j]->dest->name);
  // }

  PatherComplexPath(trainsys_server, track, complex_path, cind, train, speed, offset, zone_sem);

  Exit();
}
