#include "user/trainsys/trainsys.h"
#include "user/ui/render.h"
#include "user/io-server/interface.h"
#include "user/nameserver.h"

static TrainSystemState SystemState;

/* HELPER FUNCTIONS */
void set_train_speed(uint32_t train, uint32_t speed)
{
  SystemState.train_state[train] = (SystemState.train_state[train] & ~TRAIN_SPEED_MASK) | speed;
  push(&(SystemState.serial_out), SystemState.train_state[train]);
  push(&(SystemState.serial_out), train);
}

void set_train_lights(uint32_t train, bool state)
{
  if (state)
  {
    SystemState.train_state[train] |= TRAIN_LIGHTS_MASK;
  }
  else
  {
    SystemState.train_state[train] &= ~TRAIN_LIGHTS_MASK;
  }
  push(&(SystemState.serial_out), SystemState.train_state[train]);
  push(&(SystemState.serial_out), train);
}

void set_train_reverse(uint32_t train, uint64_t time)
{
  if (SystemState.stop_times[train] == 0 && SystemState.rev_times[train] == 0)
  {
    SystemState.trains_reversing += 1;
    push(&(SystemState.serial_out), SystemState.train_state[train] & ~TRAIN_SPEED_MASK); // 0 train speed
    push(&(SystemState.serial_out), train);
    SystemState.stop_times[train] = time;
  }
}

void set_track_switch(uint32_t switch_id, SwitchMode switch_mode)
{
  SystemState.switch_states[switch_id] = switch_mode;
  push(&(SystemState.serial_out), switch_mode);
  push(&(SystemState.serial_out), switch_id);
  push(&(SystemState.serial_out), 0x20); // reset solenoid

  // draw switch on terminal
  render_switch(switch_id, switch_mode);
}

void trainsys_execute_command(CommandResult cres, int curr_tick)
{
  switch (cres.command_type)
  {
  case TRAIN_SPEED_COMMAND:
  {
    uint32_t train = cres.command_args.train_speed_args.train;
    uint32_t speed = cres.command_args.train_speed_args.speed;
    set_train_speed(train, speed);
    break;
  }
  case LIGHTS_COMMAND:
  {
    uint32_t train = cres.command_args.light_args.train;
    bool state = cres.command_args.light_args.state;
    set_train_lights(train, state);
    break;
  }
  case REVERSE_COMMAND:
  {
    uint32_t train = cres.command_args.reverse_args.train;
    set_train_reverse(train, curr_tick);
    break;
  }
  case SWITCH_COMMAND:
  {
    uint32_t switch_id = cres.command_args.switch_args.switch_id;
    SwitchMode switch_mode = cres.command_args.switch_args.switch_mode;
    set_track_switch(switch_id, switch_mode);
    break;
  }
  default:
    break;
  }
  trainsys_try_serial_out(curr_tick);
}

/* Driver Functions */
void trainsys_init()
{
  int marklin_tid = WhoIs(MarklinIOAddress);

  SystemState = (TrainSystemState){
      .last_serial_write = 0,
      .train_state = {0},
      .serial_out = new_byte_queue(),
      .stop_times = {0},
      .rev_times = {0},
      .last_sensor_read = 0,
      .switch_states = {0},
      .read_sensor_bytes = 0,
      .last_sensor_byte_read = 0,
      .trains_reversing = 0,
      .marklin_tid = marklin_tid,
  };
}

void trainsys_check_rev_trains(int curr_tick)
{
  if (SystemState.trains_reversing > 0)
  {
    for (int i = 0; i < TRAINS_COUNT; i++)
    {
      if (SystemState.stop_times[i] > 0 && curr_tick - SystemState.stop_times[i] > REV_STOP_DELAY)
      {
        SystemState.stop_times[i] = 0;
        push(&(SystemState.serial_out), (SystemState.train_state[i] & ~TRAIN_SPEED_MASK) | 15); // reverse
        push(&(SystemState.serial_out), i);
        SystemState.rev_times[i] = curr_tick;
      }

      if (SystemState.rev_times[i] > 0 && curr_tick - SystemState.rev_times[i] > REV_DELAY)
      {
        SystemState.rev_times[i] = 0;
        SystemState.trains_reversing -= 1;
        push(&(SystemState.serial_out), SystemState.train_state[i]);
        push(&(SystemState.serial_out), i);
      }
    }
    trainsys_try_serial_out(curr_tick);
  }
}

void trainsys_read_all_sensors(int curr_tick)
{
  if (curr_tick - SystemState.last_sensor_read > SENSOR_READ && SystemState.read_sensor_bytes == 0 && length(&(SystemState.serial_out)) == 0)
  {
    SystemState.last_sensor_read = curr_tick;
    push(&(SystemState.serial_out), 0x80 + 0x05);
    SystemState.read_sensor_bytes = 10;
    trainsys_try_serial_out(curr_tick);
  }
  if (SystemState.read_sensor_bytes > 0)
  {

    unsigned char read_sensor_byte = Getc(SystemState.marklin_tid);

    unsigned int bank_number = (10 - SystemState.read_sensor_bytes) / 2;
    unsigned int offset = ((10 - SystemState.read_sensor_bytes) % 2 == 0) ? 0 : 8;

    for (int i = 7; i >= 0; i--)
    {
      unsigned int sensor_id = bank_number * 16 + offset + i;
      int status = (read_sensor_byte >> (7 - i)) & 0x1;
      if (status && SystemState.sensor_states[sensor_id] != status)
      {
        render_sensor('A' + bank_number, offset + i + 1);
      }
      SystemState.sensor_states[sensor_id] = status;
    }
    SystemState.read_sensor_bytes -= 1;
  }
}

void trainsys_try_serial_out(int curr_tick)
{
  if (curr_tick - SystemState.last_serial_write >= M_WRITE)
  {
    SystemState.last_serial_write = curr_tick;
    while (length(&(SystemState.serial_out)) > 0)
    {
      Putc(SystemState.marklin_tid, pop(&(SystemState.serial_out)));
    }
  }
}

void trainsys_init_track(TrackSwitchPlans track_plan, int curr_tick)
{
  for (int i = 0; i < SWITCH_COUNT; i++)
  {
    int switch_id = i + 1;
    if (switch_id > 18)
    {
      switch_id += 134;
    }

    set_track_switch(switch_id, TRACK_PLANS[track_plan][i]);
  }
  trainsys_try_serial_out(curr_tick);
}
