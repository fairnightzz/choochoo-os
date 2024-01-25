#include "kern.h"
#include "rpi.h"
#include "stdlib.h"
#include "addrspace.h"
#include "kalloc.h"
#include "task_descriptor.h"
#include "util.h"
#include "scheduler.h"

void init_kernel(void)
{
  vector_table_initialize();
  gpio_init();
  uart_config_and_enable(CONSOLE);
  slab_init();
  pagetable_init();
}

uint32_t svc_create(uint32_t priority, void (*entrypoint)()) {
  uint32_t new_tid = create_task(priority, entrypoint);

  TaskDescriptor *td = get_current_task();
  td->pTid = (new_tid == 1) ? 0 : td->tid;

  LOG_DEBUG("[SYSCALL - Create]: Task #%d", new_tid);
  
  scheduler_add_task(new_tid, priority);

  return new_tid;
}

void handle_svc() {
  TaskDescriptor *curr_task = get_current_task();

  uint32_t opCode = get_esr_el1() & 0x1ffffff;
  LOG_DEBUG("[SYSCALL - Handle SVC]: Vector Table Handler OpCode #%x", opCode);

  switch (opCode) {
    case (CREATE): {
      curr_task->switch_frame->x0 = svc_create(curr_task->switch_frame->x0, (void (*)()) curr_task->switch_frame->x1);
      enter_usermode(curr_task->switch_frame);
      break;
    } case (MY_TID): {
      LOG_DEBUG("[SYSCALL - MyTid]: MyTid %d", get_current_task_id());
      curr_task->switch_frame->x0 = get_current_task_id();
      enter_usermode(curr_task->switch_frame);
      break;
    } case (MY_PARENT_TID): {
      LOG_DEBUG("[SYSCALL - MyParentTid]: MyParentTid %d", curr_task->pTid);
      curr_task->switch_frame->x0 = curr_task->pTid;
      enter_usermode(curr_task->switch_frame);
      break;
    } case (YIELD): {
      uint32_t next_tid = scheduler_next_task();

      if (next_tid == 0) {
        LOG_ERROR("[SYSCALL - Yield]: No tasks left");
        return;
      }

      LOG_DEBUG("[SYSCALL - Yield]: Context Switch [%d -> %d]", curr_task->tid, next_tid);
      set_current_task(next_tid);

      enter_usermode(get_task(next_tid)->switch_frame);
      break;
    } case (EXIT): {
      scheduler_delete_task(curr_task->tid);
      delete_task(curr_task->tid);

      uint32_t next_tid = scheduler_next_task();
      if (next_tid == 0) {
        LOG_DEBUG("[SYSCALL - Exit]: no more tasks");
        while (1) {}
      }
      LOG_DEBUG("[SYSCALL - Exit]: Context Switch [%d -> %d]", curr_task->tid, next_tid);

      set_current_task(next_tid);
      enter_usermode(get_task(next_tid)->switch_frame);
      break;
    } default: {
      LOG_WARN("[SYSCALL - ERROR]: Uncaught System Call [OpCode %x]", opCode);
      enter_usermode(curr_task->switch_frame);
      break;
    }
  }


}
