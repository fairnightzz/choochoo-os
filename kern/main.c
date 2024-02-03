#include "kern.h"
#include "stdlib.h"
#include "user/k1.h"
#include "user/k2.h"

int kmain()
{

    set_log_level(LOG_LEVEL_DEBUG);

    LOG_DEBUG("start kernel init");
    kernel_init();
    LOG_DEBUG("end kernel init");

    // K1
    /*
    svc_create(4, &FirstUserTask);
    svc_yield_first();
    */

    // K2
    LOG_DEBUG("start k2 task");
    svc_create(10, &startK2Task);
    svc_yield_first();

    return 0;
}
