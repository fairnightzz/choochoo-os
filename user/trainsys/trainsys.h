#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

#include "lib/stdlib.h"
#include <stdbool.h>
#include <stdint.h>
#include "user/traintrack/track_data.h"

#define TRAINS_COUNT 81
#define M_WRITE 10
#define SENSOR_READ 1
#define SENSOR_BYTE_READ 1
#define TRAIN_SPEED_MASK 0xF
#define TRAIN_LIGHTS_MASK 0x10
#define REV_STOP_DELAY 300
#define REV_DELAY 100

typedef struct
{
  int system_tid;
  int clock_tid;
  int switch_tid;
  int pathfinder_tid;
  bool exited;
  uint32_t moving_train;
} TrainSystemState;

void trainsys_init();
void trainsys_init_track(TrackSwitchPlans track_plan);
void trainsys_execute_command(CommandResult cres);
uint32_t trainsys_get_moving_train();
bool trainsys_exited();

#endif // __TRAINSYS_H__
