#include "server.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "user/io-server/io_marklin.h"
#include "user/traindata/train_data.h"
#include "user/traintrack/track_data.h"
#include "user/switch-server/interface.h"
#include "user/ui/render.h"

track_node traintrack[TRACK_MAX];

#define SENSOR_DEPTH 2

int find_next_sensor(int cur_node_idx, int switch_server, bool *is_unknown, int *dist_to_next)
{
  do
  {
    if (traintrack[cur_node_idx].type == NODE_EXIT || traintrack[cur_node_idx].type == NODE_NONE)
    {
      *is_unknown = true;
      break;
    }

    if (traintrack[cur_node_idx].type == NODE_BRANCH)
    {
      SwitchMode sw_mode = SwitchGet(switch_server, traintrack[cur_node_idx].num);
      if (sw_mode == SWITCH_MODE_UNKNOWN)
      {
        *is_unknown = true;
        break;
      }
      else if (sw_mode == SWITCH_MODE_C)
      {
        *dist_to_next += traintrack[cur_node_idx].edge[DIR_CURVED].dist;
        cur_node_idx = traintrack[cur_node_idx].edge[DIR_CURVED].dest - traintrack;
      }
      else
      {
        *dist_to_next += traintrack[cur_node_idx].edge[DIR_STRAIGHT].dist;
        cur_node_idx = traintrack[cur_node_idx].edge[DIR_STRAIGHT].dest - traintrack;
      }
    }
    else
    {
      *dist_to_next += traintrack[cur_node_idx].edge[DIR_AHEAD].dist;
      cur_node_idx = traintrack[cur_node_idx].edge[DIR_AHEAD].dest - traintrack;
    }
  } while (traintrack[cur_node_idx].type != NODE_SENSOR);
  return cur_node_idx;
}

