#ifndef __TASK_DESCRIPTOR_H__
#define __TASK_DESCRIPTOR_H__ 1

#include <stdint.h>
#include <stddef.h>
#include "addrspace.h"
#include "switchframe.h"
#include "stdlib.h"

#define TASK_SIZE 128

typedef enum
{
  RUNNING,
  READY,
  FINISHED,
  ERROR,
  SEND_WAIT,
  RECEIVE_WAIT,
  REPLY_WAIT,
} TaskStatus;

// SRR Buffers

typedef struct
{
  char *reply_buffer;
  size_t reply_buffer_len;

  char *send_buffer;
  size_t send_buffer_len;
} SendState;

typedef struct
{
  char *receive_buffer;
  size_t receive_buffer_len;

  int *sender_tid;
} ReceiveState;

typedef struct
{
  SwitchFrame *switch_frame;
  int tid;
  int pTid;
  TaskStatus status;
  uint32_t pri;
  addrspace addrspace;

  BQueue *send_listeners_queue;
  SendState *send_state;
  ReceiveState *receive_state;

} TaskDescriptor;

struct TaskList
{
  int next_tid;
  TaskDescriptor *tasks[128]; // Max Task Count
};

void task_init();
int create_task(uint32_t pri, void (*entrypoint)());
TaskDescriptor *get_task(int tid);
void set_task_status(TaskDescriptor *task, TaskStatus status);
void set_current_task(int tid);
int get_current_task_id();
TaskDescriptor *get_current_task();
void delete_task(int tid);

#endif