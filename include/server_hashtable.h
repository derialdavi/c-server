#ifndef __SERVER_HASHTABLE_H__
#define __SERVER_HASHTABLE_H__

#include <stdlib.h>
#include "request.h"
#include "response.h"

#define INITIAL_BUCKETS_SIZE ((size_t)7)

struct server_table_pair
{
    char *endpoint;
    void *(*hdl_func)(request*, response*);
    struct server_table_pair *next;
};

typedef struct
{
    struct server_table_pair **buckets;
    size_t buckets_size;
    size_t pair_num;
} server_hashtable;

server_hashtable *hashtable_create();
bool hashtable_put(server_hashtable *, char *, void *(*)(request*, response*));
void *(*hashtable_get(server_hashtable *, char *))(request*, response*);
void hashtable_destroy(server_hashtable *);

#endif
