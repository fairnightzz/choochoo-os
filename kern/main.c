#include "kern.h"
#include "stdlib.h"

void firstUserTask();
void secondUserTask();

typedef uint32_t Tid;

void firstUserTask()
{
    Tid t1 = Create(5, &secondUserTask);
    PRINT("Created: %d", t1);
    Tid t2 = Create(5, &secondUserTask);
    PRINT("Created: %d", t2);
    Tid t3 = Create(3, &secondUserTask);
    PRINT("Created: %d", t3);
    Tid t4 = Create(3, &secondUserTask);
    PRINT("Created: %d", t4);

    PRINT("FirstUserTask: exiting");
    Exit();
}

void secondUserTask()
{
    PRINT("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Yield();
    PRINT("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Exit();
}

int kmain()
{

    kernel_init();

    // set_log_level(LOG_LEVEL_DEBUG);

    Tid tid1 = svc_create(4, &firstUserTask);
    TaskDescriptor *task1 = get_task(tid1);
    enter_usermode(task1->switch_frame);

    return 0;
}
