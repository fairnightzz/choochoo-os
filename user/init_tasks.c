#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"
#include "kern/idle-perf.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"
#include "clock-server/server.h"
#include "io-server/server.h"
#include "ui/interface.h"

void idleTask()
{
  for (;;)
  {
    call_wfi();
  }
}

// From K4 onwards, this will create everything
void initTask()
{
  // Initialize nameserver
  NameServerTaskInit(); // todo: consider making it like clockserver

  // Initialize clockserver
  Create(2, &ClockServer);

  // Console IO Server
  Create(5, &ConsoleIOServer);

  // Marklin IO Server
  Create(5, &MarklinIOServer);

  // UI Task
  Create(5, &UITask);
}