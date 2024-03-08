#include <ctype.h>

#include "render.h"
#include "kern/rpi.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"

#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
#define ANSI_CLEAR_LINE "\033[K"
#define COMMAND_LINE_HISTORY 23
#define SENSOR_LINE_HISTORY 5

static TermUIState UIState;

void print(char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  string formattedString = _string_format(fmt, va);
  va_end(va);

  // uart_printf(CONSOLE, formattedString.data);

  // for (int i = 0; i < str_length(&formattedString); i++)
  // {
  //   if (!push(&UIState.output_queue, formattedString.data[i]))
  //   {
  //     PRINT("BUFFER OVERFLOW");
  //   }
  // }

  // while (!isEmpty(&UIState.output_queue))
  // {
  //   uint8_t ch = pop(&UIState.output_queue);
  //   Putc(UIState.console_server_tid, ch);
  //   if (length(&UIState.output_queue) % 5 == 0)
  //   {
  //     Delay(WhoIs(ClockAddress), 1);
  //   }
  // }
  Puts(UIState.console_server_tid, formattedString.data, formattedString.length);
}

void clear_screen()
{
  uart_printf(CONSOLE, "%s%s%s", ANSI_CLEAR, ANSI_ORIGIN, ANSI_HIDE);
}

void render_init()
{
  int console_server_tid = WhoIs(ConsoleIOAddress);
  uart_printf(CONSOLE, "%s%s%s", ANSI_CLEAR, ANSI_ORIGIN, ANSI_HIDE);

  // Draw part of UI that will not be re-rendered.
  uart_printf(CONSOLE, "╭───────────────────────────────────────────────────────────────────────────────╮\r\n");
  uart_printf(CONSOLE, "│ Current Time:                                         CS 452 - choochoo os    │\r\n");
  uart_printf(CONSOLE, "├─[switches]────────────────────────────────┬─[sensors]─────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│ 001 .    002 .    003 .    004 .    005 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 006 .    007 .    008 .    009 .    010 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 011 .    012 .    013 .    014 .    015 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 016 .    017 .    018 .    153 .    154 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 155 .    156 .                            │                                   │\r\n");
  uart_printf(CONSOLE, "├─[console]─────────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│                                                                               │\r\n");
  uart_printf(CONSOLE, "│╭─────────────────────────────────────────────────────────────────────────────╮│\r\n");
  uart_printf(CONSOLE, "││>                                                                            ││\r\n");
  uart_printf(CONSOLE, "│╰─────────────────────────────────────────────────────────────────────────────╯│\r\n");
  uart_printf(CONSOLE, "│─[performance]─────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│ Idle Task Execution Percentage:                                               │\r\n");
  uart_printf(CONSOLE, "│─[train-system]────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│   Train #  │  Current Sensor  │  Next Sensor  │  Time Err.  │  Distance Err.  │\r\n");
  uart_printf(CONSOLE, "│───────────────────────────────────────────────────────────────────────────────│\r\n");
  uart_printf(CONSOLE, "│                                                       ticks            mm     │\r\n");
  uart_printf(CONSOLE, "╰───────────────────────────────────────────────────────────────────────────────╯\r\n");

  UIState = (TermUIState){
      .cmd_line_history = 0,
      .sensor_count = 0,
      .output_queue = new_byte_queue(),
      .console_server_tid = console_server_tid,
  };
}

// every tick is 10ms
void render_time(int time)
{
  int tenth_secs = time % 100 / 10;
  int total_secs = time / 100;
  int secs = total_secs % 60;
  int min = total_secs / 60;

  char *single_secs = "";
  char *single_min = "";
  if (secs < 10)
  {
    single_secs = "0";
  }
  if (min < 10)
  {
    single_min = "0";
  }

  string clockPosition = string_format("\033[%d;%dH", 2, 17);
  string clockMessage = string_format("%s%u:%s%u:%u0", single_min, min, single_secs, secs, tenth_secs);
  print("%s%s", clockPosition.data, clockMessage.data);
}

string get_sensor_string(int sensor_id)
{
  int sensor_group = sensor_id / 16;
  int sensor_index = (sensor_id % 16) + 1;

  char sensorString[4] = {0};
  sensorString[0] = sensor_group + 'A';
  sensorString[3] = '\0';
  if (sensor_index < 10)
  {
    sensorString[1] = '0';
    sensorString[2] = '0' + sensor_index;
  }
  else
  {
    sensorString[1] = '1';
    sensorString[2] = '0' + (sensor_index % 10);
  }

  return to_string(sensorString);
}

