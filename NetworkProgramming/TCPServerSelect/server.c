#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
int main()
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE]; // 就绪描述符数量、 已连接socket数量
    fd_set rset, allset;
    char buf[BUFSIZ];
    char str[16];
    socklen_t clilen;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr, cliAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(11111);

    int ret = bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    ret = listen(listenfd, 100);

    maxfd = listenfd; // current max fd
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) // listenfd可读，表明有新客户连接
        {
            clilen = sizeof(cliAddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliAddr, &clilen);
            printf("received from %s at port %d\n",
                inet_ntop(AF_INET, &cliAddr.sin_addr.s_addr, str, sizeof(str)),
                ntohs(cliAddr.sin_port));

            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                write(STDOUT_FILENO, "Too many clients\n",
                    sizeof("Too many clients\n"));
                exit(1);
            }

            FD_SET(connfd, &allset); //连接成功后加入集合
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--nready <= 0) {
                continue; // 无可读描述符
            }
        }

        for (i = 0; i < maxi; i++) {
            int n;
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = read(sockfd, buf, BUFSIZ)) == 0) // connection closed by client;
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else if (n > 0) {
                    for (int j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    write(sockfd, buf, sizeof(buf));
                }
                if (--nready == 0)
                    break;
            }
        }
    }
    exit(0);
}