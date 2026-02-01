#ifndef __HTTP_METHODS_H__
#define __HTTP_METHODS_H__

#define METHOD_NUMBER ((size_t)9)

#define GET     "GET"
#define POST    "POST"
#define PUT     "PUT"
#define DELETE  "DELETE"
#define OPTIONS "OPTIONS"
#define HEAD    "HEAD"
#define CONNECT "CONNECT"
#define TRACE   "TRACE"
#define PATCH   "PATCH"

const char *const http_methods[] = {
    GET,
    POST,
    PUT,
    DELETE,
    OPTIONS,
    PATCH,
    HEAD,
    CONNECT,
    TRACE
};

#endif
