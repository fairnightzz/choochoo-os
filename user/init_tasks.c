#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"
#include "kern/idle-perf.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"
#include "clock-server/server.h"
#include "io-server/server.h"

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

// From K4 onwards, this will create everything
void initTask()
{
  // Initialize nameserver
  NameServerTaskInit(); // todo: consider making it like clockserver

  // Initialize clockserver
  Create(2, &ClockServer);

  // For printing performance idle
  Create(15, &idlePerformanceTask);

  // Console IO Server
  Create(5, &ConsoleIOServer);

  // Marklin IO Server
  Create(5, &MarklinIOServer);

  // UI Tasks
}