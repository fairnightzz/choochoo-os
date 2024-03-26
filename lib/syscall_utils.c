#include "stdlib.h"
#include "kern/task_descriptor.h"
#include "user/ui/render.h"


void USER_TASK_EXIT()
{
  Exit();
}

void AwaitTid(int tid)
{
  for (;;)
  {
    if (get_task(tid) == 0)
    {
      return;
    }
    int exitedtaskTid = AwaitEvent(EVENT_TASK_FINISHED);
    if (exitedtaskTid == tid)
    {
      break;
    }
  }
}