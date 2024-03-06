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
    .data = {
      .putc = {
        .ch = ch
      },
    },
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

int Puts(int tid, unsigned char *ch, int len) {
  IOResponse response;
  IORequest request = (IORequest) {
    .type = IO_PUTS,
    .data = {
      .puts = {
          .chs = {0},
          .chs_len = len,
      }
    }
  };

  memcpy(request.data.puts.chs, ch, min(len, PUTS_BLOCK_SIZE*sizeof(unsigned char)));

  int returnValue = Send(tid, (const char *)&request, sizeof(IORequest), (char *)&response, sizeof(IOResponse));
  if (returnValue < 0)
  {
    LOG_DEBUG("tid %d, Putc()'s returned a negative value", MyTid());
    return -1;
  }

  return 0;
}