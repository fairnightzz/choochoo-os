#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

extern int Create(int priority, void (*function)());
extern int MyTid();
extern int MyParentTid();
extern void Yield();
extern void Exit();
extern int Send(int tid, const char *msg, int msglen, char *reply, int replylen);
extern int Receive(int *tid, char *msg, int msglen);
extern int Reply(int tid, const char *reply, int replylen);
extern int AwaitEvent(int eventType);

#endif // __SYSCALL_H__
