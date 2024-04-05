#include "interface.h"
#include "stdlib.h"

int StartGame(int tid, int pactrain, int ghost1, int ghost2, int ghost3) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = START_GAME,
    .pactrain = pactrain,
    .ghost1 = ghost1,
    .ghost2 = ghost2,
    .ghost3 = ghost3
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing start returned negative value");
  }
  return returnValue;
}

bool SensorHasFood(int tid, int sensor_id) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = FOOD_QUERY,
    .sensor_id = sensor_id
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing start returned negative value");
  }

  return response.has_food;
}

PacTrainType WhoTrain(int tid, int train) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = TRAIN_IDENTITY_QUERY,
    .train = train
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing start returned negative value");
  }

  return response.train_identity;
}

int AteFood(int tid, int sensor_id) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = ATE_FOOD,
    .sensor_id = sensor_id
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing quit returned negative value");
  }
  return response.has_food;
}

int EndGame(int tid) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = END_GAME,
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, random routing quit returned negative value");
  }
  return returnValue;
}

int NotifyPacServerDeadlock(int tid, int train) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = PAC_TRAIN_DEADLOCK,
    .train = train
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, deadlock check returned negative value");
  }

  return returnValue;
}

int FetchNewFoodDest(int tid) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = FETCH_NEW_FOOD
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, deadlock check returned negative value");
  }

  return response.new_dest; 
}

int GetPacTrain(int tid) {
  PacTrainRequest request = (PacTrainRequest) {
    .type = GET_PAC_TRAIN
  };
  PacTrainResponse response;
  int returnValue = Send(tid, (const char *)&request, sizeof(PacTrainRequest), (char *)&response, sizeof(PacTrainResponse));

  if (returnValue < 0) {
    LOG_ERROR("tid %d, deadlock check returned negative value");
  }

  return response.train; 
}