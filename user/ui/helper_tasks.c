#include "helper_tasks.h"
#include "lib/stdlib.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/clock-server/interface.h"
#include <ctype.h>
#include "user/ui/render.h"
#include "user/trainsys/trainsys.h"
#include "user/io-server/io_marklin.h"

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
  case RESET_TRACK_COMMAND:
  {
    return to_string("[rt]: Resetting track");
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
  case REVERSE_INITIAL_COMMAND:
  {
    uint32_t train = cres.command_args.reverse_args.train;
    return string_format("[rvi]: Reversing initial direction for train #%u", train);
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
    // render_switch(switch_id, switch_mode);
    return string_format("[sw]: Setting switch %u to %s mode", switch_id, out_mode);
  }
  case PATH_COMMAND:
  {
    uint32_t train = cres.command_args.path_args.train;
    uint32_t speed = cres.command_args.path_args.speed;
    string dest_node = cres.command_args.path_args.dest_node;
    return string_format("[path]: Pathing train #%u with speed %u to node %s", train, speed, dest_node.data);
  }
  case CLEAR_COMMAND:
  {
    return string_format("");
  }
  case START_RANDOMPATH_COMMAND:
  {
    return string_format("[srp]: Start random pathing, seed = %d", get_rand_seed());
  }
  case END_RANDOMPATH_COMMAND:
  {
    return string_format("[erp]: End random pathing");
  }
  case START_PACMAN_COMMAND:
  {
    uint32_t pac_train = cres.command_args.pacman_args.pac_train;
    uint32_t ghost_1 = cres.command_args.pacman_args.ghost_1;
    uint32_t ghost_2 = cres.command_args.pacman_args.ghost_2;
    uint32_t ghost_3 = cres.command_args.pacman_args.ghost_3;
    return string_format("[spm]: Start Pacman Game: PM: %d G1: %d G2: %d G3: %d", pac_train, ghost_1, ghost_2, ghost_3);
  }
  case END_PACMAN_COMMAND:
  {
    return string_format("[epm]: End Pacman Game");
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
  // int marklin_server = WhoIs(MarklinIOAddress);
  int clock_server = WhoIs(ClockAddress);

  string prompt = new_string();

  trainsys_init_track(TRACK_A);
  trainsys_init_trains();

  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    int switch_id = i + 1;
    if (switch_id > 18)
    {
      switch_id += 134;
    }

    render_switch(switch_id, TRACK_PLANS[TRACK_A][i]);
  }

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
    if (isalnum(ch) || isblank(ch) || ch == '-')
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
      render_command(console_string.data);
      str_clear(&prompt);
      render_prompt_clear();

      trainsys_execute_command(command_result); // todo make sure this fn exists and works
      if (command_result.command_type == QUIT_COMMAND)
      {
        clear_screen();
      }
      else if (command_result.command_type == ERROR_COMMAND)
      {
        // io_marklin_dump_sensors(WhoIs(MarklinIOAddress), 5);
      }
    }
  }
}
