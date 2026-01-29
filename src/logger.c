#include "logger.h"

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

const char* get_level_color(const char *level);

bool logger_init(logger *l, const char *filename)
{
    pthread_mutex_init(&l->mutex, NULL);

    if (filename == NULL)
    {
        l->fd = STDOUT_FILENO;
        l->is_terminal = true;
        return true;
    }
    int file_fd = open(filename, O_WRONLY | O_CREAT);
    if (file_fd < 0)
    {
        perror("Error opening log file");
        return false;
    }

    l->fd = file_fd;
    l->is_terminal = isatty(l->fd);
    return true;
}

void logger_print(const logger *l, const char *level, const char *fmt, ...)
{
    /* get current time */
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &t);

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int msg_len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (msg_len <= 0)
    {
        va_end(args_copy);
        return;
    }

    char *msg_buf = malloc(msg_len + 1);
    if (msg_buf == NULL)
    {
        va_end(args_copy);
        return;
    }

    vsnprintf(msg_buf, msg_len + 1, fmt, args_copy);
    va_end(args_copy);

    const char *color = get_level_color(level);

    char *log_line;
    int total_len = asprintf(&log_line, "%s - %s%s%s - %s\n",
                             timestamp, color, level, COLOR_RESET, msg_buf);
    free(msg_buf);

    if (total_len > 0)
    {
        pthread_mutex_lock((pthread_mutex_t*)&l->mutex);

        ssize_t written = 0;
        while (written < total_len)
        {
            ssize_t n = write(l->fd, log_line + written, total_len - written);
            if (n < 0) {
                if (errno == EINTR) continue;
                break;
            }
            written += n;
        }
        pthread_mutex_unlock((pthread_mutex_t*)&l->mutex);
        free(log_line);
    }
}

void logger_destroy(logger *l)
{
    close(l->fd);
}

const char* get_level_color(const char *level)
{
    if (strcasecmp(level, "ERROR") == 0)
        return COLOR_ERROR;
    else if (strcasecmp(level, "WARNING") == 0)
        return COLOR_WARNING;
    else
        return COLOR_INFO;
}
