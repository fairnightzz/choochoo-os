#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "task_descriptor.h"

#define NUM_PRIORITY_LEVELS 32

typedef struct SchedulerNode SchedulerNode;

struct SchedulerNode
{
  int tid;
  uint8_t priority;
  SchedulerNode *next;
};

void scheduler_init();
int scheduler_add_task(int tid, uint8_t priority); // Returns -1 if the priority is invalid
int scheduler_next_task();                         // return 0 if no tasks
void scheduler_delete_task(int tid);
void scheduler_unblock_events(EventType eventType);

#endif // __SCHEDULER_H__