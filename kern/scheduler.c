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

int scheduler_add_task(int tid, uint8_t priority)
{
  if (priority >= NUM_PRIORITY_LEVELS)
  {
    LOG_ERROR("Invalid priority, should never reach this stage");
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

// Return task coutn for a given level of the queue
uint8_t scheduler_count_tasks(SchedulerNode *node)
{
  uint8_t count = 0;
  while (node != 0)
  {
    ++count;
    node = node->next;
  }
  return count;
}

int scheduler_next_task()
{
  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    for (uint8_t j = 0; j < scheduler_count_tasks(mlfq[i]); j++)
    {
      if (mlfq[i] != 0)
      {
        SchedulerNode *nextTask = mlfq[i];

        // Move next task forward
        mlfq[i] = mlfq[i]->next;

        // Add the next task to the end of the priority list
        // Reach the end of the linked list and insert the task there
        if (mlfq[i] == 0)
        {
          mlfq[i] = nextTask;
        }
        else
        {
          SchedulerNode *current = mlfq[i];
          for (;;)
          {
            if (current->next == 0)
            {
              current->next = nextTask;
              break;
            }
            current = current->next;
          }
        }
        // Set the linked list tail
        nextTask->next = 0;
        TaskStatus task_status = get_task(nextTask->tid)->status;
        if (task_status == READY || task_status == RUNNING)
          return nextTask->tid;
      }
    }
  }
  LOG_WARN("No next task");
  return 0;
}

void scheduler_delete_task(int tid)
{
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
  LOG_ERROR("Could not find task %d in scheduler", tid);
}

void scheduler_unblock_events(EventType eventType)
{
  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    SchedulerNode *current = mlfq[i];
    for (uint8_t j = 0; j < scheduler_count_tasks(mlfq[i]); j++)
    {
      TaskDescriptor *task = get_task(current->tid);
      if (task->status == EVENT_WAIT && task->eventWaitType == eventType)
      {
        // unblock
        task->status = READY;
        task->eventWaitType = EVENT_NONE;
      }
      current = current->next;
    }
  }
}

void scheduler_unblock_event(EventType eventType, int exitedTaskTid)
{
  for (uint8_t i = 0; i < NUM_PRIORITY_LEVELS; i++)
  {
    SchedulerNode *current = mlfq[i];
    for (uint8_t j = 0; j < scheduler_count_tasks(mlfq[i]); j++)
    {
      TaskDescriptor *task = get_task(current->tid);
      if (task->status == EVENT_WAIT && task->eventWaitType == eventType)
      {
        // unblock
        task->status = READY;
        task->eventWaitType = EVENT_NONE;
        task->switch_frame->x0 = exitedTaskTid;
      }
      current = current->next;
    }
  }
}