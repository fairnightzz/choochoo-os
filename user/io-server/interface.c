#include "interface.h"
#include "stdlib.h"

int Getc(int tid)
{
  IORequest request = (IORequest){
      .type = IO_GETC,
  };
  IOResponse response;

  int returnValue = Send(tid, (const char *)&request, sizeof(IORequest), (char *)&response, sizeof(IOResponse));
  if (returnValue < 0)
  {
    LOG_DEBUG("tid %d, Getc()'s returned a negative value", MyTid());
    return -1;
  }

  return response.data;
}

int Putc(int tid, unsigned char ch)
{
  IORequest request = (IORequest){
      .type = IO_PUTC,
      .data = ch,
  };
  IOResponse response;

  int returnValue = Send(tid, (const char *)&request, sizeof(IORequest), (char *)&response, sizeof(IOResponse));
  if (returnValue < 0)
  {
    LOG_DEBUG("tid %d, Putc()'s returned a negative value", MyTid());
    return -1;
  }

  return 0;
}