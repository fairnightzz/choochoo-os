#include "init_tasks.h"
#include "kern/asm_util.h"

void idleTask()
{
  for (;;)
  {
    call_wfi();
  }
}