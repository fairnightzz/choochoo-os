#include "server.h"
#include "interface.h"
#include "user/traindata/train_data.h"
#include "lib/stdlib.h"
#include "user/nameserver.h"
#include "user/ui/render.h"
#include "user/clock-server/interface.h"

static int reservations[NUM_ZONES]; // zero means no train has the zone reserved

// returns if the zone was successfully reserved
bool _zone_reserve(int train, int zone)
{
  if (reservations[zone] == train)
    return true; // train is allowed to reserve a zone multiple times
  if (reservations[zone] != 0)
  {
    render_command("unable for train %d to reserve zone %d, already reserved by train %d", train, zone, reservations[zone]);
    return false;
  }
  reservations[zone] = train;
  return true;
}

// returns false if zone was not able to be unreserved
bool _zone_unreserve(int train, int zone)
{
  if (reservations[zone] == 0)
    return true;
  if (reservations[zone] != train)
  {
    return false;
  }
  reservations[zone] = 0;

  return true;
}

// false means that the zone is free to be claimed by train
bool _zone_is_reserved(int train, int zone)
{
  return reservations[zone] != 0 && reservations[zone] != train;
}

typedef struct
{
  int tid;
  int train;
  int zone;
  int time;
} ZoneBufferRequest;

void reservationWaitUnblock(LList *zone_buffer_requests, int updated_zone)
{

  LListIter *it = llist_iter(zone_buffer_requests);
  while (it->current)
  {
    ZoneBufferRequest *zone_buffer_req = (ZoneBufferRequest *)llist_next(it);
    if (zone_buffer_req->zone != updated_zone)
      continue;

    if (!_zone_is_reserved(zone_buffer_req->train, updated_zone))
    {

      // render_command("freeing zone %d for train %d", updated_zone, zone_buffer_req->train);
      ZoneResponse reply_buf = (ZoneResponse){
          .type = ZONE_WAIT,
          .time_out = false,
      };
      Reply(zone_buffer_req->tid, (char *)&reply_buf, sizeof(ZoneResponse));
      llist_remove_item(zone_buffer_requests, zone_buffer_req);
      free(zone_buffer_req, ZONE_BUFFER_REQUEST);
      return; // currently unblocks one request at a time
    }
  }
}

// store tids, no need for other info
void unblockAllWaiting(BQueue *wait_change_requests)
{
  ZoneResponse response = (ZoneResponse){
      .type = ZONE_WAIT,
      .time_out = false,
  };
  while (length(wait_change_requests) > 0)
  {
    int from_tid = (int)pop(wait_change_requests);
    Reply(from_tid, (char *)&response, sizeof(ZoneResponse));
  }
}

void unblockStaleRequests(int time, LList *zone_buffer_requests)
{
  LListIter *it = llist_iter(zone_buffer_requests);
  while (it->current)
  {
    ZoneBufferRequest *zone_buffer_req = (ZoneBufferRequest *)llist_next(it);
    if (zone_buffer_req->time < time - 1000)
    {
      ZoneResponse reply_buf = (ZoneResponse){
          .type = ZONE_WAIT,
          .time_out = true,
      };
      Reply(zone_buffer_req->tid, (char *)&reply_buf, sizeof(ZoneResponse));
      llist_remove_item(zone_buffer_requests, zone_buffer_req);
      free(zone_buffer_req, ZONE_BUFFER_REQUEST);
      return; // currently unblocks one request at a time
    }
  }
}

