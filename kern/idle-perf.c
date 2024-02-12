#include "idle-perf.h"
#include "timer.h"

static uint64_t start_time;
static uint64_t total_idle_time;
static uint64_t last_idle_time;
static int idle_tid;

void idle_timer_init(int idleTid)
{
  idle_tid = idleTid;
  start_time = timer_get();
  total_idle_time = 0;

  // doesn't matter
  last_idle_time = 0;
}
void idle_timer_start()
{
  last_idle_time = timer_get();
}

void idle_timer_stop()
{
  total_idle_time += timer_get() - last_idle_time;
}

int idle_timer_percentage()
{
  int total_time = timer_get() - start_time;
  return total_idle_time * 100 / total_time;
}

void idle_timer_start_logic(int tid)
{
  if (tid == idle_tid)
  {
    idle_timer_start();
  }
}

void idle_timer_stop_logic(int tid)
{
  if (tid == idle_tid)
  {
    idle_timer_stop();
  }
}
