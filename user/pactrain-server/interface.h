#ifndef __PACTRAIN_INTERFACE_H__
#define __PACTRAIN_INTERFACE_H__

#include <stdint.h>
#include "lib/stdlib.h"

#define PacTrainAddress "pactrain"

typedef enum 
{
  PAC_TRAIN = 1,
  GHOST_TRAIN_1 = 2,
  GHOST_TRAIN_2 = 3,
  GHOST_TRAIN_3 = 4,
  NONE_TRAIN = 5
} PacTrainType;

typedef enum
{
  START_GAME = 1,
  END_GAME,
  REACHED_FOOD_DEST,
  NEW_FOOD_DEST,
  FOOD_QUERY,
  TRAIN_IDENTITY_QUERY,
  ATE_FOOD,
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
  PacTrainType train_type;
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