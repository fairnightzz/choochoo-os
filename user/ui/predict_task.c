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
#include "user/pactrain-server/interface.h"

void predictTask()
{

  int clock_server = WhoIs(ClockAddress);
  int sensor_server = WhoIs(SensorAddress);
  // int switch_server = WhoIs(SwitchAddress);
  int trainsys_server = WhoIs(TrainSystemAddress);
  int pacman_server = WhoIs(PacTrainAddress);

  int last_sensor_time[TRAIN_DATA_TRAIN_COUNT] = {0};
  int predicted_sensor_time[TRAIN_DATA_TRAIN_COUNT] = {0};
  int pacSensor = -1;
  int ghostSensors[3] = {-1, -1, -1};

  Delay(clock_server, 30);

  while (1)
  {
    int sensor_id = WaitOnSensor(sensor_server, -1);
    if (sensor_id < 0)
    {
      LOG_ERROR("[predictTask ERROR]: error on getting hit sensor: %d", sensor_id);
      continue;
    }
    // render_command("PT: after wait on sensor %s", get_sensor_string(sensor_id).data);

    // Ask TrainSysServer for train that this sensor pertains to

    TrainSystemResponse response = TrainSystemSensorToTrain(trainsys_server, sensor_id);
    int train = response.train;
    int train_speed = response.train_state & TRAIN_SPEED_MASK;
    int next_sensor_id = response.next_sensor_id;
    int prev_sens = response.prev_sensor_id;
    int dist_to_next = response.dist_to_next;

    if (train != -1)
    {
      render_predict_current_sensor(train, sensor_id);
    }

    // Begin rendering on the pacman train server
    // Ask pacman server if train or ghost
    PacTrainType train_type = WhoTrain(pacman_server, train);
    // render_command("train: %d, type: %d, sensor: %s", train, train_type, get_sensor_string(sensor_id).data);

    // if train, render no food on previous sensor
    // render train on current sensor
    // ate food to pacman server
    if (train_type == PAC_TRAIN)
    {
      int prev_sensor_id = pacSensor;
      if (prev_sensor_id == -1)
      {
        prev_sensor_id = prev_sens;
      }

      if (prev_sensor_id != -1)
      {
        bool hasFood = SensorHasFood(pacman_server, prev_sensor_id);
        if (hasFood)
        {
          render_food(prev_sensor_id);
        }
        else
        {
          render_empty_food(prev_sensor_id);
        }
      }
      render_pacman(sensor_id);
      AteFood(pacman_server, sensor_id);
      pacSensor = sensor_id;
    }

    // if ghost, ask if sensor has food on previous sensor
    // render food/no food on previous sensor
    // render ghost on current sensor
    else if (train_type != NONE_TRAIN)
    {
      int prev_sensor_id = ghostSensors[train_type - 1];
      if (prev_sensor_id == -1)
      {
        prev_sensor_id = prev_sens;
      }

      if (prev_sensor_id != -1)
      {
        bool hasFood = SensorHasFood(pacman_server, prev_sensor_id);
        if (hasFood)
        {
          render_food(prev_sensor_id);
        }
        else
        {
          render_empty_food(prev_sensor_id);
        }
      }
      render_ghost(sensor_id);
      ghostSensors[train_type - 1] = sensor_id;
    }

    if (next_sensor_id != -1)
    {
      render_predict_next_sensor(train, next_sensor_id);
    }
    else
    {
      // render_command("bad next sensor id");
      continue;
    }

    int train_index = get_train_index(train);
    if (train_index == -1 || get_speed_index(train_speed) == -1)
    {
      // render_command("bad train index or speed index %d %d", train_index, train_speed);
      continue;
    }
    int train_vel = train_data_vel(train, train_speed);

    int curr_time = Time(clock_server);
    int elapsed = curr_time - last_sensor_time[train_index];
    last_sensor_time[train_index] = curr_time;

    int t_err = elapsed - predicted_sensor_time[train_index];
    int d_err = (t_err * train_vel) / 100;

    // If too high, just don't render
    if (t_err < 500)
    {
      render_predict_t_error(train, t_err);
    }
    if (d_err < 500)
    {
      render_predict_d_error(train, d_err);
    }

    predicted_sensor_time[train_index] = (dist_to_next / train_vel) * 100; // in ticks
  }
}