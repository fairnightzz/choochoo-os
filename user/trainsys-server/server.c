#include "server.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "user/io-server/io_marklin.h"
#include "user/traindata/train_data.h"
#include "user/traintrack/track_data.h"
#include "user/switch-server/interface.h"
#include "user/ui/render.h"
#include "user/clock-server/interface.h"

#define SENSOR_DEPTH 2

track_node *train_sys_track;

typedef struct
{
  int train;
  int speed;
} ReverseRequest;

typedef struct
{
  bool success;
} ReverseResponse;

void ReverseTask()
{
  int clock_server = WhoIs(ClockAddress);
  ReverseRequest request;
  int requestTid;
  Receive(&requestTid, (char *)&request, sizeof(ReverseRequest));
  ReverseResponse response = (ReverseResponse){
      .success = true,
  };
  Reply(requestTid, (char *)&response, sizeof(ReverseResponse));

  // TrainSystemSetSpeed(SystemState.system_tid, request.train, SPEED_STOP);
  // Delay(SystemState.clock_tid, REV_STOP_DELAY);
  // TrainSystemSetSpeed(SystemState.system_tid, request.train, SPEED_REVERSE);
  // Delay(SystemState.clock_tid, REV_DELAY);
  // TrainSystemSetSpeed(SystemState.system_tid, request.train, request.train_speed);
  // SystemState.trainReverseState[get_train_index(request.train)] = false;

  TrainSystemResponse trainsys_response;
  TrainSystemRequest trainsys_request;

  // Delay(clock_server, train_data_stop_time(request.train, request.speed) / 10 + 100);
  Delay(clock_server, 250);
  trainsys_request = (TrainSystemRequest){
      .type = SYSTEM_REVERSE_REVERSE,
  };
  Send(MyParentTid(), (const char *)&trainsys_request, sizeof(TrainSystemRequest), (char *)&trainsys_response, sizeof(TrainSystemResponse));

  Delay(clock_server, 10); // TODO arbitrary delay
  trainsys_request = (TrainSystemRequest){
      .type = SYSTEM_REVERSE_RESTART,
  };
  Send(MyParentTid(), (const char *)&trainsys_request, sizeof(TrainSystemRequest), (char *)&trainsys_response, sizeof(TrainSystemResponse));
}

int find_next_sensor(int cur_node_idx, int switch_server, bool *is_unknown, int *dist_to_next)
{
  do
  {
    if (train_sys_track[cur_node_idx].type == NODE_EXIT || train_sys_track[cur_node_idx].type == NODE_NONE)
    {
      *is_unknown = true;
      break;
    }

    if (train_sys_track[cur_node_idx].type == NODE_BRANCH)
    {
      SwitchMode sw_mode = SwitchGet(switch_server, train_sys_track[cur_node_idx].num);
      if (sw_mode == SWITCH_MODE_UNKNOWN)
      {
        *is_unknown = true;
        break;
      }
      else if (sw_mode == SWITCH_MODE_C)
      {
        *dist_to_next += train_sys_track[cur_node_idx].edge[DIR_CURVED].dist;
        cur_node_idx = train_sys_track[cur_node_idx].edge[DIR_CURVED].dest - train_sys_track;
      }
      else
      {
        *dist_to_next += train_sys_track[cur_node_idx].edge[DIR_STRAIGHT].dist;
        cur_node_idx = train_sys_track[cur_node_idx].edge[DIR_STRAIGHT].dest - train_sys_track;
      }
    }
    else
    {
      *dist_to_next += train_sys_track[cur_node_idx].edge[DIR_AHEAD].dist;
      cur_node_idx = train_sys_track[cur_node_idx].edge[DIR_AHEAD].dest - train_sys_track;
    }
  } while (train_sys_track[cur_node_idx].type != NODE_SENSOR);
  return cur_node_idx;
}

