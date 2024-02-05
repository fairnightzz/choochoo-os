#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer_reset_c1(void);
void timer_init_c1(void);

uint64_t timer_get(void);

#endif // __TIMER_H__
