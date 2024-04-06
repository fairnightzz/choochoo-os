#include "server.h"
#include "user/nameserver.h"
#include <stdbool.h>
#include "lib/stdlib.h"
#include "interface.h"
#include "user/ui/render.h"
#include "user/pathfinder-server/interface.h"
#include "user/traindata/train_data.h"
#include <string.h>

#define PACTRAIN_COUNT 4
#define FOOD_COUNT 26
#define GHOST2_COUNT 4

char *FOODLIST[] = {"C13", "E7", "D7", "B5", "D3", "D5", "E15", "E3", "E1", "D1", "B15", "C1", "B13", "E9", "B3", "D15", "C9", "B1", "D13", "C5", "C15", "D11", "E11", "C7", "C3", "D10", 0};
char *GHOST2LIST[] = {"D9", "B16", "B6", "D7", 0};
char *RESET_POSITIONS[] = {"A9" , "A7", "A6", "A1", 0};

void init_food(int *food_sensors)
{
  track_node *track = get_track();
  for (int i = 0; i < 80; i++)
  {
    food_sensors[i] = 0;
    bool works = false;
    char *sensor_name = track[i].name;
    char **foodlist_node = FOODLIST;

    for (; *foodlist_node != 0; ++foodlist_node)
    {
      if (strcmp(*foodlist_node, sensor_name) == 0)
        works = true;
    }

    if (works)
    {
      food_sensors[i] = 1;
      render_food(i);
    }
  }
}

char *getRandomFoodDest(int *eaten, int *food_sensors, int *score, PacTrainType train_type, int *ghost2idx)
{
  track_node *track = get_track();
  if (train_type == PAC_TRAIN)
  {
    if (*eaten == FOOD_COUNT)
    {
      render_pacman_command("[PTS INFO]: all food eaten. resetting food...");
      *eaten = 0;
      *score += 150;
      render_pacman_score(*score);
      init_food(food_sensors);
    }
    while (1)
    {
      int counter = rand_int() % (FOOD_COUNT - *eaten);
      int dest_n = -1;
      for (int i = 0; i < 80; i++)
      {
        if (food_sensors[i] == 1 && counter == 0)
        {
          dest_n = i;
          break;
        }
        else if (food_sensors[i] == 1)
        {
          counter -= 1;
        }
      }
      if (dest_n == -1)
      {
        LOG_ERROR("something went horribly wrong.");
      }
      return track[dest_n].name;
    }
  }
  else if (train_type == GHOST_TRAIN_1)
  {
    int dest_n = rand_int() % FOOD_COUNT;
    return FOODLIST[dest_n];
  }
  else if (train_type == GHOST_TRAIN_2)
  {
    int dest_n = *ghost2idx;
    *ghost2idx = (*ghost2idx + 1) % GHOST2_COUNT;
    return GHOST2LIST[dest_n];
  }
  else if (train_type == GHOST_TRAIN_3)
  {
    LOG_ERROR("not handled - invalid train type in getRandFoodDest.");
  }
  else
  {
    LOG_ERROR("something went horribly wrong - invalid train type in getRandFoodDest.");
  }
  return 0; // unreachable
}

void SingleTrainPacTrainDestServer()
{
  int server = WhoIs(PacTrainAddress);

  PacTrainRequest request;
  PacTrainResponse response;
  int from_tid;
  bool running = true;

  while (running)
  {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(PacTrainRequest));
    if (req_len < 0)
    {
      LOG_WARN("[SingleTrainRandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type)
    {
    case NEW_FOOD_DEST:
    {
      response = (PacTrainResponse){
          .type = NEW_FOOD_DEST,
          .train = request.train,
          .train_identity = request.train_type};
      PacTrainType train_type = request.train_type;
      Path new_path = (Path){
          .allow_reversal = true,
          .dest = request.destination,
          .offset = 0,
          .speed = 14,
          .train = request.train};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));

      int tid = PlanPath(new_path);
      if (tid != 0)
        AwaitTid(tid);

      PacTrainRequest dest_reached = (PacTrainRequest){
          .destination = request.destination,
          .train = request.train,
          .type = REACHED_FOOD_DEST,
          .train_type = train_type};
      PacTrainResponse dest_reponse;
      Send(server, (const char *)&dest_reached, sizeof(PacTrainRequest), (char *)&dest_reponse, sizeof(PacTrainResponse));
      break;
    }
    case END_GAME:
    {
      running = false;
      response = (PacTrainResponse){
          .type = END_GAME,
          .train = request.train};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    default:
    {
      LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
      break;
    }
    }
  }
  Exit();
}

