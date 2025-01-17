#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

#include "lib/stdlib.h"
#include <stdbool.h>
#include <stdint.h>
#include "user/traintrack/track_data.h"
#include "user/traindata/train_data.h"

#define M_WRITE 10
#define SENSOR_READ 1
#define SENSOR_BYTE_READ 1
#define SPEED_STOP 0
#define SPEED_REVERSE 15

typedef struct
{
  int system_tid;
  int clock_tid;
  int switch_tid;
  int rand_tid;
  int pacman_tid;
} TrainSystemState;

void trainsys_init();
void trainsys_init_track(TrackSwitchPlans track_plan);
void trainsys_init_trains();
void trainsys_execute_command(CommandResult cres);
void trainsys_start_rand_routing();
void trainsys_end_rand_routing();

#endif // __TRAINSYS_H__
