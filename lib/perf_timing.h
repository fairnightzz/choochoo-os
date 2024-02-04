#ifndef __PERF_TIMING_H__
#define __PERF_TIMING_H__ 1

#include <stdint.h>

typedef enum {
  SSR_TIME = 0,
  TYPE_COUNT = 1,
} TimingType;

typedef struct {
  int64_t start;
  int64_t end;
  int64_t perf_time;
  int64_t max_time;
  int64_t sum_time;
  unsigned int count_gets;
} TimingState;

typedef struct {
  TimingState timers[TYPE_COUNT];
} PerfTimingState;

PerfTimingState new_perf_timing_state(void);
void set_start(PerfTimingState *ptime, TimingType type);
void set_end(PerfTimingState *ptime, TimingType type);
int64_t get_perf_time(PerfTimingState *ptime, TimingType type);
int64_t get_avg_perf_time(PerfTimingState *ptime, TimingType type);
int64_t get_max_perf_time(PerfTimingState *ptime, TimingType type);

#endif // __PERF_TIMING_H__
