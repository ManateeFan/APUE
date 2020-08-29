#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>


void strEcho(int connfd)
{
    int n;
    char buf[BUFSIZ];
    while((n = read(connfd, buf, sizeof(buf))) > 0)
    {
        write(connfd, buf, n);
    }
}
int main(int argc, char** argv)
{
    int listenfd, connfd;
    pid_t child_pid;
    socklen_t client_length;
    struct sockaddr_in client_addr, server_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd != -1);

    bzero(&server_addr, sizeof(server_addr));
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htonl(11111);
    server_addr.sin_family = AF_INET;

    int ret = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    for (;;)
    {
        client_length = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_length);
        if((child_pid = fork()) == 0)
        {
            close(listenfd);
            strEcho(connfd);
            exit(EXIT_SUCCESS);
        }
        close(connfd);
    }
}