#include "server.h"
#include "server_hashtable.h"

#include <stdio.h>
#include <string.h>

void *handler1(request *context, response *res)
{
    printf("Gli headers sono:\n%s\n", context->headers);
    strcpy(res->body, "Handler 1");
    return NULL;
}
void *handler2(request *context, response *res)
{
    printf("Gli headers sono %s\n", context->headers);
    strcpy(res->body, "Handler 2");
    return NULL;
}

int main(void)
{

    server_setup(12345, NULL);

    server_add_endpoint(GET, "/", handler1);
    server_add_endpoint(GET, "/api/v1/", handler2);
    server_add_endpoint(POST, "/api/v1/", handler2);

    server_start_listening();

    return 0;
}