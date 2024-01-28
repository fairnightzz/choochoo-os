#include <stdint.h>
#include "stdlib.h"

void USER_TASK_WRAPPER(void (*entrypoint)()) {
  (*entrypoint)();
  Exit();
}

void OtherTask()
{
    PRINT("MyTid: %d, MyParentTid: %d", MyTid(), MyParentTid());
    Yield();
    PRINT("MyTid: %d, MyParentTid: %d", MyTid(), MyParentTid());
    Exit();
}

void FirstUserTask()
{
    int t1 = Create(5, &OtherTask);
    PRINT("Created: %d", t1);
    int t2 = Create(5, &OtherTask);
    PRINT("Created: %d", t2);
    int t3 = Create(3, &OtherTask);
    PRINT("Created: %d", t3);
    int t4 = Create(3, &OtherTask);
    PRINT("Created: %d", t4);

    for (int i = 0; i < 150; i++) {
      int t6 = Create(0, &OtherTask);
      PRINT("Created: %d", t6);
    }

    

    PRINT("FirstUserTask: exiting");
    Exit();
}
