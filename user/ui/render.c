#include <ctype.h>

#include "render.h"
#include "kern/rpi.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "user/clock-server/interface.h"
#include "user/traindata/train_data.h"

#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
#define ANSI_CLEAR_LINE "\033[K"
#define COMMAND_LINE_HISTORY 43
#define SENSOR_LINE_HISTORY 5

static TermUIState UIState;

const int sensor_ui_pos[80][2] = {
    {7, 0}, {7, 0}, {18, 4}, {18, 4}, {10, 14}, {10, 14}, {8, 12}, {8, 12}, {6, 10}, {6, 10}, {1, 8}, {1, 8}, {5, 2}, {5, 2}, {3, 4}, {3, 4}, {35, 10}, {35, 10}, {33, 9}, {33, 9}, {35, 2}, {35, 2}, {1, 10}, {1, 10}, {1, 14}, {1, 14}, {1, 12}, {1, 12}, {38, 8}, {38, 8}, {18, 8}, {18, 8}, {34, 8}, {34, 8}, {47, 14}, {47, 14}, {24, 12}, {24, 12}, {24, 14}, {24, 14}, {24, 10}, {24, 10}, {25, 2}, {25, 2}, {30, 0}, {30, 0}, {34, 12}, {34, 12}, {38, 4}, {38, 4}, {37, 2}, {37, 2}, {48, 2}, {48, 2}, {51, 1}, {51, 1}, {51, 11}, {51, 11}, {38, 12}, {38, 12}, {37, 10}, {37, 10}, {39, 9}, {39, 9}, {34, 4}, {34, 4}, {39, 3}, {39, 3}, {44, 2}, {44, 2}, {42, 0}, {42, 0}, {48, 10}, {48, 10}, {47, 12}, {47, 12}, {44, 10}, {44, 10}, {33, 3}, {33, 3}};

const int switch_ui_pos[22][2] = {
    {10, 10}, {12, 12}, {14, 14}, {10, 2}, {42, 14}, {28, 12}, {44, 12}, {52, 10}, {52, 2}, {40, 2}, {22, 0}, {12, 0}, {32, 2}, {20, 2}, {20, 10}, {32, 10}, {40, 10}, {30, 14}, {35, 7}, {37, 7}, {37, 5}, {35, 5}};

