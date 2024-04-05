#ifndef __RENDER_H__
#define __RENDER_H__

#include "stdint.h"
#include "lib/stdlib.h"

static const unsigned int CONSOLE_MAX_LINES = 16;
static const unsigned int SENSOR_MAX_ENTRIES = 35;

typedef struct
{
  int cmd_line_history;
  int pacman_line_history;
  unsigned int sensor_count;
  BQueue output_queue;
  int console_server_tid;
} TermUIState;

void render_init();
void render_time(int time);
void render_command(char *fmt, ...);
void render_pacman_command(char *fmt, ...);
void render_char(unsigned char ch, int prompt_length);
void render_backspace(int prompt_length);
void render_prompt_clear();
void render_switch(int32_t switch_id, SwitchMode switch_mode);
void render_sensor(char bank, unsigned int sensor_number);
void render_perf_stats(int percentage);
void render_debug_log(int message);
void render_predict_current_sensor(int train, int sensor_id);
void render_predict_next_sensor(int train, int sensor_id);
void render_train_destination(int train, int sensor_id);
void render_predict_t_error(int train, int terr); // terr in ticks
void render_predict_d_error(int train, int derr); // derr in mm
void render_reserve_zone(int train, int zone);
void render_unreserve_zone(int zone);
void render_empty_food(int sensor_id);
void render_food(int sensor_id);
void render_pacman(int sensor_id);
void render_ghost(int sensor_id);
void render_pacman_score(int score);
void clear_screen();
void clear_console();
string get_sensor_string(int sensor_id);

/*
void update_switch(const char* prompt);
void clear_console(uint32_t switch_id, char mode);
void clear_sensors_view(void);
*/
#endif // __RENDER_H__
