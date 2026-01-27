#include "server.h"

int main(void)
{
    server_setup(12345);
    server_start_listening();
}