const char switch_display[22][2] = {
    {'\\', '-'},
    {'\\', '-'},
    {'-', '\\'},
    {'/', '-'},
    {'-', '/'},
    {'-', '\\'},
    {'-', '/'},
    {'|', '/'},
    {'|', '\\'},
    {'-', '/'},
    {'-', '/'},
    {'-', '/'},
    {'-', '\\'},
    {'/', '-'},
    {'\\', '-'},
    {'-', '/'},
    {'-', '\\'},
    {'-', '\\'},
    {'|', '/'},
    {'|', '\\'},
    {'|', '/'},
    {'|', '\\'}};

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
  uart_printf(CONSOLE, "├─[console]─────────────────────────────────────────────────────────────────────┤──[pac-train-game]────────────────────────────────────────────╮\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "│                                                                               │                                                              │\r\n");
  uart_printf(CONSOLE, "├───────────────────────────────────────────────────────────────────────────────┤──────────────────────────────────────────────────────────────╯\r\n");
  uart_printf(CONSOLE, "│╭─────────────────────────────────────────────────────────────────────────────╮│\r\n"); // 112
  uart_printf(CONSOLE, "││>                                                                            ││\r\n");
  uart_printf(CONSOLE, "│╰─────────────────────────────────────────────────────────────────────────────╯│\r\n");
  uart_printf(CONSOLE, "│─[performance]─────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│ Idle Task Execution Percentage:                                               │\r\n");
  uart_printf(CONSOLE, "│─[train-system]────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│   Train #  │  Current  │ Next │  Time Err.  │  Distance Err.  │  Destination  │\r\n");
  uart_printf(CONSOLE, "│───────────────────────────────────────────────────────────────────────────────│\r\n");
  uart_printf(CONSOLE, "│     02                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│     47                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│     54                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│     55                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│     58                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│     77                                ticks            mm                     │\r\n");
  uart_printf(CONSOLE, "│─[zone-system]─────────────────────────────────────────────────────────────────┤\r\n");
  uart_printf(CONSOLE, "│     00 --       01 --       02 --       03 --       04 --       05 --         │\r\n");
  uart_printf(CONSOLE, "│     06 --       07 --       08 --       09 --       10 --       11 --         │\r\n");
  uart_printf(CONSOLE, "│     12 --       13 --       14 --       15 --       16 --       17 --         │\r\n");
  uart_printf(CONSOLE, "│     18 --       19 --       20 --       21 --       22 --       23 --         │\r\n");
  uart_printf(CONSOLE, "│     24 --       25 --       26 --       27 --       28 --       29 --         │\r\n");
  uart_printf(CONSOLE, "╰───────────────────────────────────────────────────────────────────────────────╯\r\n");

  // print for track now
  // string trackPosition = string_format("\033[%d;%dH", 9, 80);
  int row = 12;
  int column = 84;
  if (getTrackType() == 0)
  {
    uart_printf(CONSOLE, "\033[%d;%dH-------X----O---------O-------X-----------X--------\r\n", row, column);
    uart_printf(CONSOLE, "\033[%d;%dH           /         /                             X\r\n", row + 1, column);
    uart_printf(CONSOLE, "\033[%d;%dH-----X----O         O----X------O--X-X--O---X---X---O\r\n", row + 2, column);
    uart_printf(CONSOLE, "\033[%d;%dH         /         /             X     X             \\\r\n", row + 3, column);
    uart_printf(CONSOLE, "\033[%d;%dH---X-----         X               X │ X               \\\r\n", row + 4, column);
    uart_printf(CONSOLE, "\033[%d;%dH                 │                 O│O                 │\r\n", row + 5, column);
    uart_printf(CONSOLE, "\033[%d;%dH                 │                  │                  │\r\n", row + 6, column);
    uart_printf(CONSOLE, "\033[%d;%dH                 │                 O│O                 │\r\n", row + 7, column);
    uart_printf(CONSOLE, "\033[%d;%dH-X-------         X               X │ X               /\r\n", row + 8, column);
    uart_printf(CONSOLE, "\033[%d;%dH         \\         \\             X     X             /\r\n", row + 9, column);
    uart_printf(CONSOLE, "\033[%d;%dH-X----X---O         O---X-------O--X-X--O---X---X---O\r\n", row + 10, column);
    uart_printf(CONSOLE, "\033[%d;%dH           \\         \\                             X\r\n", row + 11, column);
    uart_printf(CONSOLE, "\033[%d;%dH-X------X---O         --X---O-----X---X-----O--X---\r\n", row + 12, column);
    uart_printf(CONSOLE, "\033[%d;%dH             \\               \\             /\r\n", row + 13, column);
    uart_printf(CONSOLE, "\033[%d;%dH-X--------X---O---------X-----O-----------O----X--------\r\n", row + 14, column);
    for (int i = 0; i < 80; i++)
    {
      int x = sensor_ui_pos[i][0];
      int y = sensor_ui_pos[i][1];

      string pacmanPosition = string_format("\033[%d;%dH", row + y, column + x);
      uart_printf(CONSOLE, "%s○", pacmanPosition.data);
    }

    // for (int i = 0; i < 22; i++)
    // {
    //   int x = switch_ui_pos[i][0];
    //   int y = switch_ui_pos[i][1];

    //   string switchPosition = string_format("\033[%d;%dH", row + y, column + x);
    //   uart_printf(CONSOLE, "%sC", switchPosition.data);
    // }
  }

  UIState = (TermUIState){
      .cmd_line_history = 0,
      .sensor_count = 0,
      .output_queue = new_byte_queue(),
      .console_server_tid = console_server_tid,
  };
}

void render_on_sensor(int sensor_id, const char *character)
{
  int row = 12;
  int column = 84;
  int x = sensor_ui_pos[sensor_id][0] + column;
  int y = sensor_ui_pos[sensor_id][1] + row;

  string position = string_format("\033[%d;%dH", y, x);
  print("%s%s", position.data, character);
}

