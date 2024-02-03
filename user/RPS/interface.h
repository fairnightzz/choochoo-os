#ifndef __RPS_INTERFACE_H__
#define __RPS_INTERFACE_H__
#include <stdbool.h>

#define RPSAddress "ROCK-PAPER-SCISSORS"

typedef enum
{
  RPS_SIGNUP,
  RPS_PLAY,
  RPS_QUIT,
} RPSMessageType;

typedef enum
{
  MOVE_NONE = 0,
  MOVE_ROCK = 1,
  MOVE_PAPER = 2,
  MOVE_SCISSORS = 3,
} RPSMove;

typedef enum
{
  RESULT_NONE = 0,   // Undefined behaviour
  RESULT_INCOMPLETE, // the other player quits
  RESULT_WIN,
  RESULT_LOSE,
  RESULT_TIE,
} RPSResult;

typedef struct
{
  RPSMessageType type;
  RPSMove move;
} RPSRequest;

typedef struct
{
  RPSMessageType type;
  RPSResult res;
} RPSResponse;

typedef struct
{
  bool gameComplete;
  int p1;
  int p2;
  RPSMove p1Move;
  RPSMove p2Move;
} RPSGameState;

static const char *const MOVE_STRING[4] = {
    "NONE",
    "ROCK",
    "PAPER",
    "SCISSORS"};

static const char *const RESULT_STRING[4] = {
    "OPPONENT QUIT",
    "WIN",
    "LOSE",
    "TIE"};

int Signup(int server);
RPSResult Play(int server, RPSMove move);
int Quit(int server);

#endif // __RPS_INTERFACE_H__