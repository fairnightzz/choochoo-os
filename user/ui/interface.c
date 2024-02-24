#include "interface.h"
#include "user/clock-server/interface.h"

void idlePerformanceTask()
{
  int clockServer = WhoIs(ClockAddress);
  int renderServer = WhoIs(RenderAddress);
  for (;;)
  {
    Delay(clockServer, 100);
    // Replace this with render server call
    PRINT("Idle Task Execution: %d percent", idle_timer_percentage());
  }
}

void UITask()
{
  // Render server
  Create(2, &renderServer);

  // For printing performance idle
  Create(15, &idlePerformanceTask);

  // Prompt task
  Create(2, &promptTask);

  // Train sensors task
  Create(2, &sensorTask);
}