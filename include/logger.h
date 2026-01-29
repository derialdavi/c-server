#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <pthread.h>

#define INFO    "INFO"
#define WARNING "WARNING"
#define ERROR   "ERROR"

#define COLOR_ERROR   "\033[37;41m"
#define COLOR_WARNING "\033[30;43m"
#define COLOR_INFO    "\033[0m"
#define COLOR_RESET   "\033[0m"

typedef struct
{
    int fd;
    bool is_terminal;
    pthread_mutex_t mutex;
} logger;

bool logger_init(logger *, const char *);
void logger_print(const logger *, const char *, const char *, ...);
void logger_destroy(logger *);

#endif
