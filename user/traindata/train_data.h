#ifndef __TRAINDATA_H__
#define __TRAIN_DATA_H__

#include <stdint.h>

#define TRAIN_DATA_TRAIN_COUNT 2
#define TRAIN_DATA_SPEED_COUNT 10

#define TRAIN_SPEED_SNAIL  5
#define TRAIN_SPEED_LOW   8
#define TRAIN_SPEED_MED    11
#define TRAIN_SPEED_HIGH   14


static const uint32_t TRAIN_DATA_TRAINS[TRAIN_DATA_TRAIN_COUNT] = {2, 47};
static const uint32_t TRAIN_DATA_SPEEDS[TRAIN_DATA_SPEED_COUNT] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const uint32_t TRAIN_DATA_VEL[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {234, 289, 351, 407, 467, 526, 588, 644, 696, 698},  // 2
    {252, 311, 375, 418, 472, 516, 574, 628, 669, 676},  // 47
};
static const uint32_t TRAIN_DATA_STOP_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {258, 348, 455, 512, 589, 675, 762, 855, 976, 959},  // 2
    {233, 319, 421, 490, 549, 615, 703, 790, 864, 882},  // 47
};

uint32_t train_data_vel(uint32_t train, uint32_t speed);
uint32_t train_data_stop_dist(uint32_t train, uint32_t speed);
int get_train_index(uint32_t train);
int get_speed_index(uint32_t speed);

#endif // __TRAIN_DATA_H__
