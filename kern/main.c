#include "rpi.h"

extern void vector_table_initialize();

int kmain()
{
    gpio_init();
    vector_table_initialize();

    return 0;
}
