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
    Delay(clockServer, 500);
    PRINT("Idle Task Execution: %d percent", idle_timer_percentage());
  }
}