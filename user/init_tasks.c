#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"

void idleTask()
{
  PRINT("start idle task");
  for (;;)
  {
    call_wfi();
  }
}