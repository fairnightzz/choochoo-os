#ifndef __KALLOC_H__
#define __KALLOC_H__ 1

#include <stddef.h>

typedef enum
{
  TASK = 0,
  SWITCH_FRAME = 1,
  SCHEDULER_NODE = 2,
  SEND_BUFFER = 3,
  RECEIVE_BUFFER = 4,
  COUNT = 5,
} AllocationType;

void slab_init();
void *slab_alloc(AllocationType at);
void slab_free(void *ptr, AllocationType at);
void slab_set_block_size(AllocationType at, size_t block_size);

#endif // __KALLOC_H__
