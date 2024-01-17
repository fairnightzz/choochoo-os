#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);
extern int Send(int tid, const char* msg, int msglen, char* reply, int rplen);
extern int Receive(int* tid, char* msg, int msglen);
extern int Reply(int tid, const char* reply, int rplen);


#endif // __SYSCALL_H__