void TrainSystemServer()
{
  RegisterAs(TrainSystemAddress);
  int marklin_io = WhoIs(MarklinIOAddress);
  int switch_server = WhoIs(SwitchAddress);

  HashMap *NodeIndexMap = get_node_map();
  train_sys_track = get_track();

  uint8_t train_states[TRAINS_COUNT] = {0};
  bool reversed[TRAINS_COUNT] = {0};
  int reverse_tasks[TRAINS_COUNT] = {0}; // 0 means no task is reversing
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
    case SYSTEM_SWITCH_CHANGE:
    {
      response = (TrainSystemResponse){
          .type = SYSTEM_SWITCH_CHANGE};
      int switch_id = request.switch_triggered;
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      int train;
      int train_idx = -1;
      int switch_zone_id = zone_getid_by_switch_id(switch_id);
      for (int i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++)
      {
        if (zone_getid_by_sensor_id(train_next_sensors[i][0]) == switch_zone_id)
        {
          train = TRAIN_DATA_TRAINS[i];
          train_idx = i;
        }
      }
      if (train_idx != -1 && train_next_sensors[train_idx][0] != -1)
      {
        render_command("2 next sensor: %d", train_next_sensors[train_idx][0]);
        bool is_unknown = false;
        bool is_unknown2 = false;
        int dist;
        int current_next_sens = track_prev_sensor(switch_server, train_sys_track + train_next_sensors[train_idx][0]) - train_sys_track;
        render_command("2 current sensor: %d", current_next_sens);
        int new_next_sens = find_next_sensor(current_next_sens, switch_server, &is_unknown, &dist);
        render_command("2 next new sensor: %d", new_next_sens);
        int new_next_next_sens = find_next_sensor(new_next_sens, switch_server, &is_unknown2, &dist);
        new_next_sens = is_unknown ? -1 : new_next_sens;
        new_next_next_sens = is_unknown2 ? -1 : new_next_next_sens;
        train_next_sensors[train_idx][0] = new_next_sens;
        train_next_sensors[train_idx][1] = new_next_next_sens;
        render_predict_next_sensor(train, new_next_sens);
      }
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
        train_next_sensors[train_idx][0] = train_sys_track[next_sensor_node_idx].num;
      }

      response.next_sensor_id = train_sys_track[next_sensor_node_idx].num;
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
        train_next_sensors[train_idx][1] = train_sys_track[next_next_sensor_id].num;
      }

      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));
      break;
    }
    case SYSTEM_SET_SPEED:
    {
      render_command("setting train speed of train %d to %d", request.train, request.speed);

      int train = request.train;
      int speed = request.speed;
      train_states[train] = (train_states[train] & ~TRAIN_SPEED_MASK) | speed;

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

    case SYSTEM_REVERSE:
    {
      int train = request.train;
      int speed = train_states[train] & TRAIN_SPEED_MASK;

      render_command("[TSS Reverse]: train %d speed %d", request.train, speed);

      bool was_already_reversing;
      if (reverse_tasks[train] != 0)
      {
        was_already_reversing = true;
      }
      else
      {
        was_already_reversing = false;
        if (speed == 0)
        {
          uint8_t temp_state = train_states[train];
          speed = 15;
          temp_state = (temp_state & ~TRAIN_SPEED_MASK) | speed;
          io_marklin_set_train(marklin_io, train, temp_state);

          // update the next two sensors
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
          if (train_idx != -1 && train_next_sensors[train_idx][0] != -1)
          {
            render_command("next sensor: %d", train_next_sensors[train_idx][0]);
            bool is_unknown = false;
            bool is_unknown2 = false;
            int dist;
            int current_next_sens = train_sys_track[train_next_sensors[train_idx][0]].reverse - train_sys_track;
            int new_next_sens = find_next_sensor(current_next_sens, switch_server, &is_unknown, &dist);
            int new_next_next_sens = find_next_sensor(new_next_sens, switch_server, &is_unknown2, &dist);
            new_next_sens = is_unknown ? -1 : new_next_sens;
            new_next_next_sens = is_unknown2 ? -1 : new_next_next_sens;
            train_next_sensors[train_idx][0] = new_next_sens;
            train_next_sensors[train_idx][1] = new_next_next_sens;
            render_predict_next_sensor(train, new_next_sens);
          }
        }
        else
        {
          uint8_t temp_state = train_states[train];
          temp_state = (temp_state & ~TRAIN_SPEED_MASK) | 0;
          io_marklin_set_train(marklin_io, train, temp_state);
          reverse_tasks[train] = Create(2, &ReverseTask);

          ReverseResponse reverse_response;
          ReverseRequest reverse_request = (ReverseRequest){
              .train = train,
              .speed = speed};
          Send(reverse_tasks[train], (const char *)&reverse_request, sizeof(ReverseRequest), (char *)&reverse_response, sizeof(ReverseResponse));
        }
      }

      response = (TrainSystemResponse){
          .type = SYSTEM_REVERSE,
          .was_already_reversing = was_already_reversing,
      };
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      break;
    }
    case SYSTEM_REVERSE_REVERSE:
    {
      int train = -1;
      for (int i = 0; i < TRAINS_COUNT; i++)
      {
        if (reverse_tasks[i] == from_tid)
        {
          train = i;
          break;
        }
      }
      if (train == -1)
      {
        LOG_ERROR("Couldn't find train associated with reverse task");
      }
      render_command("[TSS Reverse Reverse]: train %d", train);

      uint8_t temp_state = train_states[train];
      int speed = 15;
      temp_state = (temp_state & ~TRAIN_SPEED_MASK) | speed;
      io_marklin_set_train(marklin_io, train, temp_state);

      // set the train state to reversed
      reversed[train] = !reversed[train];

      response = (TrainSystemResponse){
          .type = SYSTEM_REVERSE_REVERSE,
      };
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      // update the next two sensors
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
      if (train_idx != -1 && train_next_sensors[train_idx][0] != -1)
      {
        render_command("1 next sensor: %d", train_next_sensors[train_idx][0]);
        bool is_unknown = false;
        bool is_unknown2 = false;
        int dist;
        int current_next_sens = train_sys_track[train_next_sensors[train_idx][0]].reverse - train_sys_track;
        int new_next_sens = find_next_sensor(current_next_sens, switch_server, &is_unknown, &dist);
        int new_next_next_sens = find_next_sensor(new_next_sens, switch_server, &is_unknown2, &dist);
        new_next_sens = is_unknown ? -1 : new_next_sens;
        new_next_next_sens = is_unknown2 ? -1 : new_next_next_sens;
        train_next_sensors[train_idx][0] = new_next_sens;
        train_next_sensors[train_idx][1] = new_next_next_sens;
        render_predict_next_sensor(train, new_next_sens);
      }

      break;
    }
    case SYSTEM_REVERSE_RESTART:
    {

      int train = -1;
      for (int i = 0; i < TRAINS_COUNT; i++)
      {
        if (reverse_tasks[i] == from_tid)
        {
          train = i;
          break;
        }
      }
      if (train == -1)
      {
        LOG_ERROR("Couldn't find train associated with reverse task");
      }

      render_command("[TSS Reverse RESTART]: train %d", train);
      io_marklin_set_train(marklin_io, train, train_states[train]);
      reverse_tasks[train] = 0;

      response = (TrainSystemResponse){
          .type = SYSTEM_REVERSE_RESTART,
      };
      Reply(from_tid, (char *)&response, sizeof(TrainSystemResponse));

      break;
    }
    case SYSTEM_GET_NEXT_TRAIN_SENSOR:
    {

      int train = request.train;
      int next_sensor_id = train_next_sensors[get_train_index(train)][0];
      response = (TrainSystemResponse){
          .type = SYSTEM_GET_NEXT_TRAIN_SENSOR,
          .next_sensor_id = next_sensor_id,
      };
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
