#include "interface.h"
#include "stdlib.h"

int Signup(int server)
{
  RPSRequest request = (RPSRequest){
      .type = RPS_SIGNUP,
      .move = MOVE_NONE,
  };
  RPSResponse response;

  PRINT("Player %d signing up", MyTid());

  int returnValue = Send(
      server,
      (const char *)&request,
      sizeof(RPSRequest),
      (char *)&response,
      sizeof(RPSResponse));

  if (returnValue < 0)
  {
    PRINT("Player %d signup failed.", MyTid());
    return -1;
  }

  PRINT("Player %d signed up.", MyTid());

  return 0;
}

RPSResult
Play(int server, RPSMove move)
{
  RPSRequest request = (RPSRequest){
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
  RPSRequest request = (RPSRequest){
      .type = RPS_QUIT,
  };
  RPSResponse response;

  PRINT("Player %d quitting", MyTid());

  int ret = Send(server, (const char *)&request, sizeof(RPSRequest), (char *)&response, sizeof(RPSResponse));
  if (ret < 0)
  {
    PRINT("Player %d Quit()'s Send() call returned a negative value", MyTid());
    return -1;
  }

  PRINT("Player %d quit successfully", MyTid());

  return 0;
}
