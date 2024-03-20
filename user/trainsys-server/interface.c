#include "interface.h"
#include "user/nameserver.h"
#include "user/ui/render.h"

int TrainSystemSetSpeed(int system_server, int train, int speed)
{
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_SET_SPEED,
      .train = train,
      .speed = speed,
      .light_status = false, // unused
      .sensor_hit = -1,      // unused
  };
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    render_command("TrainSystemSetSpeed ERRORED %d", ret);
    return -1;
  }
  return 0;
}

int TrainSystemSetLights(int system_server, int train, bool status)
{
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_SET_LIGHTS,
      .train = train,
      .speed = 0,       // unused
      .sensor_hit = -1, // unused
      .light_status = status};
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    LOG_WARN("TrainSystemSetLights ERRORED");
    return -1;
  }
  return 0;
}

uint8_t TrainSystemGetTrainState(int system_server, int train)
{
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_GET_TRAIN,
      .train = train,
      .speed = 0,           // unused
      .sensor_hit = -1,     // unused
      .light_status = false // unused
  };
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    LOG_WARN("TrainSystemSetLights ERRORED");
    return -1;
  }
  return response.train_state;
}

TrainSystemResponse TrainSystemSensorToTrain(int system_server, int sensor_id)
{
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_SENSOR_HIT,
      .train = 0,            // unused
      .speed = 0,            // unused
      .light_status = false, // unused
      .sensor_hit = sensor_id,
  };
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    LOG_WARN("TrainSystemSensorHit ERRORED");
  }
  return response;
}

void TrainSystemSwitchToTrain(int system_server, int switch_id)
{
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_SWITCH_CHANGE,
      .train = 0,            // unused
      .speed = 0,            // unused
      .light_status = false, // unused
      .sensor_hit = -1,      // unused
      .switch_triggered = switch_id};
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    LOG_WARN("TrainSystemSwitchChange ERRORED");
  }
}

int TrainSystemReverse(int trainstate_server, int train)
{
  if (!(1 <= train && train <= 100))
  {
    LOG_WARN("invalid train number %d", train);
    return -1;
  }

  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_REVERSE,
      .train = train,
  };
  int ret = Send(trainstate_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (response.was_already_reversing)
  {
    render_command("Train %d was already reversing", train);
    return -1;
  }
  if (ret < 0)
  {
    LOG_WARN("TrainstateReverse errored");
    return -1;
  }
  return 0;
}

int TrainSystemGetTrainPosition(int system_server, int train)
{
  if (!(1 <= train && train <= 100))
  {
    LOG_WARN("invalid train number %d", train);
    return -1;
  }

  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest){
      .type = SYSTEM_GET_TRAIN_POSITION,
      .train = train,
  };
  int ret = Send(system_server, (const char *)&request, sizeof(TrainSystemRequest), (char *)&response, sizeof(TrainSystemResponse));
  if (ret < 0)
  {
    LOG_WARN("TrainstateReverse errored");
    return -1;
  }
  return response.next_sensor_id;
}
