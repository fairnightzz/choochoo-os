#ifndef __TASK_DESCRIPTOR_H__
#define __TASK_DESCRIPTOR_H__ 1

#include <stdint.h>
#include "addrspace.h"
#include "switchframe.h"

typedef enum
{
  RUNNING,
  READY,
  FINISHED,
  ERROR
} TaskStatus;

typedef struct
{
  SwitchFrame *switch_frame;
  int tid;
  int pTid;
  TaskStatus status;
  uint32_t pri;
  addrspace addrspace;
  address saved_sp;
  address saved_x30;
} TaskDescriptor;

struct TaskList
{
  int next_tid;
  TaskDescriptor *tasks[128]; // Max Task Count
};

void task_init();
int create_task(uint32_t pri, void (*entrypoint)());
TaskDescriptor *get_task(int tid);

void set_current_task(int tid);
int get_current_task_id();
TaskDescriptor *get_current_task();
void delete_task(int tid);

#endif