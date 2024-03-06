#include "interface.h"

int WaitOnSensor(int sensor_server, int sensor_id) {
  SensorResponse response;
  SensorRequest request = (SensorRequest) {
    .type = SENSOR_WAITING,
    .id_wait = sensor_id,
    .ids_triggered = {0},
  };
  int ret = Send(sensor_server, (const char *)&request, sizeof(request), (char *)&response, sizeof(response));
  if (ret < 0) {
    LOG_WARN("[WaitOnSensor]: send errored");
  }
  return response.triggered;
}