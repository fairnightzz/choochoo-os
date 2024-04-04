#ifndef __PACTRAIN_INTERFACE_H__
#define __PACTRAIN_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define PacTrainAddress "pactrain"

typedef enum 
{
  PAC_TRAIN,
  GHOST_TRAIN_1,
  GHOST_TRAIN_2,
  GHOST_TRAIN_3,
  NONE_TRAIN
} PacTrainType;

typedef enum
{
  START_GAME = 1,
  END_GAME,
  REACHED_FOOD_DEST,
  NEW_FOOD_DEST,
  FOOD_QUERY,
  TRAIN_IDENTITY_QUERY,
  ATE_FOOD
} PacTrainMessageType;

typedef struct
{
  PacTrainMessageType type;
  char *destination;
  int pactrain;
  int ghost1;
  int ghost2;
  int ghost3;
  int sensor_id;
  int train;
} PacTrainRequest;

typedef struct
{
  PacTrainMessageType type;
  int train;
  bool has_food;
  PacTrainType train_identity;
} PacTrainResponse;


int StartGame(int tid, int pactrain, int ghost1, int ghost2, int ghost3);
int EndGame(int tid);
bool SensorHasFood(int tid, int sensor_id);
PacTrainType WhoTrain(int tid, int train);
int AteFood(int tid, int sensor_id);


#endif // __PACTRAIN_INTERFACE_H__