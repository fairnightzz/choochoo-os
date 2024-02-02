#include "kern/kalloc.h"
#include "alloc.h"

void alloc_init(UserAllocationType uat, size_t size) {
  slab_set_block_size((AllocationType)uat, size);
}

void*
alloc(UserAllocationType uat)
{
    return slab_alloc((AllocationType)uat);
}

void
free(void *ptr, UserAllocationType uat)
{
    slab_free(ptr, (AllocationType)uat);
}
