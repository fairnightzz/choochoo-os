#include "helper_tasks.h"
#include "lib/stdlib.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/clock-server/interface.h"
#include <ctype.h>
#include "user/ui/render.h"
#include "user/trainsys/trainsys.h"

#define ENTER_CHARACTER 0x0d
#define BACKSPACE_CHARACTER 0x08

string cres_to_string(CommandResult cres)
{
  switch (cres.command_type)
  {
  case TRAIN_SPEED_COMMAND:
  {
    uint32_t train = cres.command_args.train_speed_args.train;
    uint32_t speed = cres.command_args.train_speed_args.speed;
    return string_format("[tr]: Setting speed for train #%u to %u", train, speed);
  }
  case QUIT_COMMAND:
  {
    return to_string("[q]: Exiting...");
  }
  case LIGHTS_COMMAND:
  {
    uint32_t train = cres.command_args.light_args.train;
    bool state = cres.command_args.light_args.state;
    if (state)
    {
      return string_format("[light]: Turning lights ON for train #%u", train);
    }
    else
    {
      return string_format("[light]: Turning lights OFF for train #%u", train);
    }
  }
  case REVERSE_COMMAND:
  {
    uint32_t train = cres.command_args.reverse_args.train;
    return string_format("[rv]: Reversing direction for train #%u", train);
  }
  case SWITCH_COMMAND:
  {
    uint32_t switch_id = cres.command_args.switch_args.switch_id;
    SwitchMode switch_mode = cres.command_args.switch_args.switch_mode;
    char *out_mode = "STRAIGHT";
    if (switch_mode != SWITCH_MODE_S)
    {
      out_mode = "CURVED";
    }
    return string_format("[sw]: Setting switch %u to %s mode", switch_id, out_mode);
  }
  case PATH_COMMAND:
  {
    uint32_t train = cres.command_args.path_args.train;
    uint32_t speed = cres.command_args.path_args.speed;
    string dest_node = cres.command_args.path_args.dest_node;
    return string_format("[path]: Pathing train #%u with speed %u to node %u", train, speed, dest_node.data);
  }
  default:
  {
    return to_string("[ERROR]: Invalid Command. Please Try Again.");
  }
  }
}

void clockUITask()
{
  int clock_server = WhoIs(ClockAddress);
  while (1)
  {
    int cur_tick = Time(clock_server);
    render_time(cur_tick);
    Delay(clock_server, 5);
  }
}

void promptTask()
{
  int io_server = WhoIs(ConsoleIOAddress);
  int marklin_server = WhoIs(MarklinIOAddress);
  int clock_server = WhoIs(ClockAddress);

  string prompt = new_string();

  Putc(marklin_server, 192); // reset sensors

  while (1)
  {
    int c = Getc(io_server);
    int curr_tick = Time(clock_server);
    if (c < 0)
    {
      LOG_ERROR("[Getc Error in promptTask()]: got %d", c);
      continue;
    }
    unsigned char ch = c;
    if (isalnum(ch) || isblank(ch))
    {
      render_char(ch, str_length(&prompt));
      push_char(&prompt, ch);
    }
    else if (c == BACKSPACE_CHARACTER)
    {
      render_backspace(str_length(&prompt));
      pop_char(&prompt);
    }
    else if (c == ENTER_CHARACTER)
    {

      if (curr_tick < 0)
      {
        LOG_ERROR("[promptTask ERROR]: Time() from clock server error");
        continue;
      }

      CommandResult command_result = parse_command(&prompt);
      string console_string = cres_to_string(command_result);
      render_command(&console_string);
      str_clear(&prompt);
      render_prompt_clear();

      trainsys_execute_command(command_result, curr_tick); // todo make sure this fn exists and works
    }
  }
}

void trainsysTask()
{
  int clock_server = WhoIs(ClockAddress);
  int curr_tick = Time(clock_server);
  if (curr_tick < 0)
  {
    LOG_ERROR("[trainsysTask ERROR]: Time() from clock server error");
    return;
  }
  trainsys_init_track(TRACK_A, curr_tick);

  while (1)
  {
    curr_tick = Time(clock_server);
    if (curr_tick < 0)
    {
      LOG_ERROR("[trainsysTask ERROR]: Time() from clock server error");
      continue;
    }
    trainsys_read_all_sensors(curr_tick);
    trainsys_check_rev_trains(curr_tick);
    Delay(clock_server, 1);
  }
}

void trainsysSlave()
{
  int clock_server = WhoIs(ClockAddress);
  while (1)
  {
    int curr_tick = Time(clock_server);
    trainsys_try_serial_out(curr_tick);
    Delay(clock_server, 1);
  }
}