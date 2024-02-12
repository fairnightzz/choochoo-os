#include "k3.h"
#include "stdlib.h"
#include "nameserver.h"
#include "clock-server/interface.h"
#include "clock-server/server.h"
#include "init_tasks.h"

typedef struct
{
  int delayInterval;
  int numDelays;
} K3Response;

typedef struct
{
  bool fromClient;
} K3Request;

void ClientTask()
{
  K3Request request;
  K3Response response;

  Send(MyParentTid(), (char *)&request, sizeof(K3Request), (char *)&response, sizeof(K3Response));

  int clockServer = WhoIs(ClockAddress);
  for (int i = 0; i < response.numDelays; ++i)
  {
    int ticks = Delay(clockServer, response.delayInterval);
    PRINT("Tid: %d, Delay Interval: %d, Loop Iteration: %d, Tick: %d", MyTid(), response.delayInterval, i + 1, ticks);
  }
}

void FirstUserTask()
{
  Create(2, &ClockServer);

  // For printing performance idle
  Create(15, &idlePerformanceTask);

  Create(3, &ClientTask);

  Create(4, &ClientTask);

  Create(5, &ClientTask);

  Create(6, &ClientTask);

  int requestTid;

  K3Request request;

  int delay_intervals[4] = {10, 23, 33, 71};
  int num_delays[4] = {20, 9, 6, 3};

  for (int i = 0; i < 4; i++)
  {
    Receive(&requestTid, (char *)&request, sizeof(K3Request));
    K3Response response = (K3Response){
        .delayInterval = delay_intervals[i],
        .numDelays = num_delays[i]};
    Reply(requestTid, (char *)&response, sizeof(K3Response));
  }
}

void startK3Task()
{
  PRINT("Starting K3 Task!");
  NameServerTaskInit();
  FirstUserTask();
}