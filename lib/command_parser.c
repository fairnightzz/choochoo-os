#include <ctype.h>
#include <string.h>

#include "command_parser.h"

string get_str(const char *command, uint32_t *it);
void skip_whitespace(const char *command, uint32_t *it);
int get_i(const char *command, uint32_t *it);

CommandResult
parse_command(string *command)
{
  // read until first whitespace character
  uint32_t it = 0;
  const char *data = get_data(command);
  string cmd_name = get_str(data, &it);

  if (strcmp(get_data(&cmd_name), "tr") == 0)
  {

    skip_whitespace(data, &it);

    int train = get_i(data, &it);

    skip_whitespace(data, &it);

    int speed = get_i(data, &it);

    if (speed > 14 || speed < 0 || train > 80 || train < 1)
    {
      return (CommandResult){
          .command_type = ERROR_COMMAND,
      };
    }

    return (CommandResult){
        .command_type = TRAIN_SPEED_COMMAND,
        .command_args = {
            .train_speed_args = {
                .train = train,
                .speed = speed,
            }}};
  }
  else if (strcmp(get_data(&cmd_name), "rv") == 0)
  {

    skip_whitespace(data, &it);

    int train = get_i(data, &it);

    if (train > 80 || train < 1)
    {
      return (CommandResult){
          .command_type = ERROR_COMMAND,
      };
    }

    return (CommandResult){
        .command_type = REVERSE_COMMAND,
        .command_args = {
            .reverse_args = {
                .train = train,
            }}};
  }
  else if (strcmp(get_data(&cmd_name), "sw") == 0)
  {

    skip_whitespace(data, &it);

    int switch_id = get_i(data, &it);
    if (!(1 <= switch_id && switch_id <= 18) && !(153 <= switch_id && switch_id <= 156))
    {
      return (CommandResult){
          .command_type = ERROR_COMMAND,
      };
    }

    skip_whitespace(data, &it);

    string switch_mode = get_str(data, &it);

    if (strcmp(get_data(&switch_mode), "S") == 0)
    {
      return (CommandResult){
          .command_type = SWITCH_COMMAND,
          .command_args = {
              .switch_args = {
                  .switch_id = switch_id,
                  .switch_mode = SWITCH_MODE_S}},
      };
    }
    else if (strcmp(get_data(&switch_mode), "C") == 0)
    {
      return (CommandResult){
          .command_type = SWITCH_COMMAND,
          .command_args = {
              .switch_args = {
                  .switch_id = switch_id,
                  .switch_mode = SWITCH_MODE_C}},
      };
    }
    else
    {
      return (CommandResult){
          .command_type = ERROR_COMMAND,
      };
    }
  }
  else if (strcmp(get_data(&cmd_name), "light") == 0)
  {
    skip_whitespace(data, &it);

    int train = get_i(data, &it);

    if (train > 80 || train < 1)
    {
      return (CommandResult){
          .command_type = ERROR_COMMAND,
      };
    }

    skip_whitespace(data, &it);

    string light_mode = get_str(data, &it);

    if (strcmp(get_data(&light_mode), "on") == 0)
    {
      return (CommandResult){
          .command_type = LIGHTS_COMMAND,
          .command_args = {
              .light_args = {
                  .train = train,
                  .state = true,
              }},
      };
    }
    else if (strcmp(get_data(&light_mode), "off") == 0)
    {
      return (CommandResult){
          .command_type = LIGHTS_COMMAND,
          .command_args = {
              .light_args = {
                  .train = train,
                  .state = false,
              }},
      };
    }
    return (CommandResult){
        .command_type = ERROR_COMMAND,
    };
  }
  else if (strcmp(get_data(&cmd_name), "q") == 0)
  {
    return (CommandResult){
        .command_type = QUIT_COMMAND,
    };
  }

  return (CommandResult){
      .command_type = ERROR_COMMAND,
  };
}

string
get_str(const char *command, uint32_t *it)
{
  string word = new_string();
  while (1)
  {
    char c = command[*it];
    if (c == 0)
      break;

    if (isalnum(c))
    {
      push_char(&word, c);
    }
    else if (isspace(c))
    {
      break;
    }

    ++(*it);
  }
  return word;
}

int get_i(const char *command, uint32_t *it)
{
  int number = 0;
  while (1)
  {
    char c = command[*it];
    if (c == 0)
      break;

    if (!isdigit(c))
      break;

    number = number * 10 + (c - '0');

    ++(*it);
  }
  return number;
}

void skip_whitespace(const char *command, uint32_t *it)
{
  while (1)
  {
    char c = command[*it];
    if (c == 0)
      break;

    if (!isspace(c))
      break;

    ++(*it);
  }
}
