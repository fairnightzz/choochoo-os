#ifndef __IDLE_PERF_H__
#define __IDLE_PERF_H__
#include <stdint.h>

void idle_timer_init(int idleTid);
void idle_timer_start_logic();
void idle_timer_stop_logic();
int idle_timer_percentage();

#endif // __PERF_H__
