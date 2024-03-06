#ifndef __SWITCH_INTERFACE_H__
#define __SWITCH_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define SwitchAddress "CLICK-CLICK"
#define SWITCH_COUNT 22

typedef enum
{
  SWITCH_SET = 1,
  SWITCH_GET,
  SWITCH_WAIT
} SwitchRequestType;

typedef struct
{
  SwitchRequestType type;
  int switch_id;
  SwitchMode mode; // used for SWITCH_SET
} SwitchRequest;

typedef struct
{
  SwitchRequestType type;
  int switch_id;
  SwitchMode mode;
} SwitchResponse;

typedef struct
{
  int tid;
  int switch_id;
} SwitchBufferRequest;

int SwitchSet(int switch_server, int switch_id, SwitchMode switch_mode);
SwitchMode SwitchGet(int switch_server, int switch_id);
SwitchResponse WaitOnSwitchChange(int switch_server, int switch_id); // switch_id = -1 means any switch

#endif // __SWITCH_INTERFACE_H__