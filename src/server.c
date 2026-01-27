#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"
#include "request.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>

bool EOR(const char *buf, const size_t len);
void *endpoint_handler(void *args);

server *server_setup(const int port)
{
    if (port <= 0 || port > 65'535) return NULL;

    /* create and setup socket */
    server *s = malloc(sizeof(server));
    if (s == NULL) return NULL;

    s->s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s->s_fd < 0) return NULL;

    int opt = 1;
    if (setsockopt(s->s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        return NULL;

    /* bind socket with ip and port */
    struct sockaddr_in server_info = { 0 };
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);
    server_info.sin_port = htons(port);
    if (bind(s->s_fd, (const struct sockaddr*)&server_info, sizeof(server_info)) < 0)
        return NULL;

    /* adjust configuration */
    s->s_port = port;
    s->s_backlog = DEFAULT_BACKLOG;
    s->run = false;

    /* print informations */
    logger_init(&s->s_logger, NULL);
    logger_print(&s->s_logger, INFO, "Server created succesfully");

    return s;
}

void server_start_listening(server *s)
{
    if (listen(s->s_fd, s->s_backlog) < 0)
    {
        logger_print(&s->s_logger, ERROR, "Error while listening new connections");
        exit(EXIT_FAILURE);
    }

    s->run = true;
    logger_print(&s->s_logger, INFO, "Listening for new connections at %d\n", s->s_port);
    while (s->run)
    {
        /* accept client */
        struct sockaddr_in client_info;
        int client_len = sizeof(client_info);
        int client_sfd = accept(s->s_fd, (struct sockaddr*)&client_info, (socklen_t*)&client_len);
        if (client_sfd < 0)
        {
            logger_print(&s->s_logger, ERROR, "Error accepting client");
            continue;
        }

        /* recieve client data */
        char client_buf[DEFAULT_CLIENT_BUFFER_SIZE];
        char *client_message = malloc(0);
        int bytes_read, i = 0, total_length = 0;
        while ((bytes_read = recv(client_sfd, client_buf, DEFAULT_CLIENT_BUFFER_SIZE, 0)) > 0)
        {
            total_length += bytes_read;
            client_message = realloc(client_message, total_length + 1);
            strncpy(client_message + (total_length - bytes_read), client_buf, bytes_read);

            /* check for end of request message since recv waits until connection is closed */
            if (EOR(client_buf, bytes_read))
                break;
            i++;
        }
        client_message[total_length] = '\0';

        request req;
        req.client_fd = client_sfd;
        sscanf(client_message, "%s %s HTTP/1.1", req.method, req.endpoint);
        logger_print(&s->s_logger, INFO, "%s %s", req.method, req.endpoint);

        if (strncmp("/", req.endpoint, strlen(req.endpoint)) == 0)
        {
            request *req_ptr = malloc(sizeof(request));
            memcpy(req_ptr, &req, sizeof(request));

            pthread_t thread;
            pthread_create(&thread, NULL, endpoint_handler, req_ptr);
            pthread_detach(thread);
        }

        free(client_message);
    }
}

bool EOR(const char *buf, const size_t len)
{
    return (buf[len-4] == '\r' && buf[len-2] == '\r'
        && buf[len-3] == '\n' && buf[len-1] == '\n');
}

void *endpoint_handler(void *args)
{
    request *req_ptr = (request*)args;
    request req;
    memcpy(&req, req_ptr, sizeof(request));
    free(req_ptr);

    char *response = "HTTP/1.1 200 OK\nServer: CServer\nContent-Length: 5\nContent-Type: text/plain\nConnection: close\n\nCiao\n";
    send(req.client_fd, response, strlen(response), 0);
    close(req.client_fd);

    return NULL;
}
