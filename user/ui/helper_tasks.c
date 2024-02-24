#include "helper_tasks.h"
#include "lib/stdlib.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/clock-server/interface.h"
#include <ctype.h>
#include "ui/render.h"

#define ENTER_CHARACTER     0x0d
#define BACKSPACE_CHARACTER 0x08

string cres_to_string(CommandResult cres) {
  switch (cres.command_type) {
    case TRAIN_SPEED_COMMAND: {
      uint32_t train = cres.command_args.train_speed_args.train;
      uint32_t speed = cres.command_args.train_speed_args.speed;
      return string_format("[tr]: Setting speed for train #%u to %u", train, speed);
    } case QUIT_COMMAND: {
      return to_string("[q]: Exiting...");
    } case LIGHTS_COMMAND: {
      uint32_t train = cres.command_args.light_args.train;
      bool state = cres.command_args.light_args.state;
      if (state) {
        return string_format("[light]: Turning lights ON for train #%u", train);
      } else {
        return string_format("[light]: Turning lights OFF for train #%u", train);
      }
    } case REVERSE_COMMAND: {
      uint32_t train = cres.command_args.reverse_args.train;
      return string_format("[rv]: Reversing direction for train #%u", train);
    } case SWITCH_COMMAND: {
      uint32_t switch_id = cres.command_args.switch_args.switch_id;
      SwitchMode switch_mode = cres.command_args.switch_args.switch_mode;
      char *out_mode = "STRAIGHT";
      if (switch_mode != SWITCH_MODE_S) {
        out_mode = "CURVED";
      }
      return string_format("[sw]: Setting switch %u to %s mode", switch_id, out_mode);
    } default: {
      return to_string("[ERROR]: Invalid Command. Please Try Again.");
    }
  }
}


void promptTask() {
  int io_server = WhoIs(ConsoleIOAddress);
  int marklin_server = WhoIs(MarklinIOAddress);

  string prompt = new_string();

  Putc(marklin_server, 192); // reset sensors

  while (1) {
    int c = Getc(io_server);
    if (c < 0) {
      LOG_WARN("[Getc Error in promptTask()]: got %d", c);
      continue;
    }

    unsigned char ch = c;
    if (isalnum(ch) || isblank(ch)) {
      push_char(&prompt, ch);
      render_prompt(&prompt);
    } else if (c == BACKSPACE_CHARACTER) {
      pop_char(&prompt);
      render_prompt(&prompt);
    } else if (c == ENTER_CHARACTER) {
      CommandResult command_result = parse_command(&prompt);
      string console_string = cres_to_string(command_result);
      render_command(&console_string);
      str_clear(&prompt);
      render_prompt(&prompt);

      execute_command(command_result); // todo make sure this fn exists and works
    }
  }
}

void sensorTask() {
  int clock_server = WhoIs(ClockAddress);
  while (1) {
    read_all_sensors(Time(clock_server));
    Delay(clock_server, 20);
  }
}