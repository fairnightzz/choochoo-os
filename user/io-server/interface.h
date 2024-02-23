#ifndef __IO_INTERFACE_H__
#define __IO_INTERFACE_H__

#include <stdint.h>

#define MarklinIOAddress "MARKLIN-IO"
#define ConsoleIOAddress "CONSOLE-IO"

typedef enum
{
    IO_GETC = 1,
    IO_PUTC,
    IO_RECEIVE_EVENT,
    IO_SEND_EVENT,
} IORequestType;

typedef struct
{
    IORequestType type;
    unsigned char data; // in case of IO_PUTC request
} IORequest;

typedef struct
{
    IORequestType type;
    unsigned char data; // in case of type being IO_GETC
} IOResponse;

int Getc(int tid);
int Putc(int tid, unsigned char ch);

#endif // __IO_INTERFACE_H__