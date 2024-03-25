#ifndef __RANDOM_DEST_INTERFACE_H__
#define __RANDOM_DEST_INTERFACE_H__

#include <stdint.h>

#define RandomDestAddress "randint-lol"

typedef enum
{
  START_ROUTING = 1,
  END_ROUTING,
  REACHED_DESTINATION,
  NEW_DESTINATION,
} RandDestMessageType;

typedef struct
{
  RandDestMessageType type;
  char *destination;
  int train;
} RandDestRequest;

typedef struct
{
  RandDestMessageType type;
  int train;
} RandDestResponse;


int StartRandomRouting(int tid);
int EndRandomRouting(int tid);

#endif // __RANDOM_DEST_INTERFACE_H__