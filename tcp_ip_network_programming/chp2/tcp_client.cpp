#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc, char** argv)
{
    int sock;
    struct sockaddr_in serv_addr;
    char msg[30];
    int str_len = 0;
    int idx = 0, read_len = 0;

    if(argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        perror("socket");
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    serv_addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret != 0)
    {
        perror("connect");
        return 1;
    }
    while(read_len = read(sock, &msg[idx++], 1))
    {
        if(read_len == -1)
        {
            perror("read");
            return 1;
        }
        str_len += read_len;
    }

    printf("Message from server : %s\n", msg);
    printf("Function read call count: %d\n", str_len);
    close(sock);
    return 0;
}