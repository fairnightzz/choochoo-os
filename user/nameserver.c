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

  // While loop
  for (;;)
  {
    NameServerMessage message;
    NameServerResponse reply;
    int sender_tid;

    int msg_len = Receive(
        &sender_tid,
        (char *)&message,
        sizeof(NameServerMessage));

    if (msg_len < 0)
    {
      PRINT("Namespace: Error receiving");
      continue;
    }

    if (message.type == NS_REGISTER_AS)
    {
      // Register the name to the tid.
      // Don't need to worry about string destructuring as long as the user task doesn't destructure
      // TODO: copy the string
      char *name = message.data.register_as.name;
      hashmap_insert(nameserver_map, name, sender_tid);

      // Reply back success
      reply = (NameServerResponse){
          .type = NS_REGISTER_AS,
          .data = {
              .register_as = {
                  .registered = true,
              }}};

      Reply(sender_tid, (char *)&reply, sizeof(NameServerResponse));
    }
    else if (message.type == NS_WHO_IS)
    {
      char *name = message.data.who_is.name;
      int success;
      int req_tid = hashmap_get(nameserver_map, name, &success);

      reply = (NameServerResponse){
          .type = NS_WHO_IS,
          .data = {
              .who_is = {
                  .tid = -1,
              }}};

      if (success == false)
      {
        PRINT("[NAMESERVER]: Could not find name to tid");
      }
      else
      {
        reply.data.who_is.tid = req_tid;
      }

      Reply(sender_tid, (char *)&reply, sizeof(NameServerResponse));
    }
  }
}

void NameServerTaskInit()
{
  nameserver_map = hashmap_new();

  int tid = Create(2, &NameServerTask);

  if (tid < 0)
  {
    PRINT("Name server task creation did not work");
  }
}