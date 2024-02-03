#include "server.h"
#include "stdlib.h"
#include "user/nameserver.h"

RPSResult result(RPSMove p1, RPSMove p2)
{
  if (p1 == MOVE_NONE || p2 == MOVE_NONE)
  {
    LOG_WARN("[RPS Result]: one of the player's moves is none");
    return RESULT_INCOMPLETE;
  }

  if (p1 == p2)
    return RESULT_TIE;

  if (p1 == MOVE_ROCK && p2 == MOVE_SCISSORS)
    return RESULT_WIN;

  if (p1 == MOVE_PAPER && p2 == MOVE_ROCK)
    return RESULT_WIN;
  
  if (p1 == MOVE_SCISSORS && p2 == MOVE_PAPER)
    return RESULT_WIN;

  return RESULT_LOSE;
}

void rps_signup(int *waitingTid, int senderTid, HashMap *player_games, RPSResponse *resp)
{
  if (*waitingTid == 0)
  { // no other player waiting to join
    LOG_DEBUG("[RPS Signup]: Player Tid %d joined. Waiting for one more.", senderTid);
    *waitingTid = senderTid;
    return;
  }

  LOG_DEBUG("[RPS Signup]: Player Tid %d joined. Player Tid %d already here. Starting...", *waitingTid, senderTid);

  RPSGameState *game = alloc(RPS_GAME_STATE);
  game->gameComplete = false;
  game->p1 = *waitingTid;
  game->p2 = senderTid;
  game->p1Move = MOVE_NONE;
  game->p2Move = MOVE_NONE;

  char playerTid[4];
  ui2a(game->p1, 10, playerTid);
  hashmap_insert(player_games, playerTid, game);
  ui2a(game->p2, 10, playerTid);
  hashmap_insert(player_games, playerTid, game);

  *waitingTid = 0; // ready to accept another

  resp->res = RESULT_NONE;
  resp->type = RPS_SIGNUP;
  Reply(game->p1, (char *)resp, sizeof(RPSResponse));
  Reply(senderTid, (char *)resp, sizeof(RPSResponse));
}

void rps_error(int senderTid, RPSResponse *resp, RPSMessageType type)
{
  resp->type = type;
  resp->res = RESULT_INCOMPLETE;
  Reply(senderTid, (char *)resp, sizeof(RPSResponse));
}

void rps_play(int senderTid, RPSRequest req, RPSResponse *resp, HashMap *player_games)
{
  LOG_DEBUG("[RPS Play] Player %d played move: %s", senderTid, MOVE_STRING[req.move]);

  bool success;
  char mapKey[4];
  ui2a(senderTid, 10, mapKey);
  RPSGameState *game_state = hashmap_get(player_games, mapKey, &success);

  if (!success)
  {
    LOG_ERROR("[RPS ERROR] No game state for player %d", senderTid);
    rps_error(senderTid, resp, RPS_PLAY);
  }
  else if (game_state->gameComplete)
  {
    LOG_WARN("[RPS Play] Rejected player %d's move since game is already completed.", senderTid);
    rps_error(senderTid, resp, RPS_PLAY);
  }
  else
  {
    if (senderTid == game_state->p1)
      game_state->p1Move = req.move;
    if (senderTid == game_state->p2)
      game_state->p2Move = req.move;

    if (game_state->p1Move != MOVE_NONE && game_state->p2Move != MOVE_NONE)
    {
      LOG_DEBUG("[RPS Play] player %d and %d's game completed -> responding with results", game_state->p1, game_state->p2);

      resp->type = RPS_PLAY;
      resp->res = result(game_state->p1Move, game_state->p2Move);
      Reply(game_state->p1, (char *)resp, sizeof(RPSResponse));
      resp->res = result(game_state->p2Move, game_state->p1Move);
      Reply(game_state->p2, (char *)resp, sizeof(RPSResponse));

      game_state->p1Move = MOVE_NONE;
      game_state->p2Move = MOVE_NONE;
    }
  }
}

void rps_quit(int senderTid, RPSResponse *resp, HashMap *player_games)
{
  LOG_DEBUG("[RPS Quit] Player %d has quit", senderTid);
  bool success;
  char mapKey[4];
  ui2a(senderTid, 10, mapKey);
  RPSGameState *game_state = hashmap_get(player_games, mapKey, &success);

  if (!success)
  {
    LOG_ERROR("[RPS ERROR] No game state for player %d", senderTid);
    rps_error(senderTid, resp, RPS_QUIT);
  }
  else
  {
    if (game_state->gameComplete == true)
    {
      free(game_state, RPS_GAME_STATE);
    } else {
      resp->type = RPS_PLAY;
      resp->res = RESULT_INCOMPLETE;
      int otherTid = (senderTid == game_state->p1) ? game_state->p2 : game_state->p1;
      Reply(otherTid, (char *)resp, sizeof(RPSResponse));
    }
    game_state->gameComplete = true;
    hashmap_remove(player_games, mapKey);
    resp->type = RPS_QUIT;
    resp->res = RESULT_NONE;
    Reply(senderTid, (char *)resp, sizeof(RPSResponse));
  }
}

void RPSServer()
{
  alloc_init(RPS_GAME_STATE, sizeof(RPSGameState));
  HashMap *player_games = hashmap_new();
  RPSRequest request;
  RPSResponse response;

  int senderTid, requestLength;
  int waitingTid = 0;

  // register to name server
  RegisterAs(RPSAddress);

  while (1)
  {
    requestLength = Receive(&senderTid, (char *)&request, sizeof(RPSRequest));

    if (requestLength < 0)
    {
      LOG_WARN("[RPS ERROR - Receive]: return value = %d", requestLength);
      continue;
    }

    switch (request.type)
    {
    case RPS_SIGNUP:
    {
      rps_signup(&waitingTid, senderTid, player_games, &response);
      break;
    }
    case RPS_PLAY:
    {
      rps_play(senderTid, request, &response, player_games);
      break;
    }
    case RPS_QUIT:
    {
      rps_quit(senderTid, &response, player_games);
      break;
    }
    default:
    {
      LOG_WARN("[RPS ERROR - Invalid Request Type]: %d", request.type);
      break;
    }
    }
  }
}
