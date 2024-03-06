#include "train_data.h"
#include "lib/stdlib.h"

uint32_t
get_train_index(uint32_t train)
{
    for (uint32_t i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++) {
        if (TRAIN_DATA_TRAINS[i] == train) {
            return i;
        }
    }
    LOG_ERROR("Invalid train for train data %u", train);
    return -1;
}

uint32_t
get_speed_index(uint32_t speed)
{
    for (uint32_t i = 0; i < TRAIN_DATA_SPEED_COUNT; i++) {
        if (TRAIN_DATA_SPEEDS[i] == speed) {
            return i;
        }
    }
    LOG_ERROR("Invalid speed for train data %u", speed);
    return -1;
}

uint32_t
train_data_vel(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_VEL[train_index][speed_index];
}

uint32_t
train_data_stop_dist(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_STOP_DIST[train_index][speed_index];
}
