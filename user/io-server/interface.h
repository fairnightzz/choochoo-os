#ifndef __IO_INTERFACE_H__
#define __IO_INTERFACE_H__

#include <stdint.h>

#define MarklinIOAddress "MARKLIN-IO"
#define ConsoleIOAddress "CONSOLE-IO"
#define PUTS_BLOCK_SIZE 8

typedef enum
{
    IO_GETC = 1,
    IO_PUTC,
    IO_PUTS,
    IO_RECEIVE_EVENT,
    IO_SEND_EVENT,
} IORequestType;

typedef struct
{
    IORequestType type;
    union {
      struct { unsigned char ch; } putc;
      struct {
        unsigned char chs[PUTS_BLOCK_SIZE];
        int chs_len;
      } puts;
    } data;
} IORequest;

typedef struct
{
    IORequestType type;
    unsigned char data; // in case of type being IO_GETC
} IOResponse;

int Getc(int tid);
int Putc(int tid, unsigned char ch);
int Puts(int tid, unsigned char *ch, int len);

#endif // __IO_INTERFACE_H__