#include "stdlib.h"
#include <stdint.h>

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