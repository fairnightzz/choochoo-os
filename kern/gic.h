#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>

void gic_target_and_enable(uint32_t interruptId);
uint32_t gic_read_iar();
void gic_write_eoir(uint32_t id);

#endif // __GIC_H__
