#include "kern.h"
#include "stdlib.h"
#include "user/k1.h"
#include "user/k2.h"

int kmain()
{

    // set_log_level(LOG_LEVEL_DEBUG);
    // asm volatile("msr SCTLR_EL1, %x0\n\t" ::"r"((1 << 2) | (1 << 12)));

    kernel_init();

    // K1
    /*
    svc_create(4, &FirstUserTask);
    svc_yield_first();
    */

    // K2
    svc_create(10, &startK2Task);
    svc_yield_first();

    return 0;
}
