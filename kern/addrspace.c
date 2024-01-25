#include <stddef.h>

#include "addrspace.h"
#include "rpi.h"
#include "stdlib.h"


typedef struct PageTable PageTable;

static PageTable pagetable;

struct PageTable {
    uint16_t entries[1024]; 
};

addrspace
addrspace_new(address base)
{
    return (addrspace) {
        .base = base,
        .stackbase = base + USER_ADDRSPACE_SIZE, 
    };
}

void
pagetable_init(void)
{
    pagetable = (PageTable) {
        .entries = {0}
    };
}

addrspace pagetable_createpage(void) {
    for (unsigned int i = 0; i < 1024; ++i) {
        if ((pagetable.entries[i] & PTE_ALLOCATED) == 0) {
            pagetable.entries[i] |= PTE_ALLOCATED;

            address base = USER_BASE + USER_ADDRSPACE_SIZE * i;
            LOG_DEBUG("base %x", (unsigned char*)base);
            return addrspace_new(base);

        }
    }
    LOG_ERROR("Out of space in page table");
}

void
pagetable_deletepage(addrspace* addrspace)
{
    size_t index = ((uint64_t)addrspace->base - (uint64_t)&pagetable)/USER_ADDRSPACE_SIZE;
    pagetable.entries[index] &= (~PTE_ALLOCATED);
}
