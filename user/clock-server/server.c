#include "server.h"
#include "kern/timer.h"
#include "kern/task_descriptor.h"
#include "lib/stdlib.h"
#include "user/nameserver.h"

void waitTick(int clock_server_tid);

// notifier
void awaitTick()
{
  int clock_server = WhoIs(ClockAddress);
  while (1)
  {
    AwaitEvent(EVENT_CLOCK_TICK);
    waitTick(clock_server);
  }
}

void waitTick(int clock_server_tid)
{
  ClockResponse response;
  ClockRequest request = (ClockRequest){
      .type = CLOCK_TICK,
      .ticks = -1,
  };

  int ret = Send(clock_server_tid, (const char *)&request, sizeof(ClockRequest), (char *)&response, sizeof(ClockResponse));
  if (ret < 0)
  {
    LOG_ERROR("error in waitTick()");
  }
}

void ClockServer()
{
  RegisterAs(ClockAddress);
  alloc_init(CLOCK_BUFFER_REQUEST, sizeof(ClockBufferRequest));

  LList *clk_requests = llist_new();
  uint32_t tick_count = 0;

  ClockRequest request;
  ClockResponse response;

  int from_tid;

  Create(1, &awaitTick);
  timer_init_c1();

  while (1)
  {
    int request_length = Receive(&from_tid, (char *)&request, sizeof(ClockBufferRequest));
    if (request_length < 0)
    {
      LOG_ERROR("[CLOCK SERVER]: receive request error");
    }

    switch (request.type)
    {
    case CLOCK_TIME:
    {
      response = (ClockResponse){
          .type = CLOCK_TIME,
          .ticks = tick_count,
      };
      Reply(from_tid, (char *)&response, sizeof(ClockResponse));
      break;
    }
    case CLOCK_DELAY:
    {
      ClockBufferRequest *buffer_request = alloc(CLOCK_BUFFER_REQUEST);
      buffer_request->tid = from_tid;
      buffer_request->absolute_tick_delay = request.ticks + tick_count;
      buffer_request->type = CLOCK_DELAY;
      llist_append(clk_requests, buffer_request);
      break;
    }
    case CLOCK_DELAY_UNTIL:
    {
      ClockBufferRequest *buffer_request = alloc(CLOCK_BUFFER_REQUEST);
      buffer_request->tid = from_tid;
      buffer_request->absolute_tick_delay = request.ticks;
      buffer_request->type = CLOCK_DELAY_UNTIL;
      llist_append(clk_requests, buffer_request);
      break;
    }
    case CLOCK_TICK:
    {
      // increment clock ticks
      tick_count += 1;

      response = (ClockResponse){
          .type = CLOCK_TICK,
          .ticks = tick_count,
      };
      Reply(from_tid, (char *)&response, sizeof(ClockResponse));
      LListIter *it = llist_iter(clk_requests);
      while (it->current)
      {
        ClockBufferRequest *clk_req = (ClockBufferRequest *)llist_next(it);
        if (clk_req->absolute_tick_delay <= tick_count)
        {
          switch (clk_req->type)
          {
          case CLOCK_DELAY:
          {
            response = (ClockResponse){
                .type = CLOCK_DELAY,
                .ticks = tick_count,
            };
            break;
          }
          case CLOCK_DELAY_UNTIL:
          {
            response = (ClockResponse){
                .type = CLOCK_DELAY_UNTIL,
                .ticks = tick_count,
            };
            break;
          }
          default:
          {
            LOG_ERROR("[CLOCK SERVER]: unhandled clock request type in linked list queue");
            break;
          }
          }
          Reply(clk_req->tid, (char *)&response, sizeof(ClockResponse));
          llist_remove_item(clk_requests, clk_req);
          free(clk_req, CLOCK_BUFFER_REQUEST);
        }
      }
      llist_delete(it);
      break;
    }
    default:
    {
      LOG_ERROR("[CLOCK SERVER]: Unhandled Clock Message Type - %d", request.type);
      break;
    }
    }
  }
}