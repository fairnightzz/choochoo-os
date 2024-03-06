#ifndef __CLOCK_INTERFACE_H__
#define __CLOCK_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define ClockAddress "TICK-TOCK"

typedef enum
{
  SENSOR_TRIGGERED = 1, // from notifier
  SENSOR_WAITING // from user
} SensorRequestType;

typedef struct
{
  SensorRequestType type;
  int ids_triggered[9]; // -1 terminated
  int id_wait;
} SensorRequest;

typedef struct
{
  SensorRequestType type;
  int triggered;
} SensorResponse;

int WaitOnSensor(int sensor_server, int sensor_id);

#endif // __CLOCK_INTERFACE_H__