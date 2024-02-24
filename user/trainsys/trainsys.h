#ifndef __TRAINSYS_H__
#define __TRAINSYS_H__

#include "lib/stdlib.h"
#include <stdbool.h>
#include <stdint.h>

#define TRAINS_COUNT         81
#define M_WRITE              10
#define SENSOR_READ          1
#define SENSOR_BYTE_READ     1
#define TRAIN_SPEED_MASK     0xF
#define TRAIN_LIGHTS_MASK    0x10
#define REV_STOP_DELAY       150
#define REV_DELAY            50

typedef struct {
  BQueue serial_out;
  int train_state[TRAINS_COUNT];
  
  int stop_times[TRAINS_COUNT];
  int rev_times[TRAINS_COUNT];
  unsigned int trains_reversing;
  
  int switch_states[SWITCH_COUNT]; // indexing by switich id, S = 33, C = 34

  int last_serial_write;

  int last_sensor_read;
  int last_sensor_byte_read;
  int read_sensor_bytes;
  bool sensor_states[80]; // 80 Sensors 16 * 5

  int marklin_tid;
} TrainSystemState;

void trainsys_init(int marklin_tid);
void trainsys_init_track(TrackSwitchPlans track_plan, int curr_tick);
void trainsys_try_serial_out(int curr_tick);
void trainsys_execute_command(CommandResult cres, int curr_tick);
void trainsys_check_rev_trains(int curr_tick);
void trainsys_read_all_sensors(int curr_tick);

#endif // __TRAINSYS_H__
