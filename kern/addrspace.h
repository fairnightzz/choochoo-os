#ifndef __ADDRSPACE_H__
#define __ADDRSPACE_H__ 1

#include <stdint.h>

typedef unsigned char *address;

static unsigned char *const KERN_BASE = (unsigned char *)0x00200000;
static unsigned char *const USER_BASE = (unsigned char *)0x00220000;
static const unsigned int USER_ADDRSPACE_SIZE = 0x00010000;

typedef struct
{
    address base;
    address stackbase; // the bottom of the stack
} addrspace;

#define PTE_ALLOCATED 0x1

void pagetable_init();

addrspace pagetable_createpage();
void pagetable_deletepage(addrspace *addrspace);

#endif // __ADDRSPACE_H__
