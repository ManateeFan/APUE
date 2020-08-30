#include <iostream>
#include "process_pool.hpp"
#include "CgiConn.h"

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        fprintf(stderr, "usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    auto pool = ProcessPool<CgiConn>::Create(listenfd);
    if (pool)
    {
        pool->Run();
        delete pool;
    }
    close(listenfd);
}