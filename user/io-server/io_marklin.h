#ifndef __IO_MARKLIN_H__
#define __IO_MARKLIN_H__

#include "lib/stdlib.h"

void io_marklin_init(int io_server);
void io_marklin_set_train(int io_server, int train, int speed);
void io_marklin_set_switch(int io_server, int switch_id, SwitchMode mode);
void io_marklin_dump_sensors(int io_server, uint8_t count);
void io_marklin_get_sensor(int io_server, uint8_t index);
void io_marklin_go(int io_server);
void io_marklin_stop(int io_server);
void io_marklin_reverse_train_0(int io_server, int train, int speed, int zero_speed);

#endif // __IO_MARKLIN_H__
