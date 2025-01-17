#ifndef __LOGGER_H__
#define __LOGGER_H__

/* Simple logger with a couple of logging levels */

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_ALWAYS = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
} LogLevel;

void set_log_level(LogLevel level);
void _log(LogLevel level, char* prefix, char* format, ...);

#define LOG_ERROR(str, ...) _log(LOG_LEVEL_ERROR, "[ERROR] ", (str), ##__VA_ARGS__)
#define LOG_WARN(str, ...) _log(LOG_LEVEL_WARN, "[WARN] ", (str), ##__VA_ARGS__)
#define LOG_INFO(str, ...) _log(LOG_LEVEL_INFO, "[INFO] ", (str), ##__VA_ARGS__)
#define LOG_DEBUG(str, ...) _log(LOG_LEVEL_DEBUG, "[DEBUG] ", (str), ##__VA_ARGS__)

#define PRINT(str, ...) _log(LOG_LEVEL_ALWAYS, "", (str), ##__VA_ARGS__)


#endif // __LOGGER_H__
