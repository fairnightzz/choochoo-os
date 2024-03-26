#ifndef __RAND_H__
#define __RAND_H__

#include <stdint.h>

void rand_init(void);
uint32_t rand_int(void);
uint32_t get_rand_seed(void);

#endif // __RAND_H__