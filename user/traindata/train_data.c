
#include "train_data.h"
#include "string.h"
#include "user/traintrack/track_data.h"

track_node traintrack[TRACK_MAX];
int trackType;
HashMap *NodeIndexMap;
void train_data_init(char ch)
{
    NodeIndexMap = hashmap_new();

    // Init track A or B?

    if (ch == 'A')
    {
        init_tracka(traintrack, NodeIndexMap);
        trackType = 0; // A
    }
    else
    {
        init_trackb(traintrack, NodeIndexMap);
        trackType = 1; // B
    }
}

int getTrackType()
{
    return trackType;
}

track_node *
get_track()
{
    return traintrack;
}

HashMap *get_node_map()
{
    return NodeIndexMap;
}

int zone_getid_by_sensor_id(int sensorId)
{
    int sensorsA[] = {27, 10, 9, 10, 2, 3, 3, 1, 3, 0, -1, 3, 28, 10, 10, 29, 7, 25, 7, 8, 11, 15, -1, 0, -1, 2, -1, 1, 24, 13, 5, 9, 13, 8, 4, 26, 5, 4, 3, 4, 7, 5, 10, 11, 10, 14, 4, 6, 17, 13, 15, 16, 20, 19, 18, 20, 20, 21, 4, 6, 23, 25, 23, 24, 12, 13, 16, 17, 16, 19, 14, 18, 22, 20, 4, 21, 22, 23, 12, 11};
    int sensorsB[] = {27, 10, 9, 10, 2, 3, 3, 1, 3, 0, 29, 3, 28, 10, 10, 29, 7, 25, 7, 8, 11, 15, -1, 0, -1, 2, -1, 1, 24, 13, 5, 9, 13, 8, 4, 26, 5, 4, 3, 4, 7, 5, 10, 11, 10, 14, 4, 6, 17, 13, 15, 16, 20, 19, 18, 20, 20, 21, 4, 6, 23, 25, 23, 24, 12, 13, 16, 17, 16, 19, 14, 18, 22, 20, 4, 21, 22, 23, 12, 11};
    if (trackType == 0)
    {
        return sensorsA[sensorId];
    }
    else
    {
        return sensorsB[sensorId];
    }
    // string sensoridString = to_sensor_string(sensorId);
    // for (int i = 0; i < NUM_ZONES; i++)
    // {
    //     for (int j = 0;; ++j)
    //     {
    //         char *sensor_str = zones[i].sensors[j];
    //         if (sensor_str == 0)
    //             break;

    //         if (strcmp(get_data(&sensoridString), sensor_str) == 0)
    //         {
    //             return i;
    //         }
    //     }
    // }
    // return -1;
}
int zone_getid_by_switch_id(int switch_id)
{
    int switchesA[] = {3, 3, 3, 10, 4, 4, 4, 20, 20, 16, 10, 10, 11, 10, 5, 7, 23, 4, 13, 13, 13, 13};
    int switchesB[] = {3, 3, 3, 10, 4, 4, 4, 20, 20, 16, 10, 10, 11, 10, 5, 7, 23, 4, 13, 13, 13, 13};
    if (1 <= switch_id && switch_id <= 18)
    {
        switch_id = switch_id - 1;
    }
    else if (153 <= switch_id && switch_id <= 156)
    {
        switch_id = switch_id - 135;
    }
    else
    {
        return -1;
    }

    if (trackType == 0)
    {
        return switchesA[switch_id];
    }
    else
    {
        return switchesB[switch_id];
    }

    // for (int i = 0; i < NUM_ZONES; i++)
    // {
    //     for (int j = 0;; ++j)
    //     {
    //         int switch_id = zones[i].switches[j];
    //         if (switch_id == 0)
    //             break;
    //         if (switchId == switch_id)
    //         {
    //             return i;
    //         }
    //     }
    // }
    // return -1;
}

int get_train_index(uint32_t train)
{
    for (uint32_t i = 0; i < TRAIN_DATA_TRAIN_COUNT; i++)
    {
        if (TRAIN_DATA_TRAINS[i] == train)
        {
            return i;
        }
    }
    return -1;
}

int get_speed_index(uint32_t speed)
{
    for (uint32_t i = 0; i < TRAIN_DATA_SPEED_COUNT; i++)
    {
        if (TRAIN_DATA_SPEEDS[i] == speed)
        {
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
    if (train_index == -1 || speed_index == -1)
    {
        LOG_ERROR("Invalid train data");
    }
    return TRAIN_DATA_VEL[train_index][speed_index];
}

uint32_t
train_data_stop_dist(uint32_t train, uint32_t speed)
{
    int train_index = get_train_index(train);
    int speed_index = get_speed_index(speed);
    if (train_index == -1 || speed_index == -1)
    {
        LOG_ERROR("Invalid train data");
    }
    return TRAIN_DATA_STOP_DIST[train_index][speed_index];
}

uint32_t
train_data_stop_time(uint32_t train, uint32_t speed)
{
    uint32_t train_index = get_train_index(train);
    uint32_t speed_index = get_speed_index(speed);
    return TRAIN_DATA_STOP_TIME[train_index][speed_index];
}

uint32_t
train_data_short_move_time(uint32_t train, uint32_t dist)
{
    uint32_t train_index = get_train_index(train);
    for (uint32_t i = 0; i < TRAIN_DATA_SHORT_MOVE_DIST_COUNT; i++)
    {
        // For exact matches, return the exact time
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] == dist)
        {
            return i * TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
        }
        // For non matches, linearly interpolate the time from the two surrounding measurements
        if (TRAIN_DATA_SHORT_MOVE_DIST[train_index][i] > dist)
        {
            uint32_t prev_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i - 1];
            uint32_t next_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][i];
            uint32_t prev_time = (i - 1) * TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            uint32_t next_time = i * TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT;
            return prev_time + (next_time - prev_time) * (dist - prev_dist) / (next_dist - prev_dist);
        }
    }

    // For distances above the highest measured distance, linearly extrapolate based on distance between last two recorded distances
    uint32_t last_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT - 1];
    uint32_t penultimate_dist = TRAIN_DATA_SHORT_MOVE_DIST[train_index][TRAIN_DATA_SHORT_MOVE_DIST_COUNT - 2];
    uint32_t last_time = TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT * (TRAIN_DATA_SHORT_MOVE_DIST_COUNT - 1);

    return last_time + TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT * (dist - last_dist) / (last_dist - penultimate_dist);
}
