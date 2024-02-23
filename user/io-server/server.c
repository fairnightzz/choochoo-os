#include "server.h"
#include "user/nameserver.h"
#include "kern/rpi.h"
#include <stdbool.h>
#include "lib/stdlib.h"


void marklinReceiveNotifier() {
  int ioServer = WhoIs(MarklinIOAddress);
  while (1) {
    AwaitEvent(EVENT_MARKLIN_RECEIVE);
    SendReceive()
  }
}


void IOServer(size_t line) {
    bool clearToSend = true;

    IORequest request;
    IOResponse response;
    int from_tid;

    LList* getc_tasks = llist_new();
    LList* putc_tasks = llist_new();

    while (1) {
      int request_length = Receive(&from_tid, (char*)&request, sizeof(IORequest));
      if (request_length < 0) {
        LOG_WARN("[IOServer] Error when receiving on %d", line);
        continue;
      }
      switch (request.type) {
        case IO_GETC: {
          LOG_DEBUG("[IOServer] Line %d Getc request from %d with name %s", line, from_tid, TaskName(from_tid)); // check if this fn exists (TaskName)

          bool is_buffer_empty;
          unsigned char ch = uart_getc_buffered(line, &is_buffer_empty); // check this function - exists?

          if (!is_buffer_empty) {
            response = (IOResponse) {
              .type = IO_GETC,
              .data = ch,
            };
            Reply(from_tid, (char *)&response, sizeof(IOResponse));
          } else {
            llist_append(getc_tasks, (void *)from_tid);
          }

          break;
        } case IO_PUTC: {
          LOG_DEBUG("[IOServer] Line %d Putc request from %d with name %s", line, from_tid, TaskName(from_tid));

          if (clearToSend) {
            LOG_DEBUG("[IOServer] Line %d sent immediately on putc", line);
            uart_putc(line, request.data);
            clearToSend = false;
          } else {
            LOG_DEBUG("[IOServer] Line %d queued up on putc", line);
            llist_append(putc_tasks, request.data);
          }
          response = (IOResponse) {
            .type = IO_PUTC,
            .data = 0
          };
          Reply(from_tid, (char*)&response, sizeof(IOResponse));

          break;
        } case IO_RECEIVE_EVENT: {
          LOG_DEBUG("[IOServer] Line %d RECEIVE_EVENT request from %d with name %s", line, from_tid, TaskName(from_tid));

          response = (IOResponse) {
            .type = IO_RECEIVE_EVENT,
            .data = 0
          };
          Reply(from_tid, (char*)&response, sizeof(IOResponse));

          if (llist_length(getc_tasks) > 0) {
            // Respond to all tasks waiting on Getc()
            bool is_buffer_empty;
            unsigned char ch = uart_getc_buffered(line, &is_buffer_empty);
            while (llist_length(getc_tasks) > 0) {
                int tid = llist_pop_front(getc_tasks);

                response = (IOResponse) {
                    .type = IO_GETC,
                    .data = ch
                };
                Reply(tid, (char*)&response, sizeof(IOResponse));
            }
          }

          break;
        } case IO_SEND_EVENT: {
          response = (IOResponse) {
            .type = IO_SEND_EVENT,
            .data = 0,
          };
          Reply(from_tid, (char *)&response, sizeof(IOResponse));

          if (llist_length(putc_tasks) > 0) {
            LOG_DEBUG("[IOServer] Line %d SEND_EVENT received, there is a queued char, printing...", line);
            uart_putc(line, llist_pop_front(putc_tasks));
          } else {
            LOG_DEBUG("[IOServer] Line %d SEND_EVENT received, there is no queued char.", line);
            clearToSend = true;
          }

          break;
        } default: {
          LOG_ERROR("[IOServer]: Unhandled IO Request Type - %d", request.type);
          break;
        }
      }
    }
}

void MarklinIOServer() {
  RegisterAs(MarklinIOAddress);
  Create(5, &marklinReceiveNotifier);
  Create(5, &marklinSendNotifier);
  IoServer(MARKLIN):
}

void ConsoleIOServer() {
  RegisterAs(ConsoleIOAddress);
  Create(5, &consoleReceiveNotifier);
  Create(5, &consoleSendNotifier);
  IoServer(CONSOLE):
}