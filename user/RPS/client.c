#include "client.h"

void RPSClientTask1()
{
  Tid rps = WhoIs(RPS_ADDRESS);
  Signup(rps);

  Play(rps, MOVE_ROCK);
  Play(rps, MOVE_ROCK);
  Play(rps, MOVE_ROCK);

  Quit(rps);
  Exit();
}

void RPSClientTask2()
{
  Tid rps = WhoIs(RPS_ADDRESS);
  Signup(rps);

  Play(rps, MOVE_SCISSORS);
  Play(rps, MOVE_PAPER);
  Play(rps, MOVE_ROCK);
  Play(rps, MOVE_PAPER);
  Play(rps, MOVE_SCISSORS);

  Quit(rps);
  Exit();
}

void RPSClientTask3()
{
  Tid rps = WhoIs(RPS_ADDRESS);
  Signup(rps);
  Play(rps, MOVE_ROCK);
  Quit(rps);

  Signup(rps);
  Play(rps, MOVE_SCISSORS);
  Quit(rps);

  Exit();
}