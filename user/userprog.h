#ifndef __USERPROG_H__
#define __USERPROG_H__

void OtherTask();
void FirstUserTask();

void USER_TASK_WRAPPER(void (*entrypoint)());

#endif // __USERPROG_H__