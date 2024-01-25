#ifndef __TASK_DESCRIPTOR_H__
#define __TASK_DESCRIPTOR_H__ 1

#include <stdint.h>
#include "addrspace.h"
#include "switchframe.h"

typedef enum {
  RUNNING,
  READY,
  FINISHED,
  ERROR
} TaskStatus;

typedef struct {
  SwitchFrame *switch_frame;
  uint32_t tid;
  uint32_t pTid;
  TaskStatus status;
  uint32_t pri;
  addrspace addrspace;
  address saved_sp;
  address saved_x30;
} TaskDescriptor;

struct TaskList {
    uint32_t next_tid;
    TaskDescriptor* tasks[128]; // Max Task Count
};

void task_init();
int32_t create_task(uint32_t pri, void (*entrypoint)());
TaskDescriptor* get_task(int32_t tid);

void set_current_task(int32_t tid);
void get_current_task_id(void);
void get_current_task(void);
void delete_task(int32_t tid);

#endif