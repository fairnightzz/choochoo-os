#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task_descriptor.h"

#define NUM_PRIORITY_LEVELS 32

typedef struct SchedulerNode SchedulerNode;

struct SchedulerNode
{
  uint32_t tid;
  uint8_t priority;
  SchedulerNode *next;
};

void scheduler_init();
uint32_t scheduler_count();
int scheduler_add_task(uint32_t tid, uint8_t priority); // Returns -1 if the priority is invalid
uint32_t scheduler_next_task();
void scheduler_delete_task(uint32_t tid);

#endif // __SCHEDULER_H__