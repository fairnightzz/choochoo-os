#include "interface.h"
#include "user/nameserver.h"


int TrainSystemSetSpeed(int system_server, int train, int speed) {
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest) {
    .type = SYSTEM_SET_SPEED,
    .train = train,
    .speed = speed,
    .light_status = false // unused
  };
  int ret = Send(system_server, (const char*)&request, sizeof(TrainSystemRequest), (char*)&response, sizeof(TrainSystemResponse));
    if (ret < 0) {
        LOG_WARN("TrainSystemSetSpeed ERRORED");
        return -1;
    }
    return 0;
}

int TrainSystemSetLights(int system_server, int train, bool status) {
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest) {
    .type = SYSTEM_SET_LIGHTS,
    .train = train,
    .speed = 0, // unused
    .light_status = status
  };
  int ret = Send(system_server, (const char*)&request, sizeof(TrainSystemRequest), (char*)&response, sizeof(TrainSystemResponse));
    if (ret < 0) {
        LOG_WARN("TrainSystemSetLights ERRORED");
        return -1;
    }
    return 0;
}

uint8_t TrainSystemGetTrainState(int system_server, int train) {
  TrainSystemResponse response;
  TrainSystemRequest request = (TrainSystemRequest) {
    .type = SYSTEM_GET_TRAIN,
    .train = train,
    .speed = 0, // unused
    .light_status = false // unused
  };
  int ret = Send(system_server, (const char*)&request, sizeof(TrainSystemRequest), (char*)&response, sizeof(TrainSystemResponse));
    if (ret < 0) {
        LOG_WARN("TrainSystemSetLights ERRORED");
        return -1;
    }
    return response.train_state;
}
