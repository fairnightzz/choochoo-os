#include "perf_timing.h"
#include "kern/timer.h"

PerfTimingState new_perf_timing_state(void) {
  PerfTimingState new_state = {0};
  return new_state;
}

void set_start(PerfTimingState *ptime, TimingType type) {
  ptime->timers[type].start = timer_get();
}
void set_end(PerfTimingState *ptime, TimingType type) {
  ptime->timers[type].end = timer_get();
  ptime->timers[type].perf_time = ptime->timers[type].end  - ptime->timers[type].start;
  if (ptime->timers[type].perf_time > ptime->timers[type].max_time) {
    ptime->timers[type].max_time = ptime->timers[type].perf_time;
  }
}

int64_t get_perf_time(PerfTimingState *ptime, TimingType type) {
  ptime->timers[type].sum_time += ptime->timers[type].perf_time;
  ptime->timers[type].count_gets += 1;
  return ptime->timers[type].perf_time;
}

int64_t get_avg_perf_time(PerfTimingState *ptime, TimingType type) {
  return ptime->timers[type].sum_time / ptime->timers[type].count_gets;
}

int64_t get_max_perf_time(PerfTimingState *ptime, TimingType type) {
  return ptime->timers[type].max_time;
}
