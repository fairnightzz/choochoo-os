#ifndef __KALLOC_H__
#define __KALLOC_H__ 1

#include <stddef.h>

typedef enum
{
  TASK = 0,
  SWITCH_FRAME = 1,
  SCHEDULER_NODE = 2,
  SEND_STATE = 3,
  RECEIVE_STATE = 4,
  LLIST = 5,
  LLIST_NODE = 6,
  LLIST_ITERATOR = 7,
  HASHMAP = 8,
  HASHMAP_BUCKETS = 9,
  HASHMAP_NODE = 10,
  RPS_STATE = 11,
  STRING = 12,
  COUNT = 13,
} AllocationType;

void slab_init();
void *slab_alloc(AllocationType at);
void slab_free(void *ptr, AllocationType at);
void slab_set_block_size(AllocationType at, size_t block_size);

#endif // __KALLOC_H__
