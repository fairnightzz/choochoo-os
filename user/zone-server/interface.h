#ifndef __ZONE_INTERFACE_H__
#define __ZONE_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"
#include <stdbool.h>

#define ZONE_COUNT 32
#define ZoneAddress "ZONE-SERVER"
typedef enum
{
  ZONE_RESERVE,
  ZONE_UNRESERVE,
  ZONE_UNRESERVE_ALL,
  ZONE_UNRESERVE_ALL_EXCEPT,
  ZONE_IS_RESERVED,
  ZONE_WAIT,
  ZONE_WAIT_CHANGE, // wait for a change in zone reservation
  ZONE_DEADLOCK,
} ZoneRequestType;

typedef struct
{
  ZoneRequestType type;
  int train;
  int zone;
  int unreserve_all; // train id
} ZoneRequest;

typedef struct
{
  ZoneRequestType type;
  bool reserve;
  bool unreserve;
  bool is_reserved;
  bool time_out;
} ZoneResponse;

bool zone_reserve(int zone_server, int train, int zone);
bool zone_unreserve(int zone_server, int train, int zone);
void zone_unreserve_all(int zone_server, int train);
bool zone_wait(int zone_server, int train, int zone); // wait for a given zone to be free
bool zone_is_reserved(int zone_server, int train, int zone);
void zone_wait_change(int zone_server);
void zone_unreserve_all_except(int zone_server, int train, int dest_zone);

#endif // __ZONE_INTERFACE_H__