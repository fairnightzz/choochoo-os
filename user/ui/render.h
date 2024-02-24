#ifndef __RENDER_H__
#define __RENDER_H__

#include "stdint.h"
#include "lib/stdlib.h"
#include "perf_timing.h"

static const unsigned int CONSOLE_MAX_LINES = 16;
static const unsigned int SENSOR_MAX_ENTRIES = 35;

typedef struct
{
  int cmd_line_history;
  unsigned int sensor_count;
} TermUIState;

void ui_init();
void render_time(uint64_t time);
void render_command(string* line);
void render_prompt(string* line);
void render_switch(int32_t switch_id, SwitchMode switch_mode);
void render_sensor(char bank, unsigned int sensor_number);
void render_perf_stats(PerfTimingState *ptime);
/*
void update_switch(const char* prompt);
void clear_console(uint32_t switch_id, char mode);
void clear_sensors_view(void);
*/
#endif // __RENDER_H__
