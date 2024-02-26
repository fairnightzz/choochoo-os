#include "server.h"
#include "user/nameserver.h"
#include "kern/rpi.h"
#include <stdbool.h>
#include "lib/stdlib.h"
#include "kern/task_descriptor.h"

int SendReceiveEvent(int io_server)
{
  IOResponse response;
  IORequest request = (IORequest){
      .type = IO_RECEIVE_EVENT,
      .data = 0,
  };

  int ret = Send(io_server, (const char *)&request, sizeof(IORequest), (char *)&response, sizeof(IOResponse));

  if (ret < 0)
  {
    LOG_WARN("[TID %d] WARNING, SendReceiveEvent()'s Send() call returned a negative value", MyTid());
    return -1;
  }

  if (response.type != IO_RECEIVE_EVENT)
  {
    LOG_WARN("[TID %d] WARNING, the reply to SendRX()'s Send() call is not the right type", MyTid());
    return -2;
  }

  return 0;
}

int SendSendEvent(int io_server)
{
  IOResponse response;
  IORequest request = (IORequest){
      .type = IO_SEND_EVENT,
      .data = 0,
  };

  int ret = Send(io_server, (const char *)&request, sizeof(IORequest), (char *)&response, sizeof(IOResponse));

  if (ret < 0)
  {
    LOG_WARN("[TID %d] WARNING, SendSendEvent()'s Send() call returned a negative value", MyTid());
    return -1;
  }
  if (request.type != IO_SEND_EVENT)
  {
    LOG_WARN("[TID %d] WARNING, the reply to SendSendEvent()'s Send() call is not the right type", MyTid());
    return -2;
  }

  return 0;
}

void marklinReceiveNotifier()
{
  int ioServer = WhoIs(MarklinIOAddress);
  while (1)
  {
    AwaitEvent(EVENT_MARKLIN_RECEIVE);
    SendReceiveEvent(ioServer);
  }
}

void marklinSendNotifier()
{
  int ioServer = WhoIs(MarklinIOAddress);
  while (1)
  {
    AwaitEvent(EVENT_MARKLIN_SEND);
    SendSendEvent(ioServer);
  }
}

void consoleReceiveNotifier()
{
  int ioServer = WhoIs(ConsoleIOAddress);
  while (1)
  {
    AwaitEvent(EVENT_CONSOLE_RECEIVE);
    SendReceiveEvent(ioServer);
  }
}

void consoleSendNotifier()
{
  int ioServer = WhoIs(ConsoleIOAddress);
  while (1)
  {
    AwaitEvent(EVENT_CONSOLE_SEND);
    SendSendEvent(ioServer);
  }
}

void IOServer(size_t line)
{
  bool clearToSend = true;

  IORequest request;
  IOResponse response;
  int from_tid;

  LList *getc_tasks = llist_new();
  LList *putc_tasks = llist_new();

  while (1)
  {
    int request_length = Receive(&from_tid, (char *)&request, sizeof(IORequest));
    if (request_length < 0)
    {
      LOG_WARN("[IOServer] Error when receiving on %d", line);
      continue;
    }
    switch (request.type)
    {
    case IO_GETC:
    {
      LOG_DEBUG("[IOServer] Line %d Getc request from %d", line, from_tid); // check if this fn exists (TaskName)

      bool is_buffer_empty;
      unsigned char ch = uart_getc_queued(line, &is_buffer_empty); // check this function - exists?

      if (!is_buffer_empty)
      {
        response = (IOResponse){
            .type = IO_GETC,
            .data = ch,
        };
        Reply(from_tid, (char *)&response, sizeof(IOResponse));
      }
      else
      {
        llist_append(getc_tasks, (void *)(uintptr_t)from_tid);
      }

      break;
    }
    case IO_PUTC:
    {
      if (line == 1 && llist_length(putc_tasks) > 500)
      {
        // PRINT("[IOServer] queue length: %d ", llist_length(putc_tasks));
      }

      if (clearToSend)
      {
        LOG_DEBUG("[IOServer] Line %d sent immediately on putc", line);
        uart_putc(line, request.data);
        clearToSend = false;
      }
      else
      {
        // PRINT("[IOServer] Line %d queued up on putc", line);
        llist_append(putc_tasks, (void *)(uintptr_t)request.data);
      }
      response = (IOResponse){
          .type = IO_PUTC,
          .data = 0};
      Reply(from_tid, (char *)&response, sizeof(IOResponse));

      break;
    }
    case IO_RECEIVE_EVENT:
    {
      LOG_DEBUG("[IOServer] Line %d RECEIVE_EVENT request from %d", line, from_tid);

      response = (IOResponse){
          .type = IO_RECEIVE_EVENT,
          .data = 0};
      Reply(from_tid, (char *)&response, sizeof(IOResponse));

      if (llist_length(getc_tasks) > 0)
      {
        // Respond to all tasks waiting on Getc()
        bool is_buffer_empty;
        unsigned char ch = uart_getc_queued(line, &is_buffer_empty);
        while (llist_length(getc_tasks) > 0)
        {
          int tid = (int)(uintptr_t)llist_pop_front(getc_tasks);

          response = (IOResponse){
              .type = IO_GETC,
              .data = ch};
          Reply(tid, (char *)&response, sizeof(IOResponse));
        }
      }

      break;
    }
    case IO_SEND_EVENT:
    {
      response = (IOResponse){
          .type = IO_SEND_EVENT,
          .data = 0,
      };
      Reply(from_tid, (char *)&response, sizeof(IOResponse));

      while (llist_length(putc_tasks) > 0 && uart_try_putc(line, (unsigned char)(uintptr_t)llist_front(putc_tasks)))
      {
        llist_pop_front(putc_tasks);
      }
      clearToSend = (llist_length(putc_tasks) == 0);
      break;
    }
    default:
    {
      LOG_ERROR("[IOServer]: Unhandled IO Request Type - %d", request.type);
      break;
    }
    }
  }
}

void MarklinIOServer()
{
  RegisterAs(MarklinIOAddress);
  Create(5, &marklinReceiveNotifier);
  Create(5, &marklinSendNotifier);
  IOServer(MARKLIN);
}

void ConsoleIOServer()
{
  RegisterAs(ConsoleIOAddress);
  Create(5, &consoleReceiveNotifier);
  Create(5, &consoleSendNotifier);
  IOServer(CONSOLE);
}