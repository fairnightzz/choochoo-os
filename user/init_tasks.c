#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"
#include "kern/idle-perf.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"

void idleTask()
{
  for (;;)
  {
    call_wfi();
  }
}

void idlePerformanceTask()
{
  int clockServer = WhoIs(ClockAddress);
  for (;;)
  {
    PRINT("Idle Task Execution: %d%", idle_timer_percentage());
    Delay(clockServer, 1000);
  }
}