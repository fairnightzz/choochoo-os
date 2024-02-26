#include "interface.h"
#include "user/clock-server/interface.h"
#include "user/nameserver.h"
#include "kern/idle-perf.h"
#include "render.h"
#include "helper_tasks.h"
#include "user/trainsys/trainsys.h"

void idlePerformanceTask()
{
  int clockServer = WhoIs(ClockAddress);
  for (;;)
  {
    Delay(clockServer, 500);
    render_perf_stats(idle_timer_percentage());
  }
}

void UITask()
{
  render_init();

  // For printing performance idle
  Create(15, &idlePerformanceTask);

  trainsys_init();

  // Prompt task
  Create(2, &promptTask);

  // Train sensors task
  Create(2, &trainsysTask);
}