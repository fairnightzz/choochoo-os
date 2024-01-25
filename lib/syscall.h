#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

extern int Create(int priority, void (*function)());
extern int MyTid(void);
extern int MyParentTid(void);
extern void Yield(void);
extern void Exit(void);

#endif // __SYSCALL_H__
