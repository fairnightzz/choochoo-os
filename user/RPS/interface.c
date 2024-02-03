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

  if (ret < 0)
  {
    PRINT("Player %d signup failed.", MyTid());
    return -1;
  }

  PRINT("Player %d signed up.", MyTid());

  return 0;
}

RPSResult
Play(Tid server, RPSMove move)
{
  RPSMessage request = (RPSMessage){
      .type = RPS_PLAY,
      .move = move,
  };
  RPSResponse response;

  PRINT("Player %d Playing move %s", MyTid(), MOVE_STRING[request.move]);

  int returnValue = Send(
      server,
      (const char *)&request,
      sizeof(RPSMsg),
      (char *)&response,
      sizeof(RPSResponse));

  if (returnValue < 0)
  {
    PRINT("Player %d Play()'s Send() call returned a negative value", MyTid());
    return -1;
  }
  PRINT("Player %d got a %s", MyTid(), RESULT_STRING[response.res]);

  return response.res;
}

int Quit(int server)
{
  RPSMessage request = (RPSMessage){
      .type = RPS_QUIT,
  };
  RPSResponse response;

  PRINT("Player %d quitting", MyTid());

  int ret = Send(server, (const char *)&request, sizeof(RPSMessage), (char *)&response, sizeof(RPSResponse));
  if (ret < 0)
  {
    PRINT("Player %d Quit()'s Send() call returned a negative value", MyTid());
    return -1;
  }

  PRINT("Player %d quit successfully", MyTid());

  return 0;
}
