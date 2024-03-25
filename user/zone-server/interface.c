#include "interface.h"

bool zone_reserve(int zone_server, int train, int zone)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_RESERVE,
      .train = train,
      .zone = zone,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone reserve failed");
  }

  return response.reserve;
}

bool zone_unreserve(int zone_server, int train, int zone)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_UNRESERVE,
      .train = train,
      .zone = zone,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone unreserve failed");
  }

  return response.unreserve;
}

void zone_unreserve_all(int zone_server, int train)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_UNRESERVE_ALL,
      .unreserve_all = train,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone unreserve all failed");
  }
}

void zone_unreserve_all_except(int zone_server, int train, int target_zone)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_UNRESERVE_ALL_EXCEPT,
      .unreserve_all = train,
      .zone = target_zone,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone unreserve all except failed");
  }
}

bool zone_wait(int zone_server, int train, int zone)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_WAIT,
      .train = train,
      .zone = zone,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone wait failed");
  }

  return response.time_out;
}

bool zone_is_reserved(int zone_server, int train, int zone)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_IS_RESERVED,
      .train = train,
      .zone = zone,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone is reserved failed");
  }

  return response.is_reserved;
}

void zone_wait_change(int zone_server)
{
  ZoneResponse response;
  ZoneRequest request = (ZoneRequest){
      .type = ZONE_WAIT_CHANGE,
  };

  int returnValue = Send(zone_server, (const char *)&request, sizeof(ZoneRequest), (char *)&response, sizeof(ZoneResponse));

  if (returnValue < 0)
  {
    LOG_WARN("Zone wait change failed");
  }
}