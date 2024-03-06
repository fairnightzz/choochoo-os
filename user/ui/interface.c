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
    if (trainsys_exited()) {
      break;
    }
    render_perf_stats(idle_timer_percentage());
  }
}

void UITask()
{
  render_init();

  // For printing performance idle
  Create(15, &idlePerformanceTask);

  trainsys_init();

  // clock update on ui
  Create(2, &clockUITask);

  // Prompt task
  Create(2, &promptTask);

}