#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include <pthread.h>
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
    pthread_mutex_t s_mutex;
    bool run;
} server;

void server_setup(const int, const char *);
bool server_add_endpoint(char *, char *, void*(*)(request*, response*));
void server_start_listening();
void server_destroy();

#endif
