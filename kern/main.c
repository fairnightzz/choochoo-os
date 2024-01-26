#include "kern.h"
#include "stdlib.h"
#include "user/userprog.h"

void firstUserTask();

int kmain()
{

    kernel_init();

    // set_log_level(LOG_LEVEL_DEBUG);

    int tid1 = svc_create(4, &FirstUserTask);
    TaskDescriptor *task1 = get_task(tid1);
    enter_usermode(task1->switch_frame);

    return 0;
}
