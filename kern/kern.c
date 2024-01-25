#include "kern.h"
#include "rpi.h"
#include "stdlib.h"
#include "addrspace.h"
#include "kalloc.h"

void init_kernel(void) {
  vector_table_initialize();
  gpio_init();
  uart_config_and_enable(CONSOLE);
  slab_init();
  pagetable_init();
}



uint32_t svc_create(uint32_t priority, void (*entrypoint)());
