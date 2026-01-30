#include "server.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "logger.h"
#include "request.h"
#include <pthread.h>
#include "response.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include "server_hashtable.h"

bool EOR(const char *buf, const size_t len);
void *endpoint_handler(void *args);
void sig_shutdown_server(int sig);

/* declaring the server variable global in order
 * to make operations on it when terminating the
 * process.
 */
static server *s;

void server_setup(const int port, const char *filename)
{
    s = malloc(sizeof(server));
    if (s == NULL)
    {
        perror("Error allocating memory for server");
        exit(EXIT_FAILURE);
    }

    if (logger_init(&s->s_logger, filename) == false)
    {
        free(s);
        exit(EXIT_FAILURE);
    }

    if (port <= 0 || port > 65'535)
    {
        logger_print(&s->s_logger, ERROR, "Invalid port number (%d)", port);
        free(s);
        exit(EXIT_FAILURE);
    }

    /* create and setup socket */
    s->s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s->s_fd < 0)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, ERROR, "Error creating socket: %s", err_message);
        free(s);
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(s->s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, ERROR, "Error setting up socket: %s", err_message);
        free(s);
        exit(EXIT_FAILURE);
    }

    /* bind socket with ip and port */
    struct sockaddr_in server_info = { 0 };
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);
    server_info.sin_port = htons(port);
    if (bind(s->s_fd, (const struct sockaddr*)&server_info, sizeof(server_info)) < 0)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, ERROR, "Error binding socket: %s", err_message);
        free(s);
        exit(EXIT_FAILURE);
    }

    /* adjust configuration */
    s->s_port = port;
    s->s_backlog = DEFAULT_BACKLOG;
    s->run = false;
    pthread_mutex_init(&s->s_mutex, NULL);
    s->s_ht = hashtable_create();
    if (s->s_ht == NULL)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, ERROR, "Error allocating data for the server: %s", err_message);
        free(s);
        exit(EXIT_FAILURE);
    }

    /* print informations */
    logger_init(&s->s_logger, filename);
    logger_print(&s->s_logger, INFO, "Server created succesfully");
}

bool server_add_endpoint(char *method, char *endpoint, void *(*hdl_func)(request*, response*))
{
    if (method == NULL || endpoint == NULL || hdl_func == NULL)
    {
        logger_print(&s->s_logger, WARNING, "Invalid endpoint or handler function", method, endpoint);
        return false;
    }

    size_t size = strlen(method) + strlen(endpoint) + 2;
    char *key = malloc(size);
    if (key == NULL)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, WARNING, "Failed to add |%s %s| endpoint: %s", method, endpoint, err_message);
        return false;
    }

    snprintf(key, size, "%s %s", method, endpoint);

    if (hashtable_put(s->s_ht, key, hdl_func) == false)
    {
        char *err_message = strerror(errno);
        logger_print(&s->s_logger, WARNING, "Failed to add |%s %s| endpoint: %s", method, endpoint, err_message);
        return false;
    }

    free(key);
    return true;
}

void server_start_listening()
{
    if (listen(s->s_fd, s->s_backlog) < 0)
    {
        logger_print(&s->s_logger, ERROR, "Error while listening new connections");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_shutdown_server);

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

        char method[8];
        char endpoint[256];
        sscanf(client_message, "%s %s HTTP/1.1", method, endpoint);

        size_t size = strlen(method) + strlen(endpoint) + 2;
        char *key = malloc(size);
        snprintf(key, size, "%s %s", method, endpoint);
        void *(*hdl_func)(request*, response*) = hashtable_get(s->s_ht, key);
        free(key);

        /* TODO: handle non exisisting path */
        if (hdl_func == NULL) continue;

        /* lock the resource so that if server recieves a   */
        /* new request it doesn't overwrite the memory that */
        /* the old thread points to.                        */
        pthread_mutex_lock(&s->s_mutex);
        request req;
        req.client_fd = client_sfd;
        strncpy(req.method, method, sizeof(req.method));
        strncpy(req.endpoint, endpoint, sizeof(req.endpoint));
        req.hdl_func = hdl_func;

        pthread_t thread;
        pthread_create(&thread, NULL, endpoint_handler, &req);
        pthread_detach(thread);

        free(client_message);
    }
}

void server_destroy()
{
    s->run = false;
    logger_destroy(&s->s_logger);
    hashtable_destroy(s->s_ht);
    pthread_mutex_destroy(&s->s_mutex);
    close(s->s_fd);
    free(s);
}

bool EOR(const char *buf, const size_t len)
{
    return (buf[len-4] == '\r' && buf[len-2] == '\r'
        && buf[len-3] == '\n' && buf[len-1] == '\n');
}

void *endpoint_handler(void *context)
{

    request req = (*(request*)context);
    pthread_mutex_unlock(&s->s_mutex);

    response res = { .status_code = 200, .status_message = "OK"};

    req.hdl_func(&req, &res);

    snprintf(res.text, sizeof(res.text),
            "HTTP/1.1 %d %s\r\nServer: CServer\r\nContent-Length: %ld\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n%s\r\n",
            res.status_code, res.status_message, strlen(res.body), res.body);

    send(req.client_fd, res.text, strlen(res.text), 0);
    logger_print(&s->s_logger, INFO, "%s %s => Status %d %s", req.method, req.endpoint, res.status_code, res.status_message);
    close(req.client_fd);

    return NULL;
}

void sig_shutdown_server(int sig)
{
    (void)sig;
    server_destroy();
    exit(EXIT_SUCCESS);
}
