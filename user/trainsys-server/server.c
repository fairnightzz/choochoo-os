#include "server.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"
#include "user/io-server/io_marklin.h"

void TrainSystemServer() {
  RegisterAs(TrainSystemAddress);

  int marklin_io = WhoIs(MarklinIOAddress);
  uint8_t train_states[TRAINS_COUNT] = {0};

  TrainSystemRequest request;
  TrainSystemResponse response;
  int from_tid;

  while (1) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(TrainSystemRequest));
    if (req_len < 0) {
      LOG_ERROR("[TrainSystemServer ERROR]: on receive: %d", req_len);
    }

    switch (request.type) {
      case SYSTEM_GET_TRAIN:
      {
        LOG_INFO("[TrainSystemServer INFO]: getting train state of train %d", request.train);
        response = (TrainSystemResponse) {
          .type = SYSTEM_GET_TRAIN,
          .train_state = train_states[request.train]
        };
        Reply(from_tid, (char*)&response, sizeof(TrainSystemResponse));
        break;
      }
      case SYSTEM_SET_SPEED:
      {
        LOG_INFO("[TrainSystemServer INFO]: setting train speed of train %d to %d", request.train, request.speed);
        
        int train = request.train;
        int speed = request.speed;
        train_states[train] = (train_states[train] & ~TRAIN_SPEED_MASK) | speed;

        io_marklin_set_train(marklin_io, train, train_states[train]);

        response = (TrainSystemResponse) {
          .type = SYSTEM_SET_SPEED,
          .train_state = train_states[train]
        };
        Reply(from_tid, (char*)&response, sizeof(TrainSystemResponse));

        break;
      }
      case SYSTEM_SET_LIGHTS:
      {
        LOG_INFO("[TrainSystemServer INFO]: setting train lights of train %d to %d", request.train, request.light_status);
        
        int train = request.train;
        bool light_status = request.light_status;
        train_states[train] = light_status ? train_states[train] | TRAIN_LIGHTS_MASK : train_states[train] & ~TRAIN_LIGHTS_MASK;

        io_marklin_set_train(marklin_io, train, train_states[train]);

        response = (TrainSystemResponse) {
          .type = SYSTEM_SET_SPEED,
          .train_state = train_states[train]
        };
        Reply(from_tid, (char*)&response, sizeof(TrainSystemResponse));
        
        break;
      }
      default:
      {
        LOG_ERROR("[TrainSystemServer ERROR]: invalid message type");
      }
    }
  }
}
