#include "kern.h"
#include "stdlib.h"
#include "user/userprog.h"

void firstUserTask();
void secondUserTask();

typedef uint32_t Tid;

int kmain()
{

    kernel_init();

    // set_log_level(LOG_LEVEL_DEBUG);

    Tid tid1 = svc_create(4, &firstUserTask);
    TaskDescriptor *task1 = get_task(tid1);
    enter_usermode(task1->switch_frame);

    return 0;
}
