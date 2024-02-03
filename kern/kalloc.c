#include <stdbool.h>
#include <stdint.h>
#include "kalloc.h"
#include "stdlib.h"

#define SLAB_ALLOCATOR_SIZE 10240

typedef struct SlabPartition SlabPartition;
typedef struct SlabAllocator SlabAllocator;

static SlabAllocator salloc;

struct SlabPartition
{
  size_t block_size;
  size_t cursor;
  unsigned char buf[SLAB_ALLOCATOR_SIZE];
  bool in_use[SLAB_ALLOCATOR_SIZE];
  unsigned int allocated;
};

struct SlabAllocator
{
  SlabPartition slabs[COUNT];
};

void slab_init()
{
  salloc = (SlabAllocator){
      .slabs = {{0}},
  };
}

void slab_set_block_size(AllocationType at, size_t block_size)
{
  if (at == COUNT)
  {
    LOG_WARN("INVALID ALLOCATION TYPE");
    return;
  }
  salloc.slabs[at].block_size = block_size;
}

void *slab_alloc(AllocationType at)
{
  if (at == COUNT)
  {
    LOG_WARN("INVALID ALLOCATION TYPE");
    return 0;
  }

  SlabPartition *sb = &salloc.slabs[at];

  for (int i = 0; i < SLAB_ALLOCATOR_SIZE; i += sb->block_size)
  {
    if (sb->in_use[i] == 0 && i + sb->block_size < SLAB_ALLOCATOR_SIZE)
    {
      sb->in_use[i] = 1;
      void *ptr = sb->buf + i;
      return ptr;
    }
  }

  LOG_WARN("slab allocator is out of memory");
  return 0;
}

void slab_free(void *ptr, AllocationType at)
{
  unsigned char *p = (unsigned char *)ptr; // Cast to (unsigned char*) for pointer arithmetic.
  size_t offset = p - salloc.slabs[at].buf;
  if (offset < SLAB_ALLOCATOR_SIZE && salloc.slabs[at].in_use[offset] == 1)
  {
    salloc.slabs[at].in_use[offset] = 0;
    return;
  }
  LOG_WARN("Attempt to free an unallocated or out-of-bounds slot");
}
