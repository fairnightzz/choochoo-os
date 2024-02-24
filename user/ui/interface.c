#include "interface.h"
#include "user/clock-server/interface.h"
#include "user/nameserver.h"
#include "kern/idle-perf.h"
#include "render.h"

void idlePerformanceTask()
{
  int clockServer = WhoIs(ClockAddress);
  for (;;)
  {
    Delay(clockServer, 100);
    render_perf_stats(idle_timer_percentage());
  }
}

void UITask()
{
  // For printing performance idle
  Create(15, &idlePerformanceTask);

  // Prompt task
  Create(2, &promptTask);

  // Train sensors task
  Create(2, &sensorTask);
}