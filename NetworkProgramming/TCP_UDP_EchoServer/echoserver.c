#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

int main()
{


    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1119);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket");
        return 1;
    }

    if(bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "bind");
        return 1;
    }

    // SOMAXCONN
    // 内核维护2个队列
    // 1. 已收到客户的SYN分节 但未完成三次握手的连接队列
    // 2. 已完成三次握手的连接队列，等待accept返回
    if (listen(listenfd, SOMAXCONN) < 0)
    {
        fprintf(stderr, "listen");
        return 1;
    }

    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    // 从已完成连接队列返回第一个连接，若队列空则阻塞。
    int connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientLen);
    if (connfd < 0)
    {
        fprintf(stderr, "accept");
        return 1;
    }

    char buf[BUFSIZ];
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int ret = read(connfd, buf, sizeof(buf));
        fputs(buf, stdout);
        write(connfd, buf, ret);
    }

    close(listenfd);
    close(connfd);
    return 0;
}
