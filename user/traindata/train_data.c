
#include "train_data.h"
#include "lib/stdlib.h"

int
get_train_index(uint32_t train)
{
    for (uint32_t i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++) {
        if (TRAIN_DATA_TRAINS[i] == train) {
            return i;
        }
    }
    return -1;
}

int
get_speed_index(uint32_t speed)
{
    for (uint32_t i = 0; i < TRAIN_DATA_SPEED_COUNT; i++) {
        if (TRAIN_DATA_SPEEDS[i] == speed) {
            return i;
        }
    }
    return -1;
}

uint32_t
train_data_vel(uint32_t train, uint32_t speed)
{
    int train_index = get_train_index(train);
    int speed_index = get_speed_index(speed);
    if (train_index == -1 || speed_index == -1) {
      LOG_ERROR("Invalid train data");
    }
    return TRAIN_DATA_VEL[train_index][speed_index];
}

uint32_t
train_data_stop_dist(uint32_t train, uint32_t speed)
{
    int train_index = get_train_index(train);
    int speed_index = get_speed_index(speed);
    if (train_index == -1 || speed_index == -1) {
      LOG_ERROR("Invalid train data");
    }
    return TRAIN_DATA_STOP_DIST[train_index][speed_index];
}
