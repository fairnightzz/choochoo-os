#include <stddef.h>
#include <stdbool.h>

#include "addrspace.h"
#include "rpi.h"
#include "stdlib.h"

typedef struct PageTable PageTable;

static PageTable pagetable;

struct PageTable
{
    bool in_use[1024]; // tells each page if in use
};

addrspace
addrspace_new(address base)
{
    return (addrspace){
        .base = base,
        .stackbase = base + USER_ADDRSPACE_SIZE,
    };
}

void pagetable_init()
{
    pagetable = (PageTable){
        .in_use = {0}};
}

addrspace pagetable_createpage()
{
    for (unsigned int i = 0; i < 1024; ++i)
    {
        if (pagetable.in_use[i] == 0)
        {
            pagetable.in_use[i] = true;

            address base = USER_BASE + USER_ADDRSPACE_SIZE * i;
            return addrspace_new(base);
        }
    }
    LOG_ERROR("Out of space in page table");

    return (addrspace){
        .base = 0,
        .stackbase = 0,
    };
}

void pagetable_deletepage(addrspace *addrspace)
{
    size_t index = ((uint64_t)addrspace->base - (uint64_t)&pagetable) / USER_ADDRSPACE_SIZE;
    pagetable.in_use[index] = false;
}
