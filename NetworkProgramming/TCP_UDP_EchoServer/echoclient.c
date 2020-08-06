#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1119);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connfd = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (connfd < 0)
    {
        fprintf(stderr, "connect");
        return 1;
    }

    char sendBuf[BUFSIZ];
    char recvBuf[BUFSIZ];
    while (fgets(sendBuf, sizeof(sendBuf), stdin) != NULL)
    {
        write(sock, sendBuf, strlen(sendBuf));
        read(sock, recvBuf, sizeof(recvBuf));
        fputs(recvBuf, stdout);
        memset(recvBuf, 0, sizeof(recvBuf));
        memset(sendBuf, 0, sizeof(sendBuf));
    }
    close(sock);
    return 0;
}
