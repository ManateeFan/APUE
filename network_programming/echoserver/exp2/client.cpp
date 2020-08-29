#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

int main(int argc, char** argv)
{
    int sockfd;
    struct sockaddr_in server_addr;
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <IP>\n", argv[0]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 11111;
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    char send_buf[BUFSIZ];
    char recv_buf[BUFSIZ];
    while(fgets(send_buf, BUFSIZ, stdin))
    {
        write(sockfd, send_buf, strlen(send_buf));
        if(read(sockfd, recv_buf, BUFSIZ) == 0)
        {
            break;
        }
        fputs(recv_buf, stdout);
    }
    exit(EXIT_SUCCESS);
}