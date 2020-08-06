#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0);

ssize_t readn(int fd, void *buf, size_t n)
{
    int nleft = n;
    int nread = 0;
    char *p = (char *)buf;
    while (nleft > 0)
    {
        if ((nread = read(fd, p, nleft)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        else if (nread == 0)
        {
            return n - nleft;
        }
        p += nread;
        nleft -= nread;
    }
    return n;
}

ssize_t writen(int fd, void *buf, size_t n)
{
    int nwrite = 0;
    int nleft = n;
    char *p = (char *)buf;
    while (nleft > 0)
    {
        if ((nwrite = write(fd, p, nleft)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        else if (nwrite == 0)
        {
            continue;
        }
        p += nwrite;
        nleft -= nwrite;
    }
    return n;
}

struct Packet
{
    size_t len;
    char buf[BUFSIZ];
};

int main(int argc, char **argv)
{
    // 1. 创建套接字
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // inet_aton("127.0.0.1", &servaddr.sin_addr);

    // 3. 请求链接
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof servaddr) < 0)
    {
        ERR_EXIT("connect");
    }

    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        ERR_EXIT("fork");
    }
    if (pid == 0)
    {
        signal(SIGUSR1, [](int sig) {
            printf("signal = %d\n", sig);
            exit(EXIT_SUCCESS);
        });
        Packet sendPacket;
        while (fgets(sendPacket.buf, sizeof(sendPacket.buf), stdin) != NULL) // 键盘输入获取
        {
            int n = strlen(sendPacket.buf);
            sendPacket.len = htonl(n);
            writen(sockfd, &sendPacket, n + sizeof(sendPacket.len)); // 写入服务器
            memset(&sendPacket, 0, sizeof(sendPacket));
        }
        printf("child close\n");
        return 0;
    }
    else
    {
        Packet recvPacket;
        while (1)
        {
            memset(&recvPacket, 0, sizeof(recvPacket));
            int ret = readn(sockfd, &recvPacket.len, sizeof(recvPacket.len));
            if (ret == -1)
            {
                ERR_EXIT("readn");
            }
            else if (ret < sizeof(recvPacket.len))
            {
                printf("client close\n");
                break;
            }
            int n = ntohl(recvPacket.len);
            ret = readn(sockfd, recvPacket.buf, n);
            if (ret == -1)
            {
                ERR_EXIT("readn");
            }
            else if (ret < n)
            {
                printf("client close\n");
                break;
            }
            fputs(recvPacket.buf, stdout);
            memset(&recvPacket, 0, sizeof(recvPacket));
        }
        printf("parent close\n");
        kill(pid, SIGUSR1);
        // 5. 断开连接
        close(sockfd);
        exit(EXIT_SUCCESS);
    }
}
