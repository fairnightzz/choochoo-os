#ifndef __ALLOC_H__
#define __ALLOC_H__ 1

typedef enum
{
  LINKED_LIST = 5,
  LINKED_LIST_NODE = 6,
  LINKED_LIST_ITERATOR = 7,
  HASH_MAP = 8,
  HASH_MAP_BUCKETS = 9,
  HASH_MAP_NODE = 10,
} UserAllocationType;

void alloc_init(UserAllocationType uat, size_t size);

void*
alloc(UserAllocationType uat);

void
free(void *ptr, UserAllocationType uat);

#endif /* __ALLOC_H__ */