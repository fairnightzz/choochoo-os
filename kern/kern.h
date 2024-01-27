#ifndef __KERN_H__
#define __KERN_H__ 1

#include <stdint.h>
#include "task_descriptor.h"

typedef enum
{
    CREATE = 0,
    MY_TID = 1,
    MY_PARENT_TID = 2,
    YIELD = 3,
    EXIT = 4
} opcode;

void kernel_init();

int svc_create(uint32_t priority, void (*entrypoint)());
void handle_svc();
void svc_yield_first();

extern void vector_table_initialize();

#endif // __KERN_H__
