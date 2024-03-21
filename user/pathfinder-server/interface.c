#include "interface.h"
#include "user/nameserver.h"
#include "user/traintrack/track_data.h"
#include "user/trainsys-server/interface.h"
#include "user/switch-server/interface.h"
#include "server.h"
#include "user/traindata/train_data.h"
#include "user/ui/render.h"

int PlanPath(Path path)
{
    HashMap *nodeMap = get_node_map();
    int trainsys_server = WhoIs(TrainSystemAddress);
    // TODO

    bool success;
    int dest = (int)(intptr_t)hashmap_get(nodeMap, path.dest, &success);
    ;
    if (!success)
    {
        render_command("[PlanPath ERROR]: Invalid Destination.");
        return 0;
    }
    //  ULOG_INFO_M(LOG_MASK_PATH, "routing train %d from %d to %d", path.train, start_sensor, dest_sensor);

    PathFinderResponse response;
    PathFinderRequest send_buf = (PathFinderRequest){
        .destination = dest,
        .train = path.train,
        .speed = path.speed,
        .offset = path.offset,
        .allow_reversal = path.allow_reversal};

    int pather_task = Create(2, &PathFinderTask);
    Send(pather_task, (const char *)&send_buf, sizeof(PathFinderRequest), (char *)&response, sizeof(PathFinderResponse));

    return pather_task;
}
