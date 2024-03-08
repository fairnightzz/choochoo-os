#include "interface.h"
#include "user/clock-server/interface.h"
#include "user/nameserver.h"
#include "kern/idle-perf.h"
#include "render.h"
#include "helper_tasks.h"
#include "user/trainsys/trainsys.h"
#include "user/switch-server/interface.h"
#include "user/sensor-server/interface.h"
#include "user/ui/predict_task.h"

void idlePerformanceTask()
{
  int clockServer = WhoIs(ClockAddress);
  for (;;)
  {
    Delay(clockServer, 500);
    if (trainsys_exited())
    {
      break;
    }
    render_perf_stats(idle_timer_percentage());
  }
}

void renderSwitchTask()
{
  int switch_server = WhoIs(SwitchAddress);
  int clock_server = WhoIs(ClockAddress);

  // Inital drawing of tasks screws things up
  Delay(clock_server, 20);

  // While loop
  for (;;)
  {
    SwitchResponse response;
    response = WaitOnSwitchChange(switch_server, -1); // switch_id = -1 means any switch
    render_switch(response.switch_id, response.mode);
  }
}

void renderSensorTask()
{
  int sensor_server = WhoIs(SensorAddress);
  int clock_server = WhoIs(ClockAddress);

  // Inital drawing of sensors screws things up
  Delay(clock_server, 20);

  // While loop
  for (;;)
  {
    Delay(clock_server, 100);
    // SensorGetRecentResponse response;
    // SensorRequest request = (SensorRequest){
    //     .type = SENSOR_GET_RECENT,
    //     .id_wait = -1,
    //     .ids_triggered = {0},
    // };

    for (;;)
    {
      int sensor_id;
      sensor_id = WaitOnSensor(sensor_server, -1); // switch_id = -1 means any switch
      int sensor_group = sensor_id / 16;
      int sensor_index = (sensor_id % 16) + 1;
      render_sensor(sensor_group + 'A', sensor_index);
    }
    // int ret = Send(sensor_server, (const char *)&request, sizeof(request), (char *)&response, sizeof(response));
    // if (ret < 0)
    // {
    //   LOG_WARN("[GetREcentSensor]: send errored");
    // }

    // for (int i = 0; i < 16; i++)
    // {
    //   if (response.ids_triggered[i] == -1)
    //   {
    //     break;
    //   }
    //   int sensor_id = response.ids_triggered[i];
    //   uint8_t sensor_group = sensor_id / 16;
    //   uint8_t sensor_index = (sensor_id % 16) + 1;
    //   render_sensor(sensor_group + 'A', sensor_index);
    // }
  }
}

void UITask()
{
  render_init();

  trainsys_init();

  // clock update on ui
  Create(2, &clockUITask);

  // Render the sensor and switch tasks
  Create(3, &renderSwitchTask);
  Create(3, &renderSensorTask);

  // Prompt task
  Create(2, &promptTask);
  Create(4, &predictTask);

  // For printing performance idle
  Create(15, &idlePerformanceTask);
}