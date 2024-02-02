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

static int next_tid = 0;
static uint32_t opCode = 0;
static TaskDescriptor *curr_task = 0;

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
  next_tid = scheduler_next_task();

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
  next_tid = scheduler_next_task();

  if (next_tid == 0)
  {
    LOG_ERROR("[SYSCALL - Yield]: No tasks left");
    return;
  }

  LOG_DEBUG("[SYSCALL - Initial Yield]: Context Switch [%d -> %d]", 0, next_tid);
  set_current_task(next_tid);

  enter_usermode(get_task(next_tid)->switch_frame);
}

int svc_send(int tid, const char *msg, int msglen, char *reply, int rplen, TaskDescriptor *curr_task)
{
  // Task that is receiving
  TaskDescriptor* receive_task = get_task(tid);

  if (receive_task == 0) {
    return -1;
  }

  if (curr_task->send_state != 0)
  { // ensure that task is not already sending
    return -2;
  }

  SendState *sendState = slab_alloc(SEND_STATE);

  // Define the pointer to reply and reply's max length
  // Store send message and length in here if receive is not called
  *sendState = (SendState){
      .reply_buffer = reply,
      .reply_buffer_len = rplen,
      .send_buffer = 0,
      .send_buffer_len = 0,
  };
  curr_task->send_state = sendState;

  // Scenario 1: Send first
  // Look up receiver, make sure it's not in receive wait
  if (receive_task->status != RECEIVE_WAIT) {
    // Sender blocks
    LOG_DEBUG("Sending message to task %d, not in RECEIVE_WAIT", tid);
    set_task_status(curr_task, SEND_WAIT);
    // Add it to the queue
    push(receive_task->send_listeners_queue, (uint8_t)curr_task->tid);

    // Save message state
    curr_task->send_state->send_buffer = (char*)msg;
    curr_task->send_state->send_buffer_len = msglen;
  }
  // Scenario 2: Receiver first
  else{
    if (receive_task->receive_state == 0) {
      slab_free(sendState, SEND_STATE);
      curr_task->send_state = 0;
      LOG_ERROR("Receiving task does not have receive state intialized");
      return -2;
    }

    // Unblock receiver
    set_task_status(receive_task, READY);

    // Block Sender
    set_task_status(curr_task, REPLY_WAIT);

    // Kernel copies data
    int copylen = min(msglen, receive_task->receive_state->receive_buffer_len); 
    memcpy(receive_task->receive_state->receive_buffer, msg, copylen);

    // Return
    // set the return value for receive task
    receive_task->switch_frame->x0 = copylen;

    // set sender tid for receive task
    *(receive_task->receive_state->sender_tid) = curr_task->tid;

  }

  return 0;
}

void handle_svc()
{

  opCode = get_esr_el1() & 0x1ffffff;
  LOG_DEBUG("[SYSCALL - Handle SVC]: Vector Table Handler OpCode #%x", opCode);

  // If kernel just started
  // if (get_current_task_id() == 0)
  // {
  //   LOG_DEBUG("YIELD ON FIRST CALL", opCode);
  //   svc_yield_first();
  // }

  curr_task = get_current_task();

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

    next_tid = scheduler_next_task();
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
  case (SEND):
  {
    LOG_DEBUG("[SYSCALL - Send]");

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
    break;
  }
  case (REPLY):
  {
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
