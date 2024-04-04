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
#define FOOD_COUNT 30

char* FOODLIST[] = { "C13", "E7", "D7", "C11", "B5", "D3", "E5", "D5", "A4", "E15", "E3", "E1", "D1", "B15", "C1", "B13", "E9", "B3", "D15", "C9", "B1", "D13", "E13", "C5", "C15", "D11", "E11", "C7", "C3", "D10", 0};

void init_food(int *food_sensors) {
  track_node *track = get_track();
  for (int i = 0; i < 80; i++) {
    food_sensors[i] = 0;
    bool works = false;
    char *sensor_name = track[i].name;
    char** foodlist_node = FOODLIST;

    for (; *foodlist_node != 0; ++foodlist_node) {
      if (strcmp(*foodlist_node, sensor_name) == 0) works = true;
    }

    if (works) {
      food_sensors[i] = 1;
      render_food(i);
    }
  }
}

char* getRandomFoodDest(int *eaten, int *food_sensors, int *score) {
  track_node *track = get_track();
  if (*eaten == FOOD_COUNT) {
    render_command("[PacTrain INFO]: all food eaten. resetting food...");
    *eaten = 0;
    *score += 150;
    render_pacman_score(*score);
    init_food(food_sensors);
  }
  while (1) {
    int counter = rand_int() % (FOOD_COUNT - *eaten);
    int dest_n = -1;
    for (int i = 0; i < 80; i++) {
      if (food_sensors[i] == 1 && counter == 0) {
        dest_n = i;
        break;
      } else if (food_sensors[i] == 1) {
        counter -= 1;
      }
    }
    if (dest_n == -1) {
      LOG_ERROR("something went horribly wrong.");
    }
    return track[dest_n].name;
  }
  return 0; // unreachable
}

