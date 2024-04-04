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

char* BLACKLIST[] = { "A1", "A2", "A5","A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "B7", "B8", "B9", "B10", "B11", "B12", "B15", "B16", "C3", "C4", "C11", "C12", "E5", "E6", "E11", "E12", "E13", "E14", 0 };

char* getRandomDest() {
  track_node *track = get_track();
  while (1) {
    bool works = true;
    int dest_n = rand_int() % 80;
    char *dest = track[dest_n].name;

    char** blacklist_node = BLACKLIST;
    for (; *blacklist_node != 0; ++blacklist_node) {
        if (strcmp(*blacklist_node, dest) == 0) works = false;
    }
    if (works) {
      return track[dest_n].name;
    }
  }
  return 0; // unreachable
}

void SingleTrainRandDestServer() {
  int server = WhoIs(RandomDestAddress);

  RandDestRequest request;
  RandDestResponse response;
  int from_tid;
  bool running = true;

  while (running) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(RandDestRequest));
    if (req_len < 0) {
      LOG_WARN("[SingleTrainRandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type) {
      case NEW_DESTINATION: {
        response = (RandDestResponse) {
          .type = NEW_DESTINATION,
          .train = request.train
        };

        Path new_path = (Path) {
          .allow_reversal = true,
          .dest = request.destination,
          .offset = 0,
          .speed = 14,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(RandDestResponse));
        render_command("[RandDestServer INFO]: routing train %d to %s", new_path.train, new_path.dest);

        int tid = PlanPath(new_path);
        if (tid != 0) AwaitTid(tid);

        RandDestRequest dest_reached = (RandDestRequest) {
          .destination = request.destination,
          .train = request.train,
          .type = REACHED_DESTINATION
        };
        RandDestResponse dest_reponse;
        Send(server, (const char *)&dest_reached, sizeof(RandDestRequest), (char *)&dest_reponse, sizeof(RandDestResponse));
        break;
      } case END_ROUTING: {
        running = false;
        response = (RandDestResponse) {
          .type = END_ROUTING,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(RandDestResponse));
        break;
      } default: {
        LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
        break;
      }
    }
  }
  Exit();
}

void RandomDestinationServer() {
  PacTrainRequest request;
  PacTrainResponse response;
  int from_tid;

  RegisterAs(PacTrainAddress);


  int route_trains[PACTRAIN_COUNT] = { -1, -1, -1, -1 };
  int helper_tids[PACTRAIN_COUNT] = { -1, -1, -1, -1 };
  int food_sensors[80] = {0};

  bool exiting = false;

  bool running = false;

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
        running = true;
        exiting = false;
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));

        for (int i = 0; i < 1; i++) {
          char *new_dest = getRandomDest();
          helper_tids[i] = Create(5, &SingleTrainRandDestServer);
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
          char *new_dest = getRandomDest();
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
            .train = route_trains[index],
            .type = END_GAME
          };
          PacTrainResponse exit_resp;
          Send(helper_tids[index], (const char *)&exit_req, sizeof(PacTrainRequest), (char *)&exit_resp, sizeof(PacTrainResponse));

          AwaitTid(helper_tids[index]);
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
          .train_identity = idx_train == -1 ? NONE_TRAIN : (idx_train == 0 ? PAC_TRAIN : GHOST_TRAIN);
        }
        Reply(from_tid, (char *)&response, sizeof(PacTrainResponse));
        break;
      } default: {
        LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
        break;
      } 
    }
  }
}