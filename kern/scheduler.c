#include "scheduler.h"
#include "stdlib.h"
#include "kalloc.h"

static SchedulerNode *mlfq[NUM_PRIORITY_LEVELS];

void scheduler_init()
{
  slab_set_block_size(SCHEDULER_NODE, sizeof(SchedulerNode));
  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    mlfq[i] = 0;
  }
}

int scheduler_add_task(uint32_t tid, uint8_t priority)
{
  LOG_WARN("inserting task id %d, with priority %d", tid, priority);

  if (priority >= NUM_PRIORITY_LEVELS)
  {
    LOG_ERROR("invalid priority");
    return -1;
  }

  SchedulerNode *task = slab_alloc(SCHEDULER_NODE);
  task->tid = tid;
  task->priority = priority;
  task->next = 0;

  // If no tasks in current priority, add it
  if (mlfq[priority] == 0)
  {
    mlfq[priority] = task;
  }
  else
  {
    // Add task to end of priority queue
    SchedulerNode *current = mlfq[priority];
    while (current->next != 0)
    {
      current = current->next;
    }
    current->next = task;
  }

  return 0;
}

uint32_t scheduler_next_task(void)
{
  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    if (mlfq[i] != 0)
    {
      SchedulerNode *next_task = mlfq[i];
      mlfq[i] = mlfq[i]->next;
      // Add task to the end

      // Add task to end of priority queue
      SchedulerNode *current = mlfq[i];
      while (current->next != 0)
      {
        current = current->next;
      }
      current->next = next_task;

      return next_task->tid;
    }
  }
  LOG_WARN("no next task because scheduler is empty");
  return 0;
}

void scheduler_delete_task(uint32_t tid)
{
  LOG_DEBUG("removing task id %d", tid);

  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    SchedulerNode *previous = 0;
    SchedulerNode *current = mlfq[i];
    while (current != 0)
    {
      if (current->tid == tid)
      {
        if (previous)
        {
          previous->next = current->next;
        }
        else
        {
          mlfq[i] = current->next;
        }
        slab_free(current, SCHEDULER_NODE);
        return;
      }
      previous = current;
      current = current->next;
    }
  }

  LOG_DEBUG("could not find task id %d in scheduler", tid);
}
