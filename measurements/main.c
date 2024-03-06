#include <ctype.h>
#include <stdbool.h>
#include "rpi.h"
#include "marklin.h"
#include "util.h"

#define BYTE_COUNT 10

static uint32_t sensor_state[BYTE_COUNT] = {0};
static uint32_t prev_sensor_state[BYTE_COUNT] = {0};

// Initialize switches for track A
void commander_init_switches()
{
  uint8_t currentIndex = 0; // track A

  for (int i = 1; i < 23; ++i)
  {
    uint8_t switch_num = i;
    if (i > 18)
    {
      switch_num += 134; // accounting for last three switches
    }

    marklin_switch_control(TRACK_PLANS[currentIndex][i - 1], switch_num);
  }
}

uint32_t query_sensor(size_t sensor_group)
{

  marklin_pick_s88(sensor_group);

  // seperate loop to ensure that we update prev state of other byte as well
  for (int offset = 0; offset < 2; ++offset)
  {
    size_t i = sensor_group * 2 + offset;

    uint8_t sensor_byte = uart_getc(MARKLIN);

    prev_sensor_state[i] = sensor_state[i];
    sensor_state[i] = sensor_byte;
  }

  for (int offset = 0; offset < 2; ++offset)
  {
    size_t i = sensor_group * 2 + offset;

    uint8_t triggered = ~(prev_sensor_state[i]) & sensor_state[i];
    for (uint32_t j = 0; j < 8; ++j)
    {
      if (((triggered >> j) & 0x1) == 1)
      {
        uint8_t sensor_index = (7 - j);
        return i * 8 + sensor_index;
      }
    }
  }
  return 0;
}

void measureTrainSpeed()
{

  const uint8_t train_number = 2;
  const uint8_t train_speed = 13;
  uart_printf(CONSOLE, "Calculating train velocity for train %d speed %d\r\n", train_number, train_speed);
  marklin_train_control(train_speed, train_number);

  // Clear sensor detections
  marklin_dump_s88_all();
  for (int i = 0; i < BYTE_COUNT; ++i)
  {
    uart_getc(MARKLIN);
  }

  const uint32_t SAMPLES = 20;
  uint32_t BANKS[] = {3, 2, 4, 5, 5, 4, 5, 4, 2, 3, 1, 2};
  uint32_t SENSOR[] = {10, 1, 14, 14, 9, 5, 6, 4, 6, 12, 4, 16};
  uint64_t DISTANCES[] = {128 + 231, 404, 239 + 43, 376, 239 + 155 + 239, 376, 50 + 239, 404, 231 + 120, 333 + 43, 437, 50 + 326};
  int SENSOR_COUNT = 12;
  uint64_t CUM_SPEEDS[SENSOR_COUNT];
  for (int i = 0; i < SENSOR_COUNT; ++i)
  {
    CUM_SPEEDS[i] = 0;
  }

  uint64_t prev_time = 0;
  uint64_t current_time = 0;

  for (uint32_t sample = 0; sample < SAMPLES + 1; ++sample)
  {

    int current_sensor = 0;

    while (current_sensor < SENSOR_COUNT)
    {
      int current_sensor_index = current_sensor % SENSOR_COUNT;

      uint32_t next_group = BANKS[current_sensor_index];
      uint32_t next_sensor = SENSOR[current_sensor_index];

      uint32_t triggered = query_sensor(next_group);
      if (triggered == 0)
        // nothing happened
        continue;

      char sensor_group = (triggered / 16);
      uint8_t sensor_index = (triggered % 16) + 1;
      if (sensor_group == next_group && sensor_index == next_sensor)
      {
        prev_time = current_time;
        current_time = get_time();
        if (current_sensor == 0 || sample == 0)
        {
          current_sensor++;
          continue;
        }

        // Speed in a certain section
        uint32_t speed_index = (current_sensor - 1 + SENSOR_COUNT) % SENSOR_COUNT;
        uint64_t speed = DISTANCES[speed_index] * 1000000000 / (current_time - prev_time);
        CUM_SPEEDS[speed_index] += speed;
        uart_printf(CONSOLE, "Set speed %d to %d\r\n", speed_index, speed);

        if (sample == SAMPLES)
        {
          uint32_t total = 0;
          for (int i = 0; i < SENSOR_COUNT; ++i)
          {
            total += CUM_SPEEDS[i];
          }
          uart_printf(CONSOLE, "Average speed (overall): %d\r\n", total / (SENSOR_COUNT * SAMPLES));
          break;
        }
        ++current_sensor;
      }
    }

    uint32_t total = 0;
    for (int i = 0; i < SENSOR_COUNT; ++i)
    {
      total += CUM_SPEEDS[i];
    }
    uart_printf(CONSOLE, "Average speed (overall): %d\r\n", total / (SENSOR_COUNT * sample));
    uart_printf(CONSOLE, "sample %d complete\r\n", sample);
  }
}

static void mainloop()
{
  // Initializes the switches to a known state
  commander_init_switches();

  measureTrainSpeed();
}

int kmain()
{
  // set up GPIO pins for both console and marklin uarts
  gpio_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyway, so we know what state it is in
  uart_config_and_enable(CONSOLE);
  uart_config_and_enable(MARKLIN);

  mainloop();

  // exit to boot loader
  return 0;
}
