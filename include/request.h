#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdint.h>
#include "response.h"

typedef struct request_t
{
    uint8_t client_fd;
    char    method[8];
    char    endpoint[256];
    void   *(*hdl_func)(struct request_t*, response*);
} request;

#endif
