#ifndef __CLOCK_INTERFACE_H__
#define __CLOCK_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define TrainSystemAddress "CHOOCHOO-TRAIN-SYSTEM"

typedef enum
{
  SYSTEM_SET_SPEED = 1,
  SYSTEM_SET_LIGHTS,
  SYSTEM_GET_TRAIN
} TrainSystemRequestType;

typedef struct
{
  TrainSystemRequestType type;
  int train;
  int speed;
  bool light_status;
} TrainSystemRequest;

typedef struct
{
  TrainSystemRequestType type;
  uint8_t train_state;
} TrainSystemResponse;

int TrainSystemSetSpeed(int system_server, int train, int speed);
int TrainSystemSetLights(int system_server, int train, bool status);
uint8_t TrainSystemGetTrainState(int system_server, int train);

#endif // __CLOCK_INTERFACE_H__