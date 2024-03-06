#include "marklin.h"
#include "rpi.h"

void Putc(uint8_t byte)
{
  uart_putc(MARKLIN, byte);
}

// Send the train info and then the train number
void marklin_train_control(uint8_t info, uint8_t train)
{
  Putc(info);
  Putc(train);
  delay(100000);
}

// Send the switch type and then the switch number
void marklin_switch_control(SwitchMode mode, uint8_t switch_num)
{
  Putc(mode);
  Putc(switch_num);
  Putc(32); // reset command
  delay(100000);
}

// Read everything
void marklin_dump_s88_all()
{
  Putc(128 + 5);
  delay(100000);
}

void marklin_pick_s88(uint32_t index)
{
  Putc(192 + index);
  delay(100000);
}
