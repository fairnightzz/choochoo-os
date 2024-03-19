
#include "train_data.h"
#include "lib/stdlib.h"
#include "string.h"
#include "user/traintrack/track_data.h"

#define ZONE_MAX_SENSORS 8
#define ZONE_MAX_SWITCHES 6
#define NUM_ZONES 30
typedef struct
{
    char *sensors[ZONE_MAX_SENSORS];
    int switches[ZONE_MAX_SWITCHES];
} Zones;

static const Zones zones[] = {
    {{"B8", "A10", 0}, {0}},
    {{"B12", "A8", 0}, {0}},
    {{"B10", "A5", 0}, {0}},
    {{"A12", "A9", "A7", "A6", "C7", 0}, {1, 2, 3, 0}},
    {{"C8", "C6", "C15", "D11", "C3", "E11", 0}, {6, 18, 5, 7, 0}},
    {{"C5", "C10", "B15", 0}, {15, 0}},
    {{"C16", "D12", 0}, {0}},
    {{"C9", "B1", "B3", 0}, {16, 0}},
    {{"B4", "C2", 0}, {0}},
    {{"B16", "A3", 0}, {0}},
    {{"A4", "C11", "C13", "A2", "A14", "A15", 0}, {14, 11, 12, 4, 0}},
    {{"C12", "B5", "E16", 0}, {13, 0}},
    {{"E15", "E1", 0}, {0}},
    {{"E2", "D2", "C1", "B14", 0}, {153, 154, 155, 156, 0}},
    {{"C14", "E7", 0}, {0}},
    {{"B6", "D3", 0}, {0}},
    {{"D4", "E3", "E5", 0}, {10, 0}},
    {{"D1", "E4", 0}, {0}},
    {{"E8", "D7", 0}, {0}},
    {{"E6", "D6", 0}, {0}},
    {{"D8", "D5", "E10", "D9", 0}, {9, 8, 0}},
    {{"E12", "D10", 0}, {0}},
    {{"E13", "E9", 0}, {0}},
    {{"D15", "D13", "E14", 0}, {17, 0}},
    {{"B13", "D16", 0}, {0}},
    {{"B2", "D14", 0}, {0}},
    {{"C4", 0}, {0}},
    {{"A1", 0}, {0}},
    {{"A13", 0}, {0}},
    {{"A16", 0}, {0}},
};

static const int sensors[] = {27, 10, 9, 10, 2, 3, 3, 1, 3, 0, -1, 3, 28, 10, 10, 29, 7, 25, 7, 8, 11, 15, -1, 0, -1, 2, -1, 1, 24, 13, 5, 9, 13, 8, 4, 26, 5, 4, 3, 4, 7, 5, 10, 11, 10, 14, 4, 6, 17, 13, 15, 16, 20, 19, 18, 20, 20, 21, 4, 6, 23, 25, 23, 24, 12, 13, 16, 17, 16, 19, 14, 18, 22, 20, 4, 21, 22, 23, 12, 11};
static const int switches[] = {3, 3, 3, 10, 4, 4, 4, 20, 20, 16, 10, 10, 11, 10, 5, 7, 23, 4, 13, 13, 13, 13};

static track_node traintrack[TRACK_MAX];
static HashMap *NodeIndexMap;
void train_data_init()
{
    NodeIndexMap = hashmap_new();
    init_tracka(traintrack, NodeIndexMap);
}

track_node *
get_track()
{
    return traintrack;
}

HashMap *get_node_index_map()
{
    return NodeIndexMap;
}

int zone_getid_by_sensor_id(int sensorId)
{
    return sensors[sensorId];
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
    return switches[switch_id];

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
