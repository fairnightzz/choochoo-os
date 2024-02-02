#include "stdlib.h"
#include <stdbool.h>

typedef struct NameServerEntry NameServerEntry;

typedef enum
{
  NS_REGISTER_AS,
  NS_WHO_IS
} NameServerMessageType;

typedef struct
{
  NameServerMessageType type;

  union
  {
    struct
    {
      char *name;
    } register_as;

    struct
    {
      char *name;
    } who_is;
  } data;
} NameServerMessage;

typedef struct
{
  NameServerMessageType type;

  union
  {
    struct
    {
      bool registered;
    } register_as;

    struct
    {
      int tid;
    } who_is;
  } data;
} NameServerResponse;

struct NameServerEntry
{
  char *name;
  int tid;
};

void NameServerTask();

int RegisterAs(const char *name);

int WhoIs(const char *name);

void NameServerTaskInit();