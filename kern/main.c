#include "kern.h"
#include "stdlib.h"
#include "user/k1.h"
#include "user/k2.h"

int kmain()
{

    kernel_init();

    // set_log_level(LOG_LEVEL_DEBUG);

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
