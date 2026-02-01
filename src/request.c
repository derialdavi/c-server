#include "request.h"

#include "server.h"
#include <stdio.h>
#include <string.h>

extern server *s;

bool is_valid_method(const char* method_string);

bool request_parse(const char *request_string, char *error_message, request *req)
{
    int n;

    /* validate request */
    if (sscanf(request_string, "%7s %255s HTTP/%f\r\n%n",
        req->method, req->path, &req->httpv, &n) != 3)
    {
        strcpy(error_message, "Invalid request format");
        return false;
    }

    if (is_valid_method(req->method) == false)
    {
        sprintf(error_message, "Invalid request method: %s", req->method);
        return false;
    }

    const char *headers_end = strstr(request_string, "\r\n\r\n");
    if (headers_end == NULL)
    {
        strcpy(error_message, "End of headers not found");
        return false;
    }

    /* get function handler */
    size_t key_len = strlen(req->method) + strlen(req->path) + 2;
    char *key = malloc(key_len);
    if (key == NULL)
    {
        strcpy(error_message, "Memory error");
        return false;
    }

    snprintf(key, key_len, "%s %s", req->method, req->path);
    req->hdl_func = hashtable_get(s->s_ht, key);
    free(key);

    /* get headers */
    const char *headers_start = request_string + n;
    size_t headers_len = headers_end - headers_start;
    req->headers = malloc(headers_len);
    if (req->headers == NULL)
    {
        strcpy(error_message, "Memory error");
        return false;
    }

    strncpy(req->headers, headers_start, headers_len);
    // req->headers[headers_len] =   '\0';

    // TODO: read body

    return true;
}

bool is_valid_method(const char* method_string)
{
    for (size_t i = 0; i < METHOD_NUMBER; i++)
        if (strcmp(method_string, http_methods[i]) == 0)
            return true;

    return false;
}