void SingleTrainPacTrainDestServer() {
  int server = WhoIs(PacTrainAddress);

  PacTrainRequest request;
  PacTrainResponse response;
  int from_tid;
  bool running = true;

  while (running) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(PacTrainRequest));
    if (req_len < 0) {
      LOG_WARN("[SingleTrainRandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type) {
      case NEW_FOOD_DEST: {
        response = (PacTrainResponse) {
          .type = NEW_FOOD_DEST,
          .train = request.train
        };

        Path new_path = (Path) {
          .allow_reversal = true,
          .dest = request.destination,
          .offset = 0,
          .speed = 14,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        render_command("[PacTrain INFO]: routing train %d to %s", new_path.train, new_path.dest);

        int tid = PlanPath(new_path);
        if (tid != 0) AwaitTid(tid);

        PacTrainRequest dest_reached = (PacTrainRequest) {
          .destination = request.destination,
          .train = request.train,
          .type = REACHED_FOOD_DEST
        };
        PacTrainResponse dest_reponse;
        Send(server, (const char *)&dest_reached, sizeof(PacTrainRequest), (char *)&dest_reponse, sizeof(PacTrainResponse));
        break;
      } case END_GAME: {
        running = false;
        response = (PacTrainResponse) {
          .type = END_GAME,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        break;
      } default: {
        LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
        break;
      }
    }
  }
  Exit();
}

void PacTrainServer() {
  PacTrainRequest request;
  PacTrainResponse response;
  int from_tid;
  RegisterAs(PacTrainAddress);


  int route_trains[PACTRAIN_COUNT] = { -1, -1, -1, -1 };
  int helper_tids[PACTRAIN_COUNT] = { -1, -1, -1, -1 };
  int food_sensors[80] = {0};
  track_node *track = get_track();

  bool exiting = false;
  int eaten = 0;
  int score = 0;

  while (1) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(PacTrainRequest));
    if (req_len < 0) {
      LOG_WARN("[RandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type)
    {
      case START_GAME: {
        response = (PacTrainResponse) {
          .type = START_GAME
        };
        route_trains[0] = request.pactrain;
        route_trains[1] = request.ghost1;
        route_trains[2] = request.ghost2;
        route_trains[3] = request.ghost3;
        exiting = false;
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        eaten = 0;
        score = 0;
        init_food(food_sensors);

        for (int i = 0; i < 1; i++) {
          char *new_dest = getRandomFoodDest(&eaten, food_sensors, &score);
          helper_tids[i] = Create(5, &SingleTrainPacTrainDestServer);
          PacTrainRequest new_dest_req = (PacTrainRequest) {
            .destination = new_dest,
            .train = route_trains[i],
            .type = NEW_FOOD_DEST
          };
          PacTrainResponse new_dest_resp;
          Send(helper_tids[i], (const char *)&new_dest_req, sizeof(PacTrainRequest), (char *)&new_dest_resp, sizeof(PacTrainResponse));
        }
        break;
      } case END_GAME: {
        response = (PacTrainResponse) {
          .type = END_GAME
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        exiting = true;
        break;
      } case REACHED_FOOD_DEST: {
        response = (PacTrainResponse) {
          .type = REACHED_FOOD_DEST,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        int index = -1;
        for (int i = 0; i < PACTRAIN_COUNT; i++) {
          if (route_trains[i] == request.train) {
            index = i;
            break;
          }
        }

        if (!exiting) {
          char *new_dest = getRandomFoodDest(&eaten, food_sensors, &score);
          render_command("[PacTrainServer INFO]: routing train %d to %s", route_trains[index], new_dest);

          PacTrainRequest new_dest_req = (PacTrainRequest) {
            .destination = new_dest,
            .train = route_trains[index],
            .type = NEW_FOOD_DEST
          };
          PacTrainResponse new_dest_resp;
          Send(helper_tids[index], (const char *)&new_dest_req, sizeof(PacTrainRequest), (char *)&new_dest_resp, sizeof(PacTrainResponse));
        } else {
          PacTrainRequest exit_req = (PacTrainRequest) {
            .destination = 0,
            .train = route_trains[0],
            .type = END_GAME
          };
          PacTrainResponse exit_resp;
          Send(helper_tids[index], (const char *)&exit_req, sizeof(PacTrainRequest), (char *)&exit_resp, sizeof(PacTrainResponse));

          AwaitTid(helper_tids[0]);
          helper_tids[index] = -1;
          if (helper_tids[0] == -1) { //&& helper_tids[1] == -1) {
            render_command("[RandDestServer INFO]: finished exiting random path routing");
          }
        }

        break;
      } case TRAIN_IDENTITY_QUERY: {
        int idx_train = -1;
        for (int i = 0; i < PACTRAIN_COUNT; i++) {
          if (!exiting && request.train != -1 && route_trains[i] == request.train) {
            idx_train = i;
          }
        }
        response = (PacTrainResponse) {
          .train = request.train,
          .train_identity = idx_train == -1 ? NONE_TRAIN : (idx_train == 0 ? PAC_TRAIN : GHOST_TRAIN)
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        break;
      } case ATE_FOOD: {
        response = (PacTrainResponse) {
          .type = ATE_FOOD
        };
        if (food_sensors[request.sensor_id] == 1) {
          // render_command("ate food at %s. now eaten %d", get_sensor_string(request.sensor_id), eaten + 1);
          food_sensors[request.sensor_id] = 0;
          eaten += 1;
          score += 5;
          render_pacman_score(score);
        }
        int rev_sens = track[request.sensor_id].reverse - track;
        if (food_sensors[rev_sens] == 1) {
          // render_command("ate food at %s. now eaten %d", get_sensor_string(rev_sens), eaten + 1);
          food_sensors[rev_sens] = 0;
          eaten += 1;
          score += 5;
          render_pacman_score(score);
        }   
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        break;
      } case FOOD_QUERY: {
        bool has_food = food_sensors[request.sensor_id] == 1;
        response = (PacTrainResponse) {
          .type = FOOD_QUERY,
          .has_food = has_food
        };
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        break;
      } default: {
        LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
        break;
      } 
    }
  }
}