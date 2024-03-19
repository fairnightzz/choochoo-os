#include "interface.h"
#include "user/nameserver.h"
#include "user/traintrack/track_data.h"
#include "user/trainsys-server/interface.h"
#include "server.h"

int PlanPath(Path path)
{
    track_node* track = get_track();
    int trainsys_server = WhoIs(TrainSystemAddress);

    // TODO
    int start_sensor = TrainSystemGetTrainSensor(trainsys_server, path.train);
    if (start_sensor == -1) {
        LOG_WARN("Train %d has unknown current position, aborting PlanPath", path.train);
        return 0;
    }
    track_node* dest = track + start_sensor;
    if (dest == NULL) {
        // TODO send back error?
        LOG_WARN("invalid destination");
        return 0;
    }
    int dest_sensor = dest - track;
   //  ULOG_INFO_M(LOG_MASK_PATH, "routing train %d from %d to %d", path.train, start_sensor, dest_sensor);

    PathFinderResponse response;
    PathFinderRequest send_buf = (PathFinderRequest) {
        .source = start_sensor,
        .destination = dest_sensor,
        .train = path.train,
        .speed = path.speed,
        .offset = path.offset,
        .allow_reversal = path.allow_reversal 
    };

    int pather_task = Create(2, &PathFinderTask);
    Send(pather_task, (const char*)&send_buf, sizeof(PathFinderRequest), (char*)&response, sizeof(PathFinderResponse));

    return pather_task;
}
