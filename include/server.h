#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include "server_hashtable.h"
#include "logger.h"

#define DEFAULT_BACKLOG            ((int)5)
#define DEFAULT_CLIENT_BUFFER_SIZE ((size_t)4096)

#define GET     "GET"
#define POST    "POST"
#define OPTIONS "OPTIONS"
#define DELETE  "DELETE"
#define PUT     "PUT"
#define PATCH   "PATCH"

typedef struct
{
    int s_fd;
    int s_port;
    int s_backlog;
    logger s_logger;
    server_hashtable *s_ht;
    bool run;
} server;

server *server_setup(const int);
void server_start_listening();
bool server_add_endpoint(char *, char *, void*(*)(request*, response*));
void server_destroy();

#endif
