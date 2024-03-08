#include "interface.h"
#include "user/nameserver.h"

int FindPath(int pathfinder_server, int train, int speed, int offset, char destination[3])
{
    PathFinderResponse response;
    PathFinderRequest request = (PathFinderRequest){
        .train = train,
        .speed = speed,
        .offset = offset,
        .destination = {0},
    };

    memcpy(request.destination, destination, 3);

    int ret = Send(pathfinder_server, (const char *)&request, sizeof(PathFinderRequest), (char *)&response, sizeof(PathFinderResponse));

    return ret;
}
