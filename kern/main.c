#include "kern.h"
#include "stdlib.h"
#include "user/k1.h"

int kmain()
{

    kernel_init();

    // set_log_level(LOG_LEVEL_DEBUG);

    svc_create(4, &FirstUserTask);
    // TaskDescriptor *task1 = get_task(tid1);
    svc_yield_first();
    // enter_usermode(task1->switch_frame);

    return 0;
}
