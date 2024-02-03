#include "interface.h"

int Signup(int server)
{
  RPSMessage request = (RPSMessage){
      .type = RPS_SIGNUP,
      .move = MOVE_NONE,
  };
  RPSResponse response;

  PRINT("Player %d signing up", MyTid());

  int returnValue = Send(
      server,
      (const char *)&request,
      sizeof(RPSMessage),
      (char *)&response,
      sizeof(RPSResponse));
  return 0;
}