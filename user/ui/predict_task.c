#include "predict_task.h"
#include "user/traintrack/track_data.h"
#include "user/nameserver.h"
#include "user/sensor-server/interface.h"
#include "user/switch-server/interface.h"
#include "user/clock-server/interface.h"
#include "user/trainsys-server/interface.h"
#include "user/trainsys/trainsys.h"
#include "user/ui/render.h"
#include "user/traindata/train_data.h"

track_node traintrack[TRACK_MAX];

void predictTask() {

  int clock_server = WhoIs(ClockAddress);
  int sensor_server = WhoIs(SensorAddress);
  int switch_server = WhoIs(SwitchAddress);
  int trainsys_server = WhoIs(TrainSystemAddress);
  
  HashMap *NodeIndexMap = hashmap_new();
  init_tracka(traintrack, NodeIndexMap);

  int last_sensor_time = -1;
  int predicted_sensor_time = -1;

  Delay(clock_server, 30);

  while (1) {
    int sensor_id = WaitOnSensor(sensor_server, -1);
    if (sensor_id < 0)
    {
      LOG_ERROR("[predictTask ERROR]: error on getting hit sensor: %d", sensor_id);
      continue;
    }

    char letter[2] = {'A' + sensor_id / 16, '\0'};
    string sensor_str = string_format("%s%d", letter, (sensor_id % 16) + 1);
    render_predict_current_sensor(sensor_id);

    bool success;
    int cur_node_idx = (int)(intptr_t)hashmap_get(NodeIndexMap, sensor_str.data, &success);
    if (!success) {
      LOG_ERROR("[predictTask ERROR]: error on getting sensor from hashmap: %s", sensor_str.data);
      continue;
    }

    bool is_unknown = false;
    int dist_to_next = 0;

    do {
      if (traintrack[cur_node_idx].type == NODE_EXIT || traintrack[cur_node_idx].type == NODE_NONE) {
        is_unknown = true;
        break;
      }

      if (traintrack[cur_node_idx].type == NODE_BRANCH) {
        SwitchMode sw_mode = SwitchGet(switch_server, traintrack[cur_node_idx].num);
        if (sw_mode == SWITCH_MODE_UNKNOWN) {
          is_unknown = true;
          break;
        } else if (sw_mode == SWITCH_MODE_C) {
          dist_to_next += traintrack[cur_node_idx].edge[DIR_CURVED].dist;
          cur_node_idx = traintrack[cur_node_idx].edge[DIR_CURVED].dest - traintrack;
        } else {
          dist_to_next += traintrack[cur_node_idx].edge[DIR_STRAIGHT].dist;
          cur_node_idx = traintrack[cur_node_idx].edge[DIR_STRAIGHT].dest - traintrack;
        }
      } else {
        dist_to_next += traintrack[cur_node_idx].edge[DIR_AHEAD].dist;
        cur_node_idx = traintrack[cur_node_idx].edge[DIR_AHEAD].dest - traintrack;
      }
    } while (traintrack[cur_node_idx].type != NODE_SENSOR);


    if (!is_unknown) {
      render_predict_next_sensor(traintrack[cur_node_idx].num);
    } else {
      continue;
    }
    int train = trainsys_get_moving_train();
    if (train == -1) {
      continue;
    }
    int train_speed = TrainSystemGetTrainState(trainsys_server, train) & TRAIN_SPEED_MASK;
    int train_vel = train_data_vel(train, train_speed);

    int curr_time = Time(clock_server);
    int elapsed = curr_time - last_sensor_time;
    last_sensor_time = curr_time;

    if (predicted_sensor_time != -1) {
      int t_err = elapsed - predicted_sensor_time;
      int d_err = (t_err*train_vel)/100;
      render_predict_error(t_err, d_err);
    }

    predicted_sensor_time = (dist_to_next/train_vel)*100; // in ticks
  }
} 