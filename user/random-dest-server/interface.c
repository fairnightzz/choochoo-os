#include "interface.h"
#include "stdlib.h"

int StartRandomRouting(int tid) {
  RandDestRequest request = (RandDestRequest) {
    .type = START_ROUTING,
  };
  RandDestResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(RandDestRequest), (char *)&response, sizeof(RandDestResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing start returned negative value");
  }
  return returnValue;
}


int EndRandomRouting(int tid) {
  RandDestRequest request = (RandDestRequest) {
    .type = END_ROUTING,
  };
  RandDestResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(RandDestRequest), (char *)&response, sizeof(RandDestResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing quit returned negative value");
  }
  return returnValue;
}
