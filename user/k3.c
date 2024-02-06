#include "k3.h"
#include "stdlib.h"
#include "nameserver.h"
#include "clock-server/interface.h"
#include "clock-server/server.h"

void ClientTask(int delayInterval, int numDelays)
{
  int clockServer = WhoIs(ClockAddress);
  for (int i = 0; i < numDelays; ++i)
  {
    int ticks = Delay(clockServer, delayInterval);
    PRINT("Tid: %d, Delay Interval: %d, Loop Iteration: %d, Tick: %d", MyTid(), delayInterval, i + 1, ticks);
  }
}
void ClientTask1()
{
  int delayInterval = 10;
  int numDelays = 20;
  ClientTask(delayInterval, numDelays);
}
void ClientTask2()
{
  int delayInterval = 23;
  int numDelays = 9;
  ClientTask(delayInterval, numDelays);
}
void ClientTask3()
{
  int delayInterval = 33;
  int numDelays = 6;
  ClientTask(delayInterval, numDelays);
}
void ClientTask4()
{
  int delayInterval = 71;
  int numDelays = 3;
  ClientTask(delayInterval, numDelays);
}

void FirstUserTask()
{
  Create(2, &ClockServer);
  Create(3, &ClientTask1);
  Create(4, &ClientTask2);
  Create(5, &ClientTask3);
  Create(6, &ClientTask4);
}

void startK3Task()
{
  PRINT("Starting K3 Task!");
  NameServerTaskInit();
  FirstUserTask();
  // k3_performance_measuring();
}