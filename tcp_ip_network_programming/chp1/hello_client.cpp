#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <assert.h>

int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in serv_addr;
    char msg[30];
    int str_len;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != -1);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    assert(ret != -1);

    str_len = read(sock, msg, sizeof(msg));
    assert(str_len != -1);

    printf("Message from server: %s\n", msg);
    close(sock);

    return 0;
}