void zoneDeadlockNotifier()
{
  int clockServer = WhoIs(ClockAddress);
  int parentTid = MyParentTid();
  for (;;)
  {
    Delay(clockServer, 1000);
    ZoneResponse response;
    ZoneRequest request = (ZoneRequest){
        .type = ZONE_DEADLOCK,
    };

    int returnValue = Send(parentTid, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

    if (returnValue < 0)
    {
      LOG_WARN("Zone deadlock request check failed");
    }
  }
}

void ZoneServer()
{
  RegisterAs(ZoneAddress);
  int clockServer = WhoIs(ClockAddress);
  alloc_init(ZONE_BUFFER_REQUEST, sizeof(ZoneBufferRequest));

  // track_node *track = get_track();

  for (int i = 0; i < NUM_ZONES; ++i)
  {
    reservations[i] = 0;
  }

  LList *zone_requests = llist_new();
  BQueue wait_change_requests = new_byte_queue();

  Create(4, &zoneDeadlockNotifier);
  ZoneRequest request;
  ZoneResponse response;
  int from_tid;
  for (;;)
  {
    int msg_len = Receive(&from_tid, (char *)&request, sizeof(ZoneRequest));
    if (msg_len < 0)
    {
      render_command("Zone request error when receiving");
      continue;
    }

    if (request.type == ZONE_RESERVE)
    {
      bool ret = _zone_reserve(request.train, request.zone);

      if (ret)
      {
        render_reserve_zone(request.train, request.zone);
      }

      response = (ZoneResponse){
          .type = ZONE_RESERVE,
          .reserve = ret,
      };
      Reply(from_tid, (char *)&response, sizeof(ZoneResponse));

      unblockAllWaiting(&wait_change_requests);
    }
    else if (request.type == ZONE_UNRESERVE)
    {
      bool ret = _zone_unreserve(request.train, request.zone);

      if (ret)
      {
        render_unreserve_zone(request.zone);
      }

      response = (ZoneResponse){
          .type = ZONE_UNRESERVE,
          .unreserve = ret,
      };
      Reply(from_tid, (char *)&response, sizeof(ZoneResponse));

      reservationWaitUnblock(zone_requests, request.zone);

      unblockAllWaiting(&wait_change_requests);
    }
    else if (request.type == ZONE_UNRESERVE_ALL)
    {
      for (int i = 0; i < NUM_ZONES; ++i)
      {
        if (reservations[i] == request.unreserve_all)
        {
          reservations[i] = 0;
          render_unreserve_zone(i);
          reservationWaitUnblock(zone_requests, i);
        }
      }

      response = (ZoneResponse){
          .type = ZONE_UNRESERVE_ALL,
      };
      Reply(from_tid, (char *)&request, sizeof(ZoneRequest));

      unblockAllWaiting(&wait_change_requests);
    }
    else if (request.type == ZONE_UNRESERVE_ALL_EXCEPT)
    {
      for (int i = 0; i < NUM_ZONES; ++i)
      {
        if (reservations[i] == request.unreserve_all && i != request.zone) // only reserve if i != zone
        {
          reservations[i] = 0;
          render_unreserve_zone(i);
          reservationWaitUnblock(zone_requests, i);
        }
      }

      response = (ZoneResponse){
          .type = ZONE_UNRESERVE_ALL,
      };
      Reply(from_tid, (char *)&request, sizeof(ZoneRequest));

      unblockAllWaiting(&wait_change_requests);
    }
    else if (request.type == ZONE_IS_RESERVED)
    {
      int train = request.train;
      int zone = request.zone;
      bool is_reserved = _zone_is_reserved(train, zone);

      response = (ZoneResponse){
          .type = ZONE_IS_RESERVED,
          .is_reserved = is_reserved,
      };
      Reply(from_tid, (char *)&response, sizeof(ZoneResponse));
    }
    else if (request.type == ZONE_DEADLOCK)
    {
      unblockStaleRequests(Time(clockServer), zone_requests);
      response = (ZoneResponse){
          .type = ZONE_DEADLOCK,
      };
      Reply(from_tid, (char *)&response, sizeof(ZoneResponse));
    }
    else if (request.type == ZONE_WAIT)
    {

      int train = request.train;
      int zone = request.zone;

      // if the train is already holding the zone, return
      if (!_zone_is_reserved(train, zone))
      {
        render_command("Zone server train %d is alreading holding zone %d, unblocking", train, zone);
        response = (ZoneResponse){
            .type = ZONE_WAIT,
            .time_out = false,
        };
        Reply(from_tid, (char *)&response, sizeof(ZoneResponse));
        continue;
      }

      // otherwise, buffer the wait request
      ZoneBufferRequest *request_buffer_req = alloc(ZONE_BUFFER_REQUEST);
      *request_buffer_req = (ZoneBufferRequest){
          .tid = from_tid,
          .train = train,
          .zone = zone,
          .time = Time(clockServer)};
      llist_append(zone_requests, request_buffer_req);
    }
    else if (request.type == ZONE_WAIT_CHANGE)
    {
      // for waiting on ANY change
      push(&wait_change_requests, from_tid);
    }
  }

  Exit();
}