#ifndef __CLOCK_INTERFACE_H__
#define __CLOCK_INTERFACE_H__

#include <stdint.h>

#define ClockAddress "TICK-TOCK"

typedef enum
{
  CLOCK_TIME = 1,
  CLOCK_DELAY,
  CLOCK_DELAY_UNTIL,
  CLOCK_TICK,
} ClockMessageType;

typedef struct
{
  ClockMessageType type;
  int ticks;
} ClockRequest;

typedef struct
{
  ClockMessageType type;
  int ticks;
} ClockResponse;

typedef struct
{
  int tid;
  uint32_t absolute_tick_delay;
  ClockMessageType type;
} ClockBufferRequest;

int Time(int tid);
int Delay(int tid, int ticks);
int DelayUntil(int tid, int ticks);

#endif // __CLOCK_INTERFACE_H__