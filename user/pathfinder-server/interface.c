#include "interface.h"
#include "user/nameserver.h"

int FindPath(int pathfinder_server, int train, int speed, int offset, char *destination)
{
    PathFinderResponse response;
    PathFinderRequest request = (PathFinderRequest) {
        .train = train,
        .speed = speed,
        .offset = offset,
        .destination = destination
    };

    int ret = Send(pathfinder_server, (const char*)&request, sizeof(PathFinderRequest), (char*)&response, sizeof(PathFinderResponse));

    return ret;
}
