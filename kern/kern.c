#include "kern.h"
#include "rpi.h"
#include "stdlib.h"
#include "addrspace.h"
#include "kalloc.h"
#include "scheduler.h"
#include "svc_helpers.h"
#include "asm_util.h"
#include "gic.h"
#include "timer.h"
#include "idle-perf.h"

void kernel_init()
{
  vector_table_initialize();
  gpio_init();
  uart_config_and_enable(CONSOLE);
  uart_config_and_enable(MARKLIN);
  slab_init();
  switchframe_init();
  pagetable_init();
  task_init();
  scheduler_init();
  llist_init();
  hashmap_init();
  // string_init();
  gic_target_and_enable(97);
  gic_target_and_enable(153);
  rand_init();
}

static int next_tid = 0;
static uint32_t opCode = 0;
static TaskDescriptor *curr_task = 0;
static uint32_t iar;
static uint32_t interruptId;

void handle_svc()
{

  opCode = get_esr_el1() & 0x1FFFFFF;
  // LOG_DEBUG("[SYSCALL - Handle SVC]: Vector Table Handler OpCode #%x", opCode);

  curr_task = get_current_task();

  idle_timer_stop_logic(curr_task->tid);

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
    scheduler_unblock_event(EVENT_TASK_FINISHED, curr_task->tid);
    scheduler_delete_task(curr_task->tid);
    delete_task(curr_task->tid);

    next_tid = scheduler_next_task();
    if (next_tid == 0)
    {
      // LOG_DEBUG("[SYSCALL - Exit]: no more tasks");
      while (1)
      {
      }
    }
    // LOG_DEBUG("[SYSCALL - Exit]: Context Switch [%d -> %d]", curr_task->tid, next_tid);

    set_current_task(next_tid);
    idle_timer_start_logic(next_tid);
    enter_usermode(get_task(next_tid)->switch_frame);
    break;
  }
  case (KILL):
  {
    // unblock tasks waiting for that task to exit
    scheduler_unblock_event(EVENT_TASK_FINISHED, curr_task->switch_frame->x0);
    scheduler_delete_task(curr_task->switch_frame->x0);
    delete_task(curr_task->switch_frame->x0);
    svc_yield(curr_task);
    break;
  }
  case (SEND):
  {
    // LOG_DEBUG("[SYSCALL - Send]");

    // tid, msg, msglen, reply, rplen
    int ret = svc_send(
        curr_task->switch_frame->x0, (const char *)curr_task->switch_frame->x1, curr_task->switch_frame->x2,
        (char *)curr_task->switch_frame->x3, curr_task->switch_frame->x4, curr_task);

    // If we get an error
    // TODO: make the return in sender perhaps? have to think about it
    if (ret < 0)
    {
      curr_task->switch_frame->x0 = ret;
    }

    svc_yield(curr_task);
    break;
  }
  case (RECEIVE):
  {
    // LOG_DEBUG("[SYSCALL - Receive]");
    svc_receive((int *)curr_task->switch_frame->x0, (char *)curr_task->switch_frame->x1, curr_task->switch_frame->x2, curr_task);
    svc_yield(curr_task);
    break;
  }
  case (REPLY):
  {
    // LOG_DEBUG("[SYSCALL - Reply]");

    curr_task->switch_frame->x0 = svc_reply(
        (int)curr_task->switch_frame->x0,
        (const char *)curr_task->switch_frame->x1,
        curr_task->switch_frame->x2);
    svc_yield(curr_task);
    break;
  }
  case (AWAIT_EVENT):
  {
    // LOG_DEBUG("[SYSCALL - AwaitEvent]");
    curr_task->switch_frame->x0 = svc_await_event((int)curr_task->switch_frame->x0, curr_task);
    svc_yield(curr_task);
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

void handle_irq()
{
  curr_task = get_current_task();
  idle_timer_stop_logic(curr_task->tid);

  iar = gic_read_iar();
  interruptId = iar & 0x3FF; // Read lower 10 bits

  // PRINT("interrupt: %d", interruptId);
  if (interruptId == 97)
  {
    // Unblock tasks with EVENT_WAIT
    scheduler_unblock_events(EVENT_CLOCK_TICK);
    timer_reset_c1();
  }
  else if (interruptId == 153)
  {
    // Handle UART Interrupts
    if (uart_is_cts_interrupt(MARKLIN))
    {
      if (uart_get_cts(MARKLIN))
      {
        scheduler_unblock_events(EVENT_MARKLIN_SEND);
      }
      uart_clear_cts(MARKLIN);
    }

    if (uart_is_rx_interrupt(MARKLIN))
    {
      scheduler_unblock_events(EVENT_MARKLIN_RECEIVE);
      uart_clear_rx(MARKLIN);
    }

    if (uart_is_tx_interrupt(CONSOLE))
    {
      if (uart_get_cts(CONSOLE))
      {
        scheduler_unblock_events(EVENT_CONSOLE_SEND);
      }
      uart_clear_tx(CONSOLE);
      uart_clear_cts(CONSOLE);
    }

    if (uart_is_rx_interrupt(CONSOLE))
    {
      scheduler_unblock_events(EVENT_CONSOLE_RECEIVE);
      uart_clear_rx(CONSOLE);
    }
  }

  gic_write_eoir(iar);

  svc_yield(curr_task);
}