#ifndef __ASM_UTIL_H__
#define __ASM_UTIL_H__
#include <stdint.h>

extern uint32_t get_esr_el1();

extern void call_wfi();

#endif // __ASM_UTIL_H__