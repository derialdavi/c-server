#ifndef __LOGGER_H__
#define __LOGGER_H__

#define INFO "INFO"
#define WARNING "WARNING"
#define ERROR "ERROR"

typedef struct
{
    int fd;
} logger;

void logger_init(logger *, const char *);
void logger_print(const logger *, const char *, const char *, ...);
void logger_destroy(logger *);

#endif