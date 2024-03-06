#ifndef __MARKLIN_H__
#define __MARKLIN_H__

#include <stdint.h>
#include <stdbool.h>

// enums set to the straight and curved commands
typedef enum
{
  SWITCH_STRAIGHT = 33,
  SWITCH_CURVED = 34,
} SwitchMode;

// Track A hardcoded switches for initialization
static const SwitchMode TRACK_PLANS[1][23] = {
    {SWITCH_STRAIGHT, SWITCH_STRAIGHT, SWITCH_STRAIGHT, SWITCH_STRAIGHT, SWITCH_CURVED, SWITCH_STRAIGHT,
     SWITCH_STRAIGHT, SWITCH_CURVED, SWITCH_CURVED, SWITCH_STRAIGHT, SWITCH_CURVED, SWITCH_CURVED,
     SWITCH_STRAIGHT, SWITCH_CURVED, SWITCH_CURVED, SWITCH_STRAIGHT, SWITCH_STRAIGHT, SWITCH_CURVED,
     SWITCH_STRAIGHT, SWITCH_CURVED, SWITCH_STRAIGHT, SWITCH_CURVED}};

// 10 bytes
typedef struct Sensors
{
  uint8_t data[10]; // A: 0, 1 B: 2, 3 C: 4, 5 D: 6, 7 E: 8, 9
} Sensors;

Sensors make_sensor();

void marklin_train_control(uint8_t info, uint8_t train);
void marklin_switch_control(SwitchMode mode, uint8_t switch_num);
void marklin_dump_s88_all();
void marklin_pick_s88(uint32_t index);

#endif // __MARKLIN_H__
