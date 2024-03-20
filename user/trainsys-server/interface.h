#ifndef __TRAIN_SYSTEM_INTERFACE_H__
#define __TRAIN_SYSTEM_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define TrainSystemAddress "CHOOCHOO-TRAIN-SYSTEM"

#define TRAINS_COUNT 81
#define TRAIN_SPEED_MASK 0xF
#define TRAIN_LIGHTS_MASK 0x10
#define REV_STOP_DELAY 300
#define REV_DELAY 100

typedef enum
{
  SYSTEM_SET_SPEED = 1,
  SYSTEM_SET_LIGHTS,
  SYSTEM_GET_TRAIN,
  SYSTEM_SENSOR_HIT,
  SYSTEM_SWITCH_CHANGE,
  SYSTEM_REVERSE,
  SYSTEM_REVERSE_REVERSE, // for the reverse task
  SYSTEM_REVERSE_RESTART, // for the reverse restart task
  SYSTEM_GET_TRAIN_POSITION,
} TrainSystemRequestType;

typedef struct
{
  TrainSystemRequestType type;
  int train;
  int speed;
  bool light_status;
  int sensor_hit;
  int switch_triggered;
} TrainSystemRequest;

typedef struct
{
  TrainSystemRequestType type;
  uint8_t train_state;
  int train;
  int next_sensor_id;
  int dist_to_next;
  bool was_already_reversing;
  int position;
} TrainSystemResponse;

int TrainSystemSetSpeed(int system_server, int train, int speed);
int TrainSystemSetLights(int system_server, int train, bool status);
TrainSystemResponse TrainSystemSensorToTrain(int system_server, int sensor_id);
uint8_t TrainSystemGetTrainState(int system_server, int train);
void TrainSystemSwitchToTrain(int system_server, int switch_id);
int TrainSystemReverse(int system_server, int train);
int TrainSystemGetTrainPosition(int system_server, int train);

#endif // __TRAIN_SYSTEM_INTERFACE_H__