void TrainSystemServer()
{
  RegisterAs(TrainSystemAddress);
  int marklin_io = WhoIs(MarklinIOAddress);
  int switch_server = WhoIs(SwitchAddress);

  HashMap *NodeIndexMap = hashmap_new();
  init_tracka(traintrack, NodeIndexMap);

  uint8_t train_states[TRAINS_COUNT] = {0};
  int train_next_sensors[TRAIN_DATA_TRAIN_COUNT][SENSOR_DEPTH] = {
      {0, 44},  // 2   A1 -> C13
      {12, 44}, // 47  A13 -> C13
      {9, 38},  // 54  A10 -> C7
      {7, 38},  // 55  A8 -> C7
      {4, 38},  // 58  A5 -> C7
      {35, 37}  // 77  C4 -> C6
  };

  TrainSystemRequest request;
  TrainSystemResponse response;
  int from_tid;

  while (1)
  {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(TrainSystemRequest));
    if (req_len < 0)
    {
      LOG_ERROR("[TrainSystemServer ERROR]: on receive: %d", req_len);
    }

    switch (request.type)
    {
    case SYSTEM_GET_TRAIN:
    {
      LOG_INFO("[TrainSystemServer INFO]: getting train state of train %d", request.train);
      response = (TrainSystemResponse){
          .type = SYSTEM_GET_TRAIN,
          .train_state = train_states[request.train],
          .train = request.train};
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
      break;
    }
    case SYSTEM_SENSOR_HIT:
    {
      // render_command("[TrainSystemServer INFO]: sensor hit %d -> update train", request.sensor_hit);

      response = (TrainSystemResponse){
          .type = SYSTEM_SENSOR_HIT,
          .train_state = 0,
          .train = -1,
          .next_sensor_id = -1,
          .dist_to_next = -1,
      };

      int sensor_hit = request.sensor_hit;
      int train = -1;
      int train_idx = -1;
      for (int i = 0; i < SENSOR_DEPTH; i++)
      {
        for (int j = 0; j < TRAIN_DATA_TRAIN_COUNT; j++)
        {
          if (train_next_sensors[j][i] == request.sensor_hit)
          {
            train = TRAIN_DATA_TRAINS[j];
            train_idx = j;
            break;
          }
        }
        if (train != -1)
        {
          break;
        }
      }

      if (train == -1 || sensor_hit == -1)
      {
        Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
        break;
      }

      char letter[2] = {'A' + sensor_hit / 16, '\0'};
      string sensor_str = string_format("%s%d", letter, (sensor_hit % 16) + 1);

      bool success;
      int cur_node_idx = (int)(intptr_t)hashmap_get(NodeIndexMap, sensor_str.data, &success);
      if (!success)
      {
        render_command("[TrainSystem Sensor Hit ERROR]: error on getting sensor from hashmap: %s", sensor_str.data);
        Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
        break;
      }

      bool is_unknown = false;
      int dist_to_next = 0;

      int next_sensor_node_idx = find_next_sensor(cur_node_idx, switch_server, &is_unknown, &dist_to_next);
      if (is_unknown)
      {
        render_command("[TrainSystem Sensor Hit ERROR]: no next sensor found");
        train_next_sensors[train_idx][0] = -1;
        train_next_sensors[train_idx][1] = -1;
        Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
        break;
      }
      else
      {
        train_next_sensors[train_idx][0] = traintrack[next_sensor_node_idx].num;
      }

      response.next_sensor_id = traintrack[next_sensor_node_idx].num;
      response.train = train;
      response.train_state = train_states[train];
      response.dist_to_next = dist_to_next;

      int next_next_sensor_id = find_next_sensor(next_sensor_node_idx, switch_server, &is_unknown, &dist_to_next);
      if (is_unknown)
      {
        train_next_sensors[train_idx][1] = -1;
        render_command("[TrainSystem Sensor Hit ERROR]: no next next sensor found");
      }
      else
      {
        train_next_sensors[train_idx][1] = traintrack[next_next_sensor_id].num;
      }

      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
      break;
    }
    case SYSTEM_SET_SPEED:
    {
      // render_command("log: setting train speed of train %d to %d", request.train, request.speed);

      int train = request.train;
      int speed = request.speed;
      train_states[train] = (train_states[train] & ~TRAIN_SPEED_MASK) | speed;

      if (speed == 15)
      { // update the next two sensors
        int train_idx = -1;
        for (int j = 0; j < TRAIN_DATA_TRAIN_COUNT; j++)
        {
          if (TRAIN_DATA_TRAINS[j] == (uint32_t)train)
          {
            train = TRAIN_DATA_TRAINS[j];
            train_idx = j;
            break;
          }
        }
        if (train_idx != -1)
        {
          bool is_unknown, is_unknown2;
          int dist;
          int current_next_sens = train_next_sensors[train_idx][0];
          int new_next_sens = find_next_sensor(current_next_sens, switch_server, &is_unknown, &dist);
          int new_next_next_sens = find_next_sensor(new_next_sens, switch_server, &is_unknown2, &dist);
          new_next_sens = is_unknown ? -1 : new_next_sens;
          new_next_next_sens = is_unknown2 ? -1 : new_next_next_sens;
          train_next_sensors[train_idx][0] = new_next_sens;
          train_next_sensors[train_idx][1] = new_next_next_sens;
          render_predict_next_sensor(train, new_next_sens);
        }
      }

      io_marklin_set_train(marklin_io, train, train_states[train]);

      response = (TrainSystemResponse){
          .type = SYSTEM_SET_SPEED,
          .train_state = train_states[train],
          .train = train};
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      break;
    }
    case SYSTEM_SET_LIGHTS:
    {
      LOG_INFO("[TrainSystemServer INFO]: setting train lights of train %d to %d", request.train, request.light_status);

      int train = request.train;
      bool light_status = request.light_status;
      train_states[train] = light_status ? train_states[train] | TRAIN_LIGHTS_MASK : train_states[train] & ~TRAIN_LIGHTS_MASK;

      io_marklin_set_train(marklin_io, train, train_states[train]);

      response = (TrainSystemResponse){
          .type = SYSTEM_SET_SPEED,
          .train_state = train_states[train],
          .train = train};
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      break;
    }
    default:
    {
      LOG_ERROR("[TrainSystemServer ERROR]: invalid message type");
    }
    }
  }
}
