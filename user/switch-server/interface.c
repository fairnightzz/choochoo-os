#include "interface.h"

int SwitchSet(int switch_server, int switch_id, SwitchMode switch_mode) {
  SwitchResponse response;
  SwitchRequest request = (SwitchRequest) {
    .type = SWITCH_SET,
    .switch_id = switch_id,
    .mode = switch_mode
  };
  int ret = Send(switch_server, (const char*)&request, sizeof(SwitchRequest), (char*)&response, sizeof(SwitchResponse));
  if (ret < 0) {
      LOG_WARN("[SwitchSet ERROR]: return value - %d", ret);
      return -1;
  }
  return 0;
}

SwitchMode SwitchGet(int switch_server, int switch_id) {
  SwitchResponse response;
  SwitchRequest request = (SwitchRequest) {
    .type = SWITCH_GET,
    .switch_id = switch_id,
    .mode = SWITCH_MODE_UNKNOWN
  };
  int ret = Send(switch_server, (const char*)&request, sizeof(SwitchRequest), (char*)&response, sizeof(SwitchResponse));
  if (ret < 0) {
      LOG_WARN("[SwitchGet ERROR]: return value - %d", ret);
      return SWITCH_MODE_UNKNOWN;
  }
  return response.mode; 
}

SwitchResponse WaitOnSwitchChange(int switch_server, int switch_id) {
  SwitchResponse response;
  SwitchRequest request = (SwitchRequest) {
    .type = SWITCH_WAIT,
    .switch_id = switch_id,
    .mode = SWITCH_MODE_UNKNOWN
  };
  int ret = Send(switch_server, (const char*)&request, sizeof(SwitchRequest), (char*)&response, sizeof(SwitchResponse));
  if (ret < 0) {
      LOG_WARN("[WaitOnSwitchChange ERROR]: return value - %d", ret);
      return (SwitchResponse) {
        .type = SWITCH_WAIT,
        .switch_id = -1,
        .mode = SWITCH_MODE_UNKNOWN
      };
  }
  return (SwitchResponse) {
    .type = SWITCH_WAIT,
    .switch_id = response.switch_id,
    .mode = response.mode
  };
} 
