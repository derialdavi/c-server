#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdint.h>

typedef struct
{
    uint8_t client_fd;
    char    method[8];
    char    endpoint[256];
} request;

#endif
