#include "server.h"
#include "user/clock-server/interface.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "lib/stdlib.h"
#include "user/ui/render.h"
#include "user/io-server/io_marklin.h"
#define UNIT_COUNT 5
#define BYTE_PER_UNIT 2
#define BYTE_COUNT 10

BQueue tid_sensor_queue[128];
bool tid_registered[128];

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

    io_marklin_dump_sensors(marklin_server, UNIT_COUNT);

    int triggered_list[MAX_TRIGGERED];
    int triggered_list_len = 0;
    for (int i = 0; i < BYTE_COUNT; ++i)
    {
      uint8_t sensor_byte = Getc(marklin_server);
      Delay(clock_server, 1);
      prev_sensor_state[i] = sensor_state[i];
      sensor_state[i] = sensor_byte;
      uint8_t triggered = ~(prev_sensor_state[i]) & sensor_state[i];

      for (int j = 0; j < 8; ++j)
      {
        if (((triggered >> j) & 0x1) == 1)
        {
          int index = (7 - j);

          triggered_list[triggered_list_len] = i * 8 + index;
          ++triggered_list_len;
        }
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

    Delay(clock_server, 10);
  }
}

void SensorServer()
{
  RegisterAs(SensorAddress);
  alloc_init(SENSOR_BUFFER_REQUEST, sizeof(SensorBufferRequest));

  Create(2, &sensorNotifierTask);
  // Create(5, &sensorNotiferMonitorTask);

  for (int i = 0; i < 128; i++)
  {
    tid_sensor_queue[i] = new_byte_queue();
    tid_registered[i] = false;
  }

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
        // char letter[2] = {'A' + *triggered / 16, '\0'};
        // string sensor_str = string_format("%s%d", letter, (*triggered % 16) + 1);
        // render_command("Sensor triggered: %s", sensor_str.data);
        for (int i = 0; i < 128; i++)
        {
          if (tid_registered[i])
          {
            push(&tid_sensor_queue[i], *triggered);
          }
        }
      }

      LListIter *it = llist_iter(sensor_requests);
      while (it->current)
      {
        SensorBufferRequest *sensor_buffer_req = (SensorBufferRequest *)llist_next(it);

        int triggered_sensor = -1;
        bool found = false;
        while (!isEmpty(&tid_sensor_queue[sensor_buffer_req->tid]))
        {
          triggered_sensor = pop(&tid_sensor_queue[sensor_buffer_req->tid]);
          if (sensor_buffer_req->sensor_id == -1 || sensor_buffer_req->sensor_id == triggered_sensor)
          {
            found = true;
            break;
          }
        }

        if (found)
        {
          SensorResponse reply_buf = (SensorResponse){
              .type = SENSOR_WAITING,
              .triggered = triggered_sensor,
          };
          Reply(sensor_buffer_req->tid, (char *)&reply_buf, sizeof(SensorResponse));
          llist_remove_item(sensor_requests, sensor_buffer_req);
          free(sensor_buffer_req, SENSOR_BUFFER_REQUEST);
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
      // A1, A2, A3, A4
      int triggered_sensor = -1;
      bool found = false;
      while (!isEmpty(&tid_sensor_queue[from_tid]))
      {
        triggered_sensor = pop(&tid_sensor_queue[from_tid]);
        if (request.id_wait == -1 || request.id_wait == triggered_sensor)
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        tid_registered[from_tid] = true;
        SensorBufferRequest *request_buffer = alloc(SENSOR_BUFFER_REQUEST);
        *request_buffer = (SensorBufferRequest){
            .tid = from_tid,
            .sensor_id = request.id_wait,
        };

        // buffer the wait response
        llist_append(sensor_requests, request_buffer);
      }
      else
      {
        SensorResponse reply_buf = (SensorResponse){
            .type = SENSOR_WAITING,
            .triggered = triggered_sensor,
        };
        Reply(from_tid, (char *)&reply_buf, sizeof(SensorResponse));
      }

      break;
    }
    // case SENSOR_GET_RECENT:
    // {
    //   int ids_triggered[9];
    //   int current = 0;

    //   // put the sensor ids into array
    //   for (int i = 0; i < 9; i++)
    //   {
    //     if (!isEmpty(&sensor_triggered_queue))
    //     {
    //       ids_triggered[current] = pop(&sensor_triggered_queue);
    //       current++;
    //     }
    //   }

    //   ids_triggered[current] = -1;

    //   SensorGetRecentResponse response = (SensorGetRecentResponse){
    //       .ids_triggered = {0},
    //   };
    //   memcpy(response.ids_triggered, ids_triggered, sizeof(ids_triggered));
    //   Reply(from_tid, (char *)&response, sizeof(SensorGetRecentResponse));
    //   break;
    // }
    default:
    {
      LOG_WARN("[SENSOR SERVER] Invalid message type");
      break;
    }
    }
  }

  Exit();
}
