#include "user/nameserver.h"
#include "io_marklin.h"
#include "user/io-server/interface.h"

void io_marklin_init(int io_server)
{
    Putc(io_server, 192);
}

void io_marklin_set_train(int io_server, int train, int speed)
{
    char s[] = {speed, train};
    Puts(io_server, s, countof(s));
}


void io_marklin_reverse_train_0(int io_server, int train, int speed, int zero_speed)
{
    char s[] = {zero_speed, train, speed, train};
    Puts(io_server, s, countof(s));
}

/*
void io_marklin_stop_train(int io_server, int train, int speed)
{
    io_marklin_set_train(io_server, train, speed);
    io_marklin_set_train(io_server, train, speed);
}
*/
void io_marklin_set_switch(int io_server, int switch_id, SwitchMode mode)
{
    char s[] = {mode, switch_id, 32};
    Puts(io_server, s, countof(s));
}

void io_marklin_dump_sensors(int io_server, uint8_t count)
{
    Putc(io_server, 128 + count);
}

void io_marklin_get_sensor(int io_server, uint8_t index)
{
    Putc(io_server, 192 + index);
}

void io_marklin_go(int io_server)
{
    char s[] = {96, 96};
    Puts(io_server, s, countof(s));
}

void io_marklin_stop(int io_server)
{
    char s[] = {97, 97};
    Puts(io_server, s, countof(s));
}
