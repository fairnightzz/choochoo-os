#include "kern.h"
#include "stdlib.h"
#include "user/k1.h"
#include "user/k2.h"
#include "user/k3.h"
#include "user/init_tasks.h"
#include "idle-perf.h"

int kmain()
{

    set_log_level(LOG_LEVEL_DEBUG);

    kernel_init();

    // K1
    /*
    svc_create(4, &K1_FirstUserTask);
    svc_yield_first();
    */

    // K2
    /*
    svc_create(4, &startK2Task);
    svc_yield_first();
    */

    // K3
    int idleTid = svc_create(31, &idleTask);
    idle_timer_init(idleTid);

    svc_create(10, &startK3Task);
    svc_yield_first();

    return 0;
}
