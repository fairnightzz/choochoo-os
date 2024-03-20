#include "interface.h"
#include "stdlib.h"
#include "user/ui/render.h"

// Returns time as ticks and so will every other call
int Time(int tid)
{
  ClockRequest request = (ClockRequest){
      .type = CLOCK_TIME,
  };
  ClockResponse response;

  int returnValue = Send(tid, (const char *)&request, sizeof(ClockRequest), (char *)&response, sizeof(ClockResponse));

  if (returnValue < 0 || response.type != CLOCK_TIME)
  {
    PRINT("Time request to clock server %d has errored.", tid);
    return -1;
  }

  return response.ticks;
}

int Delay(int tid, int ticks)
{
  if (ticks < 0)
  {
    return -2;
  }

  ClockRequest request = (ClockRequest){
      .type = CLOCK_DELAY,
      .ticks = ticks,
  };
  ClockResponse response;

  int returnValue = Send(tid, (const char *)&request, sizeof(ClockRequest), (char *)&response, sizeof(ClockResponse));

  if (returnValue < 0 || response.type != CLOCK_DELAY)
  {
    render_command("Delay request to clock server %d has errored: ret value: %d type %d", tid, returnValue, response.type);
    return -1;
  }

  return response.ticks;
}

int DelayUntil(int tid, int ticks)
{
  if (ticks < 0)
  {
    return -2;
  }

  ClockRequest request = (ClockRequest){
      .type = CLOCK_DELAY_UNTIL,
      .ticks = ticks,
  };
  ClockResponse response;

  int returnValue = Send(tid, (const char *)&request, sizeof(ClockRequest), (char *)&response, sizeof(ClockResponse));

  if (returnValue < 0 || response.type != CLOCK_DELAY_UNTIL)
  {
    PRINT("Delay Until request to clock server %d has errored.", tid);
    return -1;
  }

  return response.ticks;
}