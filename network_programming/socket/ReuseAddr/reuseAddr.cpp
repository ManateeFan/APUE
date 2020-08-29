#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        printf("usage:%s ip_address port_number\n", argv[0]);
        return 1;
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); // 重用本地地址

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);
    int ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
}