void PacTrainServerHelper(int *eaten, int *food_sensors, int *score, int *helper_tids, int *route_trains, PacTrainType train_type, int train_type_idx, int *ghost2idx)
{
  char *new_dest = getRandomFoodDest(eaten, food_sensors, score, train_type, ghost2idx);
  helper_tids[train_type_idx] = Create(5, &SingleTrainPacTrainDestServer); // 0 holds pacman tid
  PacTrainRequest new_dest_req = (PacTrainRequest){
      .destination = new_dest,
      .train = route_trains[train_type_idx],
      .type = NEW_FOOD_DEST,
      .train_type = PAC_TRAIN};
  PacTrainResponse new_dest_resp;
  Send(helper_tids[train_type_idx], (const char *)&new_dest_req, sizeof(PacTrainRequest), (char *)&new_dest_resp, sizeof(PacTrainResponse));
}

void PacTrainServer()
{
  PacTrainRequest request;
  PacTrainResponse response;
  int from_tid;
  RegisterAs(PacTrainAddress);

  PacTrainType GHOST_TRAIN_IDS[3] = {GHOST_TRAIN_1, GHOST_TRAIN_2, GHOST_TRAIN_3};
  int route_trains[PACTRAIN_COUNT] = {-1, -1, -1, -1};
  int helper_tids[PACTRAIN_COUNT] = {-1, -1, -1, -1};
  int food_sensors[80] = {0};
  track_node *track = get_track();

  bool exiting = false;
  int eaten = 0;
  int score = 0;
  int ghost2idx = 0;

  while (1)
  {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(PacTrainRequest));
    if (req_len < 0)
    {
      LOG_WARN("[RandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type)
    {
    case START_GAME:
    {
      response = (PacTrainResponse){
          .type = START_GAME};
      route_trains[0] = request.pactrain;
      route_trains[1] = request.ghost1;
      route_trains[2] = request.ghost2;
      route_trains[3] = request.ghost3;
      exiting = false;
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      eaten = 0;
      score = 0;
      ghost2idx = 0;
      init_food(food_sensors);
      render_pacman_score(score);

      // Path Pacman
      PacTrainServerHelper(&eaten, food_sensors, &score, helper_tids, route_trains, PAC_TRAIN, 0, &ghost2idx);

      break;
    }
    case END_GAME:
    {
      response = (PacTrainResponse){
          .type = END_GAME};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      exiting = true;
      break;
    }
    case REACHED_FOOD_DEST:
    {
      response = (PacTrainResponse){
          .type = REACHED_FOOD_DEST,
          .train = request.train,
          .train_identity = request.train_type};
      PacTrainType train_type = request.train_type;
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      int index = -1;
      for (int i = 0; i < PACTRAIN_COUNT; i++)
      {
        if (route_trains[i] == request.train)
        {
          index = i;
          break;
        }
      }

      if (!exiting)
      {
        char *new_dest = getRandomFoodDest(&eaten, food_sensors, &score, train_type, &ghost2idx);

        if (helper_tids[1] == -1 && score >= 20)
        { // spawn ghost 1
          render_pacman_command("[PTS INFO]: spawning ghost #1 train #%d", route_trains[1]);
          PacTrainServerHelper(&eaten, food_sensors, &score, helper_tids, route_trains, GHOST_TRAIN_1, 1, &ghost2idx);
        }
        if (helper_tids[2] == -1 && score >= 40)
        {
          render_pacman_command("[PTS INFO]: spawning ghost #2 train #%d", route_trains[2]);
          PacTrainServerHelper(&eaten, food_sensors, &score, helper_tids, route_trains, GHOST_TRAIN_2, 2, &ghost2idx);
        }
        
        if (index != 0)
        {
          render_pacman_command("[PTS INFO]: routing GHOST train %d to %s", route_trains[index], new_dest);
        }

        PacTrainRequest new_dest_req = (PacTrainRequest){
            .destination = new_dest,
            .train = route_trains[index],
            .type = NEW_FOOD_DEST,
            .train_type = train_type};
        PacTrainResponse new_dest_resp;
        Send(helper_tids[index], (const char *)&new_dest_req, sizeof(PacTrainRequest), (char *)&new_dest_resp, sizeof(PacTrainResponse));
      }
      else
      {
        PacTrainRequest exit_req = (PacTrainRequest){
            .destination = 0,
            .train = route_trains[0],
            .type = END_GAME};
        PacTrainResponse exit_resp;
        Send(helper_tids[index], (const char *)&exit_req, sizeof(PacTrainRequest), (char *)&exit_resp, sizeof(PacTrainResponse));

        AwaitTid(helper_tids[index]);
        helper_tids[index] = -1;
        if (helper_tids[0] == -1 && helper_tids[1] == -1 && helper_tids[2] == -1 && helper_tids[3] == -1)
        { //&& helper_tids[1] == -1) {
          render_pacman_command("[PTS INFO]: game completed");
        }
      }

      break;
    }
    case TRAIN_IDENTITY_QUERY:
    {
      int idx_train = -1;
      for (int i = 0; i < PACTRAIN_COUNT; i++)
      {
        if (request.train != -1 && route_trains[i] == request.train)
        {
          idx_train = i;
        }
      }
      response = (PacTrainResponse){
          .train = request.train,
          .train_identity = idx_train == -1 ? NONE_TRAIN : (idx_train == 0 ? PAC_TRAIN : GHOST_TRAIN_IDS[idx_train - 1])};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case PAC_TRAIN_DEADLOCK:
    {
      response = (PacTrainResponse){
          .type = PAC_TRAIN_DEADLOCK};
      for (int i = 0; i < PACTRAIN_COUNT; i++) {
        if (helper_tids[i] != -1) {
          Kill(helper_tids[i]);
        }
        if (route_trains[i] != -1) {
          int train = route_trains[i];
          route_trains[i] = -1;
          Path new_path = (Path){
            .allow_reversal = true,
            .dest = RESET_POSITIONS[i],
            .offset = 0,
            .speed = 14,
            .train = train};
          PlanPath(new_path);
        }
        helper_tids[i] = -1;
      }
      render_pacman_command("[PTS INFO]: GAME OVER!!");
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case FETCH_ALL_FOOD:
    {
      response = (PacTrainResponse){
          .type = FETCH_ALL_FOOD,
          .all_food = food_sensors};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case FETCH_NEW_FOOD:
    {
      int counter = rand_int() % (FOOD_COUNT - eaten);
      int dest_n = -1;
      for (int i = 0; i < 80; i++)
      {
        if (food_sensors[i] == 1 && counter == 0)
        {
          dest_n = i;
          break;
        }
        else if (food_sensors[i] == 1)
        {
          counter -= 1;
        }
      }
      if (dest_n == -1)
        render_pacman_command("[PTS ERR]: fetching new food -1 error");

      response = (PacTrainResponse){
          .type = FETCH_NEW_FOOD,
          .new_dest = dest_n};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case GET_PAC_TRAIN:
    {
      response = (PacTrainResponse){
          .type = GET_PAC_TRAIN,
          .train = route_trains[0]};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case ATE_FOOD:
    {
      response = (PacTrainResponse){
          .type = ATE_FOOD};
      if (food_sensors[request.sensor_id] == 1)
      {
        render_pacman_command("[PTS INFO]: ate food at %s. now eaten %d", get_sensor_string(request.sensor_id), eaten + 1);
        food_sensors[request.sensor_id] = 0;
        eaten += 1;
        score += 5;
        render_pacman_score(score);
      }
      int rev_sens = track[request.sensor_id].reverse - track;
      if (food_sensors[rev_sens] == 1)
      {
        render_pacman_command("[PTS INFO]: ate food at %s. now eaten %d", get_sensor_string(rev_sens), eaten + 1);
        food_sensors[rev_sens] = 0;
        eaten += 1;
        score += 5;
        render_pacman_score(score);
      }
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    case FOOD_QUERY:
    {
      int rev_sens = track[request.sensor_id].reverse - track;
      bool has_food = food_sensors[request.sensor_id] == 1 || food_sensors[rev_sens] == 1;
      response = (PacTrainResponse){
          .type = FOOD_QUERY,
          .has_food = has_food};
      Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
      break;
    }
    default:
    {
      render_pacman_command("[PTS ERROR]: Unhandled Request Type - %d", request.type);
      break;
    }
    }
  }
}