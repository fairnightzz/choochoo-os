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
#include "user/traindata/train_data.h"
#include "user/zone-server/server.h"
#include "user/ui/render.h"

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
  Create(1, &ClockServer);

  // Console IO Server
  Create(2, &ConsoleIOServer);

  // Marklin IO Server
  int marklin_server = Create(2, &MarklinIOServer);

  io_marklin_init(marklin_server);
  train_data_init('A');
  render_init();


  Create(2, &SensorServer);
  Create(2, &SwitchServer);

  Create(4, &ZoneServer);
  Create(4, &TrainSystemServer);
  // Create(4, &PathFinderServer);

  // UI Task
  Create(4, &UITask);
  // Create(5, &putcTestTask);
}