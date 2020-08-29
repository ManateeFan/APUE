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
        printf("usage:%s ip_address port_number send_bufer_size\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int recvbuf = atoi(argv[3]);
    int len = sizeof(recvbuf);
    /* 设置tcp发送缓冲区大小*/
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &recvbuf, sizeof(recvbuf));
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &recvbuf, (socklen_t *)&len);
    printf("the tcp send buffer size after setting is %d\n", recvbuf);

    int ret = bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlength);
    if (connfd < 0)
    {
        perror("accept error\n");
    }
    else
    {
        char buffer[BUFSIZ];
        memset(buffer, '\0', BUFSIZ);
        while ((recv(connfd, buffer, BUFSIZ - 1, 0)) > 0)
        {
        }
        close(connfd);
    }

    close(sock);
    return 0;
}