#include "svc_helpers.h"
#include "scheduler.h"
#include "kalloc.h"
#include "stdlib.h"
#include "idle-perf.h"

static int next_tid = 0;

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
    while (1)
    {
    }
  }

  LOG_DEBUG("[SYSCALL - Yield]: Context Switch [%d -> %d]", curr_task->tid, next_tid);
  set_current_task(next_tid);
  idle_timer_start_logic(next_tid);

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
  TaskDescriptor *receive_task = get_task(tid);

  if (receive_task == 0)
  {
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
  if (receive_task->status != RECEIVE_WAIT)
  {
    // Sender blocks
    set_task_status(curr_task, SEND_WAIT);

    // Add it to the queue
    push(&receive_task->send_listeners_queue, (uint8_t)curr_task->tid);

    // Save message state
    curr_task->send_state->send_buffer = (char *)msg;
    curr_task->send_state->send_buffer_len = msglen;
  }
  // Scenario 2: Receiver first
  else
  {
    if (receive_task->receive_state == 0)
    {
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

    slab_free(receive_task->receive_state, RECEIVE_STATE);
    receive_task->receive_state = 0;
  }

  return 0;
}

void svc_receive(int *tid, char *msg, int msg_len, TaskDescriptor *curr_task)
{
  /// Scenario 1: Send First

  if (length(&curr_task->send_listeners_queue) != 0)
  {
    uint8_t sender_tid = pop(&curr_task->send_listeners_queue);
    TaskDescriptor *sender_task = get_task(sender_tid);
    // Sanity Check
    LOG_DEBUG("[SYSCALL - Receive]: Send first");
    if (sender_task->status != SEND_WAIT)
    {
      LOG_WARN("[SYSCALL ERROR] - sender task %d not in Send Wait as expected", sender_tid);
    }

    // Set state of sender from SendWait -> ReplyWait
    set_task_status(sender_task, REPLY_WAIT);

    // copy over data
    int copylen = min(sender_task->send_state->send_buffer_len, msg_len);
    memcpy(msg, sender_task->send_state->send_buffer, copylen);

    curr_task->switch_frame->x0 = copylen;

    *tid = sender_tid; // update sender on receiver side
  }
  else
  { // Scenario 2: Receive First
    set_task_status(curr_task, RECEIVE_WAIT);
    ReceiveState *receive_state = slab_alloc(RECEIVE_STATE);
    *receive_state = (ReceiveState){
        .receive_buffer = msg,
        .receive_buffer_len = msg_len,
        .sender_tid = tid};
    curr_task->receive_state = receive_state;
    LOG_DEBUG("[SYSCALL - Receive]: empty receive queue blocking on TID %d", curr_task->tid);
  }
}

int svc_reply(int tid, const char *reply, int rplen)
{
  // Sender must be waiting
  TaskDescriptor *senderTask = get_task(tid);

  if (senderTask == 0)
  {
    return -1;
  }

  // Look up sender (sanity check for reply wait)
  if (senderTask->status != REPLY_WAIT)
  {
    return -2;
  }

  // Make sure that the sender send state is initialized
  if (senderTask->send_state == 0)
  {
    LOG_ERROR("Sender task send state not initialized");
  }

  // unblock sender, set to ready
  set_task_status(senderTask, READY);

  // kernel copies data
  int copylen = min(rplen, senderTask->send_state->reply_buffer_len);
  memcpy(senderTask->send_state->reply_buffer, reply, copylen);

  // Free senderTask send state
  slab_free(senderTask->send_state, SEND_STATE);
  senderTask->send_state = 0;

  // Return value
  senderTask->switch_frame->x0 = copylen;

  return copylen;
}

int svc_await_event(int eventId, TaskDescriptor *curr_task)
{
  if (eventId <= EVENT_NONE || EVENT_MAX <= eventId)
  {
    // Invalid id
    return -1;
  }
  EventType eventType = (EventType)eventId;

  curr_task->status = EVENT_WAIT;
  curr_task->eventWaitType = eventType;
  return 0;
}
