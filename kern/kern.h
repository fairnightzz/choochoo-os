#ifndef __KERN_H__
#define __KERN_H__ 1

#include <stdint.h>

typedef enum
{
    CREATE = 0,
    MY_TID = 1,
    MY_PARENT_TID = 2,
    YIELD = 3,
    EXIT = 4
} opcode;

void init_kernel();

void handle_svc();
uint32_t svc_create(uint32_t priority, void (*entrypoint)());

extern void vector_table_initialize();

#endif // __KERN_H__
