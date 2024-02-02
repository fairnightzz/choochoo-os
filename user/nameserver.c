#include "nameserver.h"

static int nameserver_tid;
static HashMap *nameserver_map;

int RegisterAs(const char *name)
{
  NameServerMessage request = (NameServerMessage){
      .type = NS_REGISTER_AS,
      .data = {
          .register_as = {
              .name = (char *)name,
          }}};
  NameServerResponse response;

  int returnValue = Send(
      nameserver_tid,
      (const char *)&request,
      sizeof(NameServerMessage),
      (char *)&response,
      sizeof(NameServerResponse));

  if (returnValue < 0)
  {
    return -1;
  }

  if (response.data.register_as.registered == false)
  {
    return -1;
  }

  return 0;
}

int WhoIs(const char *name)
{
  NameServerMessage request = (NameServerMessage){
      .type = NS_WHO_IS,
      .data = {
          .who_is = {
              .name = (char *)name,
          }}};
  NameServerResponse response;

  int returnValue = Send(
      nameserver_tid,
      (const char *)&request,
      sizeof(NameServerMessage),
      (char *)&response,
      sizeof(NameServerResponse));

  if (returnValue < 0)
  {
    return -1;
  }

  return response.data.who_is.tid;
}

void NameServerTask()
{
  nameserver_tid = MyTid();

  for (;;)
  {
    // do shit
  }
}

void NameServerTaskInit()
{
  nameserver_map = hashmap_init();

  int tid = Create(2, &NameServerTask);

  if (tid < 0)
  {
    LOG_ERROR("Name server task creation did not work");
  }
}