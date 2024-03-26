#include "server.h"
#include "user/nameserver.h"
#include <stdbool.h>
#include "lib/stdlib.h"
#include "interface.h"
#include "user/ui/render.h"
#include "user/pathfinder-server/interface.h"
#include "user/traindata/train_data.h"
#include <string.h>

#define ROUTE_TRAIN_COUNT 2

char* BLACKLIST[] = { "A1", "A2", "A5","A6", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "B7", "B8", "B9", "B10", "B11", "B12", "C3", "C4", "C11", 0 };

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
  RandDestRequest request;
  RandDestResponse response;
  int from_tid;

  RegisterAs(RandomDestAddress);
  int route_trains[ROUTE_TRAIN_COUNT] = { 2, 55 };
  int helper_tids[ROUTE_TRAIN_COUNT] = { -1, -1 };
  bool exiting = false;

  while (1) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(RandDestRequest));
    if (req_len < 0) {
      LOG_WARN("[RandDestServer]: error on receiving request");
      continue;
    }

    switch (request.type)
    {
      case START_ROUTING: {
        response = (RandDestResponse) {
          .type = START_ROUTING
        };
        Reply(from_tid, (char *)&response, sizeof(RandDestResponse));
        exiting = false;

        for (int i = 0; i < ROUTE_TRAIN_COUNT; i++) {
          char *new_dest = getRandomDest();
          helper_tids[i] = Create(5, &SingleTrainRandDestServer);
          RandDestRequest new_dest_req = (RandDestRequest) {
            .destination = new_dest,
            .train = route_trains[i],
            .type = NEW_DESTINATION
          };
          RandDestResponse new_dest_resp;
          Send(helper_tids[i], (const char *)&new_dest_req, sizeof(PathFinderRequest), (char *)&new_dest_resp, sizeof(PathFinderResponse));
        }
        break;
      } case END_ROUTING: {
        response = (RandDestResponse) {
          .type = END_ROUTING
        };
        Reply(from_tid, (char *)&response, sizeof(RandDestResponse));
        exiting = true;
        break;
      } case REACHED_DESTINATION: {
        response = (RandDestResponse) {
          .type = REACHED_DESTINATION,
          .train = request.train
        };
        Reply(from_tid, (char *)&response, sizeof(RandDestResponse));
        int index = request.train == route_trains[0] ? 0 : 1;

        if (!exiting) {
          char *new_dest = getRandomDest();
          render_command("[RandDestServer INFO]: routing train %d to %s", route_trains[index], new_dest);

          RandDestRequest new_dest_req = (RandDestRequest) {
            .destination = new_dest,
            .train = route_trains[index],
            .type = NEW_DESTINATION
          };
          RandDestResponse new_dest_resp;
          Send(helper_tids[index], (const char *)&new_dest_req, sizeof(PathFinderRequest), (char *)&new_dest_resp, sizeof(PathFinderResponse));
        } else {
          RandDestRequest exit_req = (RandDestRequest) {
            .destination = 0,
            .train = route_trains[index],
            .type = END_ROUTING
          };
          RandDestResponse exit_resp;
          Send(helper_tids[index], (const char *)&exit_req, sizeof(PathFinderRequest), (char *)&exit_resp, sizeof(PathFinderResponse));

          AwaitTid(helper_tids[index]);
          helper_tids[index] = -1;
          if (helper_tids[0] == -1) { //&& helper_tids[1] == -1) {
            render_command("[RandDestServer INFO]: finished exiting random path routing");
          }
        }

        break;
      } default: {
        LOG_ERROR("[RandDestServer]: Unhandled Request Type - %d", request.type);
        break;
      } 
    }
  }
}