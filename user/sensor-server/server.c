#include "server.h"
#include "user/clock-server/interface.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "lib/stdlib.h"
#define UNIT_COUNT 5
#define BYTE_PER_UNIT 2
#define BYTE_COUNT 10

void marklin_get_bank(int io_server, int index)
{
  Putc(io_server, 192 + index);
}

// task for querying sensor states
void sensorNotifierTask()
{
  uint8_t sensor_state[BYTE_COUNT] = {0};
  uint8_t prev_sensor_state[BYTE_COUNT] = {0};

  int marklin_server = WhoIs(MarklinIOAddress);
  int clock_server = WhoIs(ClockAddress);
  int sensor_server = MyParentTid();

  for (;;)
  {

    for (int sensor_index = 0; sensor_index < UNIT_COUNT; ++sensor_index)
    {

      marklin_get_bank(marklin_server, sensor_index + 1);

      for (int byte_index = 0; byte_index < BYTE_PER_UNIT; ++byte_index)
      {

        int i = sensor_index * 2 + byte_index;

        int sensor_byte = Getc(marklin_server);
        prev_sensor_state[i] = sensor_state[i];
        sensor_state[i] = sensor_byte;
        int triggered = ~(prev_sensor_state[i]) & sensor_state[i];

        // send triggers in batches of 8 sensors
        int triggered_list[9];
        int triggered_list_len = 0;
        for (int j = 0; j < 8; ++j)
        {
          if (((triggered >> j) & 0x1) == 1)
          {
            int index = (7 - j);

            triggered_list[triggered_list_len] = i * 8 + index;
            ++triggered_list_len;
          }
        }

        // send to server task if we have sensors changed
        if (triggered_list_len > 0)
        {
          triggered_list[triggered_list_len] = -1; // set element to be -1 terminated

          triggered_list_len = 0;

          SensorRequest request = (SensorRequest){
              .type = SENSOR_TRIGGERED,
              .ids_triggered = {0},
          };
          memcpy(request.ids_triggered, triggered_list, sizeof(triggered_list));
          SensorResponse response;
          Send(sensor_server, (const char *)&request, sizeof(SensorRequest), (char *)&response, sizeof(SensorResponse));
        }
      }
    }

    Delay(clock_server, 5);
  }

  Exit();
}

void SensorServer()
{
  RegisterAs(SensorAddress);
  alloc_init(SENSOR_BUFFER_REQUEST, sizeof(SensorBufferRequest));

  Create(2, &sensorNotifierTask);

  LList *sensor_requests = llist_new();

  SensorRequest request;
  SensorResponse response;
  int from_tid;
  for (;;)
  {
    int msg_len = Receive(&from_tid, (char *)&request, sizeof(SensorRequest));
    if (msg_len < 0)
    {
      LOG_WARN("[SENSOR SERVER] Error when receiving");
      continue;
    }

    switch (request.type)
    {

    case SENSOR_TRIGGERED:
    {

      // read sensor values and reply to anyone who's waiting
      int *triggered = request.ids_triggered;
      for (; *triggered != -1; ++triggered)
      {
        // unblock all tasks that are waiting
        LListIter *it = llist_iter(sensor_requests);
        while (it->current)
        {
          SensorBufferRequest *sensor_buffer_req = (SensorBufferRequest *)llist_next(it);
          if (sensor_buffer_req->sensor_id == *triggered || sensor_buffer_req->sensor_id == -1)
          {
            SensorResponse reply_buf = (SensorResponse){
                .type = SENSOR_WAITING,
                .triggered = *triggered};
            Reply(sensor_buffer_req->tid, (char *)&reply_buf, sizeof(SensorResponse));
            llist_remove_item(sensor_requests, sensor_buffer_req);
          }
        }
      }

      // response to notifier - could do this at the end?
      response = (SensorResponse){
          .type = SENSOR_TRIGGERED};
      Reply(from_tid, (char *)&response, sizeof(SensorResponse));
      break;
    }
    case SENSOR_WAITING:
    {
      SensorBufferRequest *request_buffer = alloc(SENSOR_BUFFER_REQUEST);
      *request_buffer = (SensorBufferRequest){
          .tid = from_tid,
          .sensor_id = request.id_wait};
      // buffer the wait response
      llist_append(sensor_requests, request_buffer);

      break;
    }
    default:
    {
      LOG_WARN("[SENSOR SERVER] Invalid message type");
      break;
    }
    }
  }

  Exit();
}