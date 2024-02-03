#include "k2.h"
#include "stdlib.h"
#include "nameserver.h"
#include "RPS/client.h"
#include "RPS/server.h"

void RPSTask()
{
  Create(3, &RPSServer);

  // PRINT("Test 1, TID: %d", MyTid());
  // Should end after three and then task 2 sends three
  // PRINT("Test 1");
  // Create(3, &RPSClientTask1);
  // Create(3, &RPSClientTask1);

  // PRINT("Test 2");
  // Create(3, &RPSClientTask1);
  // Create(3, &RPSClientTask2);

  // PRINT("Test 3");
  // Create(3, &RPSClientTask2);
  // Create(3, &RPSClientTask2);

  PRINT("Test 4");
  Create(3, &RPSClientTask2);
  Create(3, &RPSClientTask3);
}

void startK2Task()
{
  NameServerTaskInit();
  RPSTask();
}