void render_empty_food(int sensor_id)
{
  render_on_sensor(sensor_id, "○");
}

void render_food(int sensor_id)
{
  render_on_sensor(sensor_id, "●");
}

void render_pacman(int sensor_id)
{
  render_on_sensor(sensor_id, "<");
}

void render_ghost(int sensor_id)
{
  render_on_sensor(sensor_id, "ᗣ");
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
  if (sensor_id == -1)
  {
    sensorString[1] = '-';
    sensorString[2] = '-';
    sensorString[3] = '-';
    return to_string(sensorString);
  }
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
  print("\033[%u;%uH%d%%", 22 + 35, 35, percentage);
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

  print("\033[%u;%uH%s", 26 + 35, 6, trainString.data);
}

void render_predict_current_sensor(int train, int sensor_id)
{
  int train_index = get_train_index(train);
  string sensorString = get_sensor_string(sensor_id);
  print("\033[%u;%uH%s", 26 + 35 + train_index, 18, sensorString.data);
}

void render_predict_next_sensor(int train, int sensor_id)
{
  int train_index = get_train_index(train);
  string sensorString = get_sensor_string(sensor_id);
  print("\033[%u;%uH%s", 26 + 35 + train_index, 28, sensorString.data);
}

void render_reserve_zone(int train, int zone_id)
{
  int col = (10) + (zone_id % 6) * 12;
  int row = (26 + 35 + 7) + zone_id / 6;

  char trainString[3];
  i2a(train, trainString);
  if (train <= 9)
  {
    print("\033[%u;%uH0%s", row, col, trainString);
  }
  else
  {
    print("\033[%u;%uH%s", row, col, trainString);
  }
}

void render_unreserve_zone(int zone_id)
{
  int col = (10) + (zone_id % 6) * 12;
  int row = (26 + 35 + 7) + zone_id / 6;

  print("\033[%u;%uH%s", row, col, "--");
}

void render_predict_t_error(int train, int terr)
{
  int train_index = get_train_index(train);
  char tstring[10];
  i2a(terr, tstring);

  string toffset = to_string(tstring);

  print("\033[%u;%uH   %d ticks", 26 + 35 + train_index, 37 - toffset.length, terr);
}

void render_predict_d_error(int train, int derr)
{
  int train_index = get_train_index(train);
  char dstring[10];
  i2a(derr, dstring);

  string doffset = to_string(dstring);

  print("\033[%u;%uH   %d mm", 26 + 35 + train_index, 54 - doffset.length, derr);
}

void render_train_destination(int train, int sensor_id)
{
  int train_index = get_train_index(train);
  string sensorString = get_sensor_string(sensor_id);
  print("\033[%u;%uH%s", 26 + 35 + train_index, 71, sensorString.data);
}

void render_debug_log(int message)
{
  uart_printf(CONSOLE, "\033[%u;%uH%d", 23 + 35, 35, message);
}

void render_char(unsigned char ch, int prompt_length)
{
  if (prompt_length < 74)
  {
    char buf[2];
    buf[1] = '\0';
    buf[0] = ch;
    print("\033[%d;%dH%s", 34 + 20, prompt_length + 5, buf);
  }
}

void render_backspace(int prompt_length)
{
  if (0 < prompt_length && prompt_length <= 74)
  {
    char buf[2];
    buf[1] = '\0';
    buf[0] = ' ';
    print("\033[%d;%dH%s", 34 + 20, prompt_length + 5 - 1, buf);
  }
}

void render_prompt_clear()
{
  print("\033[54;5H                                                                           ");
}

void clear_console()
{
  UIState.cmd_line_history = 0;
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

  int r = 12;
  int c = 84;
  int x = switch_ui_pos[position_id][0] + c;
  int y = switch_ui_pos[position_id][1] + r;

  string switchPosition = string_format("\033[%d;%dH", y, x);
  int switch_display_type = 0;
  if (switch_mode == SWITCH_MODE_C)
  {
    switch_display_type = 1;
  }
  char character_data[2] = {switch_display[position_id][switch_display_type], '\0'};
  print("%s%s", switchPosition.data, character_data);
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
