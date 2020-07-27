#define _GNU_SOURCE 1 //poll监听POLLRDHUP事件需要定义此宏
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <netdb.h>

#define BUFFER_SIZE 84



int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    const char* name = "localhost";
    struct hostent* host = gethostbyaddr(name, 4, AF_INET);
    char buf[BUFFER_SIZE];
    bzero(buf, BUFFER_SIZE);
    strcpy(buf, host->h_name);
    strcat(buf, ":");


    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("connection failed\n");
        close(sockfd);
        return 1;
    }

    pollfd fds[2];
    /* 注册文件描述符0（标准输入）的读事件 */
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    /* 注册文件描述符sockfd的读事件 */
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);


    while (1)
    {
        ret = poll(fds, 2, -1);
        if (ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        if (fds[1].revents & POLLRDHUP)
        {
            printf("server close the connection\n");
            break;
        }
        else if (fds[1].revents & POLLIN)
        {
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }
        if (fds[0].revents & POLLIN)
        {
            char me[BUFFER_SIZE];
            scanf("%s", me);
            strcat(buf, me);
            send(sockfd, buf, BUFFER_SIZE, 0);
        }
    }
    close(sockfd);
    return 0;
}