#include "kern.h"
#include "rpi.h"
#include "stdlib.h"
#include "addrspace.h"
#include "kalloc.h"
#include "util.h"
#include "scheduler.h"

void kernel_init()
{
  vector_table_initialize();
  gpio_init();
  uart_config_and_enable(CONSOLE);
  slab_init();
  switchframe_init();
  pagetable_init();
  task_init();
  scheduler_init();
}

int svc_create(uint32_t priority, void (*entrypoint)())
{
  int new_tid = (priority < NUM_PRIORITY_LEVELS) ? create_task(priority, entrypoint) : -1;

  LOG_DEBUG("[SYSCALL - Create]: Task #%d", new_tid);

  // Only add if tid is valid
  if (new_tid > 0)
  {
    scheduler_add_task(new_tid, priority);
  }

  return new_tid;
}

void svc_yield(TaskDescriptor *curr_task)
{
  int next_tid = scheduler_next_task();

  if (next_tid == 0)
  {
    LOG_ERROR("[SYSCALL - Yield]: No tasks left");
    return;
  }

  LOG_DEBUG("[SYSCALL - Yield]: Context Switch [%d -> %d]", curr_task->tid, next_tid);
  set_current_task(next_tid);

  enter_usermode(get_task(next_tid)->switch_frame);
}

void svc_yield_first()
{
  int next_tid = scheduler_next_task();

  if (next_tid == 0)
  {
    LOG_ERROR("[SYSCALL - Yield]: No tasks left");
    return;
  }

  LOG_DEBUG("[SYSCALL - Initial Yield]: Context Switch [%d -> %d]", 0, next_tid);
  set_current_task(next_tid);

  enter_usermode(get_task(next_tid)->switch_frame);
}

void handle_svc()
{

  uint32_t opCode = get_esr_el1() & 0x1ffffff;
  LOG_DEBUG("[SYSCALL - Handle SVC]: Vector Table Handler OpCode #%x", opCode);

  // If kernel just started
  // if (get_current_task_id() == 0)
  // {
  //   LOG_DEBUG("YIELD ON FIRST CALL", opCode);
  //   svc_yield_first();
  // }

  TaskDescriptor *curr_task = get_current_task();

  switch (opCode)
  {
  case (CREATE):
  {
    curr_task->switch_frame->x0 = svc_create(curr_task->switch_frame->x0, (void (*)())curr_task->switch_frame->x1);
    svc_yield(curr_task);
    break;
  }
  case (MY_TID):
  {
    LOG_DEBUG("[SYSCALL - MyTid]: MyTid %d", get_current_task_id());
    curr_task->switch_frame->x0 = get_current_task_id();
    svc_yield(curr_task);
    break;
  }
  case (MY_PARENT_TID):
  {
    LOG_DEBUG("[SYSCALL - MyParentTid]: MyParentTid %d", curr_task->pTid);
    curr_task->switch_frame->x0 = curr_task->pTid;
    svc_yield(curr_task);
    break;
  }
  case (YIELD):
  {
    svc_yield(curr_task);
    break;
  }
  case (EXIT):
  {
    scheduler_delete_task(curr_task->tid);
    delete_task(curr_task->tid);

    int next_tid = scheduler_next_task();
    if (next_tid == 0)
    {
      LOG_DEBUG("[SYSCALL - Exit]: no more tasks");
      while (1)
      {
      }
    }
    LOG_DEBUG("[SYSCALL - Exit]: Context Switch [%d -> %d]", curr_task->tid, next_tid);

    set_current_task(next_tid);
    LOG_DEBUG("Done setting current task");
    enter_usermode(get_task(next_tid)->switch_frame);
    break;
  }
  default:
  {
    LOG_WARN("[SYSCALL - ERROR]: Uncaught System Call [OpCode %x]", opCode);
    svc_yield(curr_task);
    break;
  }
  }
}
