#include "init_tasks.h"
#include "kern/asm_util.h"
#include "stdlib.h"

void idleTask()
{
  for (;;)
  {
    call_wfi();
  }
}