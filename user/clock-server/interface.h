#ifndef __CLOCK_INTERFACE_H__
#define __CLOCK_INTERFACE_H__

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

#endif // __CLOCK_INTERFACE_H__