void render_perf_stats(int percentage)
{
  print("\033[%u;%uH%d%%", 22 + 15, 35, percentage);
}

void render_train_system_train(int train)
{
  string trainString;
  if (train < 10)
  {
    trainString = string_format("0%d", train);
  }
  else
  {
    trainString = string_format("%d", train);
  }

  print("\033[%u;%uH%s", 26 + 15, 6, trainString.data);
}

void render_predict_current_sensor(int sensor_id)
{
  string sensorString = get_sensor_string(sensor_id);
  print("\033[%u;%uH%s", 26 + 15, 22, sensorString.data);
}
void render_predict_next_sensor(int sensor_id)
{
  string sensorString = get_sensor_string(sensor_id);
  print("\033[%u;%uH%s", 26 + 15, 39, sensorString.data);
}
void render_predict_error(int terr, int derr)
{

  char tstring[10];
  i2a(terr, tstring);
  char dstring[10];
  i2a(derr, dstring);

  string toffset = to_string(tstring);
  string doffset = to_string(dstring);

  print("\033[%u;%uH    ", 26 + 15, 52, terr);
  print("\033[%u;%uH    ", 26 + 15, 69, derr);
  print("\033[%u;%uH%d", 26 + 15, 56 - toffset.length, terr);
  print("\033[%u;%uH%d", 26 + 15, 73 - doffset.length, derr);
}

void render_debug_log(int message)
{
  uart_printf(CONSOLE, "\033[%u;%uH%d", 23 + 15, 35, message);
}

void render_char(unsigned char ch, int prompt_length)
{
  if (prompt_length < 74)
  {
    char buf[2];
    buf[1] = '\0';
    buf[0] = ch;
    print("\033[34;%dH%s", prompt_length + 5, buf);
  }
}

void render_backspace(int prompt_length)
{
  if (0 < prompt_length && prompt_length <= 74)
  {
    char buf[2];
    buf[1] = '\0';
    buf[0] = ' ';
    print("\033[34;%dH%s", prompt_length + 5 - 1, buf);
  }
}

void render_prompt_clear()
{
  print("\033[34;5H                                                                           ");
}

void clear_console()
{
  for (int i = 0; i < COMMAND_LINE_HISTORY; ++i)
  {
    print("\033[%u;2H                                                                             ", 10 + i);
  }
}

void render_command(char *fmt, ...)
{

  va_list va;
  va_start(va, fmt);
  string command = _string_format(fmt, va);
  va_end(va);
  if (UIState.cmd_line_history >= COMMAND_LINE_HISTORY)
  {
    clear_console();
    UIState.cmd_line_history = 0;
  }

  print("\033[%u;3H%s", 10 + UIState.cmd_line_history, command.data);

  UIState.cmd_line_history += 1;
}

void clear_sensor_ui()
{
  for (int i = 0; i < SENSOR_LINE_HISTORY; ++i)
  {
    print("\033[%u;47H                                  ", 4 + i);
  }
}

void render_switch(int32_t switch_id, SwitchMode switch_mode)
{
  int position_id = switch_id - 1;
  if (position_id > 18)
  {
    position_id -= 134; // make 153-156 -> 19->21
  }

  int col = 7 + (position_id % 5) * 9;
  int row = 4 + position_id / 5;

  char *out = "S";
  if (switch_mode == SWITCH_MODE_C)
  {
    out = "C";
  }
  else if (switch_mode == SWITCH_MODE_UNKNOWN)
  {
    out = "X";
  }
  print("\033[%u;%uH%s", row, col, out);
}

void render_sensor(char bank, unsigned int sensor_number)
{
  char sensorString[4] = {0};
  sensorString[0] = bank;
  sensorString[3] = '\0';
  if (sensor_number < 10)
  {
    sensorString[1] = '0';
    sensorString[2] = '0' + sensor_number;
  }
  else
  {
    sensorString[1] = '1';
    sensorString[2] = '0' + (sensor_number % 10);
  }

  if (UIState.sensor_count == 30)
  {
    clear_sensor_ui();
    UIState.sensor_count = 0;
  }
  int row = 4 + UIState.sensor_count / 6;
  int col = 47 + (UIState.sensor_count % 6) * 6;
  print("\033[%u;%uH%s", row, col, sensorString);
  UIState.sensor_count += 1;
}
