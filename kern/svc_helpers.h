#ifndef __SVC_HELPERS_H__
#define __SVC_HELPERS_H__ 1

#include <stdint.h>
#include "task_descriptor.h"

int svc_create(uint32_t priority, void (*entrypoint)());

void svc_yield(TaskDescriptor *curr_task);

void svc_yield_first();

int svc_send(int tid, const char *msg, int msglen, char *reply, int rplen, TaskDescriptor *curr_task);

void svc_receive(int *tid, char *msg, int msg_len, TaskDescriptor *curr_task);

int svc_reply(int tid, const char *reply, int rplen);

extern uint32_t get_esr_el1(void);

#endif // __KERN_H__
