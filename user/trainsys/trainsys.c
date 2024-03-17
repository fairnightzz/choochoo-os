#include "user/trainsys/trainsys.h"
#include "user/ui/render.h"
#include "user/nameserver.h"
#include "lib/stdlib.h"
#include "user/clock-server/interface.h"
#include "user/trainsys-server/interface.h"
#include "user/switch-server/interface.h"
#include "user/pathfinder-server/interface.h"

static TrainSystemState SystemState;

void trainsys_execute_command(CommandResult cres)
{
  switch (cres.command_type)
  {
  case TRAIN_SPEED_COMMAND:
  {
    uint32_t train = cres.command_args.train_speed_args.train;
    uint32_t speed = cres.command_args.train_speed_args.speed;
    TrainSystemSetSpeed(SystemState.system_tid, train, speed);
    break;
  }
  case LIGHTS_COMMAND:
  {
    uint32_t train = cres.command_args.light_args.train;
    bool state = cres.command_args.light_args.state;
    TrainSystemSetLights(SystemState.system_tid, train, state);
    break;
  }
  case REVERSE_COMMAND:
  {
    uint32_t train = cres.command_args.reverse_args.train;
    int old_speed = TrainSystemGetTrainState(SystemState.system_tid, train) & TRAIN_SPEED_MASK;
    TrainSystemSetSpeed(SystemState.system_tid, train, SPEED_STOP);
    Delay(SystemState.clock_tid, REV_STOP_DELAY);
    TrainSystemSetSpeed(SystemState.system_tid, train, SPEED_REVERSE);
    Delay(SystemState.clock_tid, REV_DELAY);
    TrainSystemSetSpeed(SystemState.system_tid, train, old_speed);
    break;
  }
  case SWITCH_COMMAND:
  {
    uint32_t switch_id = cres.command_args.switch_args.switch_id;
    SwitchMode switch_mode = cres.command_args.switch_args.switch_mode;
    SwitchSet(SystemState.switch_tid, switch_id, switch_mode);
    break;
  }
  case QUIT_COMMAND:
  {
    reboot();
    break;
  }
  case RESET_TRACK_COMMAND:
  {
    trainsys_init_track(TRACK_A);
    break;
  }
  case PATH_COMMAND:
  {
    uint32_t train = cres.command_args.path_args.train;
    uint32_t speed = cres.command_args.path_args.speed;
    string dest_node = cres.command_args.path_args.dest_node;
    int32_t offset = cres.command_args.path_args.offset;

    FindPath(SystemState.pathfinder_tid, train, speed, offset, dest_node.data);
    break;
  }
  default:
    break;
  }
}

void trainsys_init()
{
  int system_tid = WhoIs(TrainSystemAddress);
  int clock_tid = WhoIs(ClockAddress);
  int switch_tid = WhoIs(SwitchAddress);
  int pathfinder_tid = WhoIs(PathFinderAddress);
  SystemState = (TrainSystemState){
      .system_tid = system_tid,
      .clock_tid = clock_tid,
      .switch_tid = switch_tid,
      .pathfinder_tid = pathfinder_tid,
  };
}

void trainsys_init_track(TrackSwitchPlans track_plan)
{
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    int switch_id = i + 1;
    if (switch_id > 18)
    {
      switch_id += 134;
    }

    SwitchSet(SystemState.switch_tid, switch_id, TRACK_PLANS[track_plan][i]);
  }

  /*
  if (track_plan == TRACK_A) {
    init_tracka(SystemState.track);
  } else {
    init_trackb(SystemState.track);
  }*/
}