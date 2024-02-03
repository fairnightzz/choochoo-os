#include "client.h"
#include "interface.h"
#include "user/nameserver.h"

// Plays only rock!
void RPSClientTask1()
{
  int server = WhoIs(RPSAddress);
  Signup(server);

  Play(server, MOVE_ROCK);
  Play(server, MOVE_ROCK);
  Play(server, MOVE_ROCK);

  Quit(server);
}

// Plays in a certain order
void RPSClientTask2()
{
  int server = WhoIs(RPSAddress);
  Signup(server);

  Play(server, MOVE_ROCK);
  Play(server, MOVE_PAPER);
  Play(server, MOVE_SCISSORS);
  Play(server, MOVE_ROCK);
  Play(server, MOVE_PAPER);
  Play(server, MOVE_SCISSORS);

  Quit(server);
}

// Plays one move, quits and then signs up to play another move
void RPSClientTask3()
{
  int server = WhoIs(RPSAddress);
  Signup(server);
  Play(server, MOVE_ROCK);
  Quit(server);

  Signup(server);
  Play(server, MOVE_PAPER);
  Quit(server);
}