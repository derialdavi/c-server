#include "logger.h"

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

void logger_init(logger *l, const char *filename)
{
    if (filename == NULL)
    {
        l->fd = STDOUT_FILENO;
        return;
    }

    int file_fd = open(filename, O_WRONLY);
    if (file_fd < 0)
    {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    l->fd = file_fd;
}

void logger_print(const logger *l, const char *level, const char *fmt, ...)
{
    /* get current time */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int msg_len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (msg_len > 0)
    {
        char *msg_buf = malloc(msg_len + 1);
        if (msg_buf != NULL)
        {
            vsnprintf(msg_buf, msg_len + 1, fmt, args_copy);

            char *log_line;
            int total_len = asprintf(&log_line, "%s - %s - %s\n",
                                     timestamp, level, msg_buf);

            if (total_len > 0)
            {
                write(l->fd, log_line, total_len);
                free(log_line);
            }
            free(msg_buf);
        }
    }
    va_end(args_copy);
}

void logger_destroy(logger *l)
{
    close(l->fd);
}
