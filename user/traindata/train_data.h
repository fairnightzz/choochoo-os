#ifndef __TRAIN_DATA_H__
#define __TRAIN_DATA_H__

#include <stdint.h>
#include "user/traintrack/track_node.h"
#include "user/traintrack/track_data.h"
#include "lib/stdlib.h"

#define TRAIN_DATA_TRAIN_COUNT 6
#define TRAIN_DATA_SPEED_COUNT 12

#define TRAIN_SPEED_SNAIL 5
#define TRAIN_SPEED_LOW 8
#define TRAIN_SPEED_MED 11
#define TRAIN_SPEED_HIGH 14

#define TRAIN_DATA_SHORT_MOVE_DIST_COUNT 17
#define TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT 250

static const uint32_t TRAIN_DATA_SHORT_MOVE_SPEED[TRAIN_DATA_TRAIN_COUNT] = {4, 4, 3, 7, 9, 4};
static const uint32_t TRAIN_DATA_TRAINS[TRAIN_DATA_TRAIN_COUNT] = {2, 47, 54, 55, 58, 77};
static const uint32_t TRAIN_DATA_SPEEDS[TRAIN_DATA_SPEED_COUNT] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const uint32_t TRAIN_DATA_VEL[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {125, 170, 234, 289, 351, 407, 467, 526, 588, 644, 696, 698}, // n2
    {120, 180, 252, 311, 375, 418, 472, 516, 574, 628, 669, 676}, // 47 // speed 3 sus
    {126, 174, 231, 285, 344, 385, 435, 482, 535, 590, 599, 606}, // 54 track B
    {6, 26, 53, 105, 162, 202, 255, 315, 373, 428, 479, 517},     // 55
    {21, 58, 86, 121, 171, 191, 296, 357, 438, 510, 589, 626},    // 58
    {31, 70, 129, 169, 221, 278, 335, 403, 473, 544, 618, 658}    // 77 speed 3 and 4 sus
};

static const uint32_t TRAIN_DATA_STOP_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {158, 240, 290, 348, 455, 532, 589, 675, 810, 855, 976, 1040}, // 2 number is on back, for T2 speed 14 C10->B3 and C13->E7
    // For higher speeds 11 + , change C13 to E7
    {180, 250, 293, 319, 421, 490, 549, 615, 803, 790, 864, 882},  // 47 track B
    {149, 232, 304, 382, 450, 551, 617, 682, 835, 870, 909, 943},  // 54 track B // D1 offset 10
    {80, 100, 271, 109, 126, 125, 281, 237, 460, 587, 760, 1050},  // 55 track B
    {5, 20, 42, 93, 164, 359, 319, 490, 580, 726, 945, 1245},      // 58 tested 14-10
    {60, 120, 151, 235, 205, 452, 604, 427, 779, 1042, 1370, 1700} // 77 tested 14?
};

// we don't use this anymore
static const uint32_t TRAIN_DATA_STOP_TIME[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {2455, 0, 0, 2713, 0, 0, 2780, 0, 0, 2988}, // 2
    {2377, 0, 0, 2672, 0, 0, 2641, 0, 0, 2865}, // 47
    {3485, 0, 0, 3663, 0, 0, 3860, 0, 0, 4332}, // 54 // dummy data
    {3485, 0, 0, 3663, 0, 0, 3860, 0, 0, 4332}, // 55 // dummy data
    {3485, 0, 0, 3663, 0, 0, 3860, 0, 0, 4332}, // 58
    {3495, 0, 0, 3853, 0, 0, 4543, 0, 0, 5190}, // 77
};

static const uint32_t TRAIN_DATA_SHORT_MOVE_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SHORT_MOVE_DIST_COUNT] = {
    {0, 10, 17, 33, 66, 97, 132, 186, 235, 305, 376, 475, 601, 725, 890, 992, 1118},   // 2
    {0, 8, 24, 39, 73, 105, 152, 203, 269, 334, 435, 555, 730, 873, 1002, 1110, 1175}, // 47
    {0, 10, 25, 72, 118, 192, 245, 290, 330, 384, 446, 477, 540, 575, 639, 672, 714},  // 54 dummy
    {0, 10, 25, 72, 118, 192, 245, 290, 330, 384, 446, 477, 540, 575, 639, 672, 714},  // 55 dummy
    {0, 10, 25, 72, 118, 192, 245, 290, 330, 384, 446, 477, 540, 575, 639, 672, 714},  // 58
    {0, 17, 58, 97, 154, 209, 300, 351, 405, 458, 515, 570, 634, 690, 745, 805, 849},  // 77
};

void train_data_init(char ch);
track_node *get_track();
HashMap *get_node_map();
uint32_t train_data_vel(uint32_t train, uint32_t speed);
uint32_t train_data_stop_dist(uint32_t train, uint32_t speed);
int get_train_index(uint32_t train);
int get_speed_index(uint32_t speed);
int zone_getid_by_sensor_id(int sensorId);
int zone_getid_by_switch_id(int switchId);

uint32_t train_data_stop_time(uint32_t train, uint32_t speed);
uint32_t train_data_short_move_time(uint32_t train, uint32_t dist);

#endif // __TRAIN_DATA_H__
