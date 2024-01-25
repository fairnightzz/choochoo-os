#include "switchframe.h"
#include "rpi.h"
#include "stdlib.h"
#include "kalloc.h"

void switchframe_init()
{
  slab_set_block_size(SWITCH_FRAME, sizeof(SwitchFrame));
}

SwitchFrame switchframe_new(address sp, void (*entrypoint)())
{
  return (SwitchFrame){
      .x0 = 0,
      .x1 = 0,
      .x2 = 0,
      .x3 = 0,
      .x4 = 0,
      .x5 = 0,
      .x6 = 0,
      .x7 = 0,
      .x8 = 0,
      .x9 = 0,
      .x10 = 0,
      .x11 = 0,
      .x12 = 0,
      .x13 = 0,
      .x14 = 0,
      .x15 = 0,
      .x16 = 0,
      .x17 = 0,
      .x18 = 0,

      .x19 = 0,
      .x20 = 0,
      .x21 = 0,
      .x22 = 0,
      .x23 = 0,
      .x24 = 0,
      .x25 = 0,
      .x26 = 0,
      .x27 = 0,
      .x28 = 0,
      .x30 = (uint64_t)entrypoint,

      .elr_el1 = (uint64_t)entrypoint,
      .spsr_el1 = 0,
      .sp_el0 = (uint64_t)sp,
  };
}