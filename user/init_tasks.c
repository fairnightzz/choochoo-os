#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"
#include "kern/idle-perf.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"
#include "clock-server/server.h"
#include "io-server/server.h"
#include "ui/interface.h"
#include "sensor-server/server.h"
#include "trainsys-server/server.h"
#include "switch-server/server.h"
#include "user/io-server/io_marklin.h"
#include "user/pathfinder-server/server.h"

void idleTask()
{
  for (;;)
  {
    call_wfi();
  }
}

void putcTestTask(void)
{
  int clock_server = WhoIs(ClockAddress);
  int io_server = WhoIs(MarklinIOAddress);
  Putc(io_server, 192);
  Putc(io_server, 26);
  Putc(io_server, 77);
  for (;;)
  {
    Putc(io_server, 133);
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    PRINT("DATA GOTTEN: %d", Getc(io_server));
    Delay(clock_server, 1000);
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
  int marklin_server = Create(5, &MarklinIOServer);

  io_marklin_init(marklin_server);

  Create(2, &SensorServer);
  Create(2, &SwitchServer);

  Create(3, &PathFinderServer);
  Create(2, &TrainSystemServer);

  // UI Task
  Create(5, &UITask);
  // Create(5, &putcTestTask);
}