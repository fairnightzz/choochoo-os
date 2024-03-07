#ifndef __SENSOR_INTERFACE_H__
#define __SENSOR_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define SensorAddress "SENSOR-SERVER"

typedef enum
{
  SENSOR_TRIGGERED = 1,  // from notifier
  SENSOR_GET_RECENT = 2, // from user
  SENSOR_WAITING         // from user
} SensorRequestType;

typedef struct
{
  SensorRequestType type;
  int ids_triggered[17]; // -1 terminated
  int id_wait;
} SensorRequest;

typedef struct
{
  SensorRequestType type;
  int triggered;
} SensorResponse;

typedef struct
{
  int ids_triggered[9]; // -1 terminated
} SensorGetRecentResponse;

typedef struct
{
  int tid;
  int sensor_id;
} SensorBufferRequest;

int WaitOnSensor(int sensor_server, int sensor_id);

#endif // __SENSOR_INTERFACE_H__