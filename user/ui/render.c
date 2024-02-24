#include <ctype.h>

#include "render.h"
#include "kern/rpi.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"

#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
#define ANSI_CLEAR_LINE "\033[K"
#define COMMAND_LINE_HISTORY 8
#define SENSOR_LINE_HISTORY 5

static TermUIState UIState;

void render_init()
{
  int console_server_tid = WhoIs(ConsoleIOAddress);
  uart_printf(CONSOLE, "%s%s%s", ANSI_CLEAR, ANSI_ORIGIN, ANSI_HIDE);

  // Draw part of UI that will not be re-rendered.
  uart_printf(CONSOLE, "╭───────────────────────────────────────────────────────────────────────────────╮\r\n");
  uart_printf(CONSOLE, "│  Current Time:                                        CS 452 - Anish Aggarwal │\r\n");
  uart_printf(CONSOLE, "├─[switches]────────────────────────────────┬─[sensors]─────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│ 001 .    002 .    003 .    004 .    005 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 006 .    007 .    008 .    009 .    010 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 011 .    012 .    013 .    014 .    015 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 016 .    017 .    018 .    153 .    154 . │                                   │\r\n");
  uart_printf(CONSOLE, "│ 155 .    156 .                            │                                   │\r\n");
  uart_printf(CONSOLE, "│──[console]────────────────────────────────────────────────────────────────────┤\r\n");
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
  uart_printf(CONSOLE, "│ Main Loop Time:                                                               │\r\n");
  uart_printf(CONSOLE, "│ Sensor Loop Time:                                                             │\r\n");
  uart_printf(CONSOLE, "│ Sensor First Byte Loop Time:                                                  │\r\n");
  uart_printf(CONSOLE, "╰───────────────────────────────────────────────────────────────────────────────╯\r\n");

  UIState = (TermUIState){
      .cmd_line_history = 0,
      .sensor_count = 0,
      .output_queue = new_byte_queue(),
      .console_server_tid = console_server_tid,
  };
}

void print(char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  string formattedString = string_format(fmt, va);
  va_end(va);

  for (int i = 0; i < str_length(&formattedString); i++)
  {
    push(&UIState.output_queue, formattedString.data[i]);
  }

  while (!isEmpty(&UIState.output_queue))
  {
    uint8_t ch = pop(&UIState.output_queue);
    Putc(UIState.console_server_tid, ch);
  }
}

void render_time(uint64_t time)
{
  unsigned int tenth_secs = time % 1000000 / 100000;
  unsigned int total_secs = time / 1000000;
  unsigned int secs = total_secs % 60;
  unsigned int min = total_secs / 60;

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

  string clockPosition = string_format("\033[%d;%dH", 3, 2);
  string clockMessage = string_format("Clock: %s%u:%s%u:%u0", single_min, min, single_secs, secs, tenth_secs);
  print("%s%s", clockPosition.data, clockMessage.data);
}

void render_perf_stats(int percentage)
{
  print("Idle Task Execution: %d percent", percentage);
}

void render_prompt(string *prompt)
{
  const char *out = get_data(prompt);
  if (str_length(prompt) > 74)
  {
    string strippedString = get_suffix(prompt, 74);
    out = get_data(&strippedString);
  }
  const char *COMMAND_START = ANSI_MOVE("19", "5");
  print("%s                                                                           ", COMMAND_START);
  print("%s%s", COMMAND_START, out);
}

void clear_console()
{
  for (int i = 0; i < COMMAND_LINE_HISTORY; ++i)
  {
    print("\033[%u;2H                                                                           ", 10 + i);
  }
}

void render_command(string *command)
{
  if (UIState.cmd_line_history >= COMMAND_LINE_HISTORY)
  {
    clear_console();
    UIState.cmd_line_history = 0;
  }

  print("\033[%u;3H%s", 10 + UIState.cmd_line_history, command->data);

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
