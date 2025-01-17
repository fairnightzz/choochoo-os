#ifndef _command_parser_h_
#define _command_parser_h_ 1

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "switch.h"

typedef struct
{

  enum
  {
    TRAIN_SPEED_COMMAND,
    REVERSE_COMMAND,
    REVERSE_INITIAL_COMMAND,
    SWITCH_COMMAND,
    LIGHTS_COMMAND,
    PATH_COMMAND,
    CLEAR_COMMAND,
    RESET_TRACK_COMMAND,
    QUIT_COMMAND,
    START_RANDOMPATH_COMMAND,
    END_RANDOMPATH_COMMAND,
    START_PACMAN_COMMAND,
    END_PACMAN_COMMAND,
    ERROR_COMMAND,
  } command_type;

  union
  {
    struct
    {
      uint32_t train;
      uint32_t speed;
    } train_speed_args;

    struct
    {
      uint32_t train;
    } reverse_args;

    struct
    {
      uint32_t switch_id;
      SwitchMode switch_mode;
    } switch_args;

    struct
    {
      uint32_t train;
      bool state;
    } light_args;

    struct
    {
      uint32_t train;
      uint32_t speed;
      int offset;
      string dest_node;
    } path_args;

    struct
    {
      uint32_t pac_train;
      uint32_t ghost_1;
      uint32_t ghost_2;
      uint32_t ghost_3;
      uint32_t ghost_4;
    } pacman_args;
  } command_args;
} CommandResult;

CommandResult parse_command(string *command);

#endif /* command_parser.h */
