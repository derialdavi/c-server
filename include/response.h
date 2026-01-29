#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdint.h>

typedef struct
{
    uint8_t status_code;
    char    status_message[64];
    char    body[1024];
    char    text[4096];
} response;

#endif
