#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdint.h>
#include "response.h"

typedef struct request_t
{
    uint8_t client_fd;
    char    method[8];
    char    path[256];
    float   httpv;
    char   *headers;
    char   *body;
    void   *(*hdl_func)(struct request_t*, response*);
} request;

bool request_parse(const char *, char *, request *);

#endif
