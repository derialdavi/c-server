#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include "logger.h"

#define DEFAULT_BACKLOG ((int)5)
#define DEFAULT_CLIENT_BUFFER_SIZE ((size_t)4096)

typedef struct
{
    int s_fd;
    int s_port;
    int s_backlog;
    logger s_logger;
    bool run;
} server;

server *server_setup(const int);
void server_start_listening();
void server_destroy();

#endif
