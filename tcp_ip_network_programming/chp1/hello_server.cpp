#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <assert.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(serv_sock != -1);

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    memset(&serv_addr, 0, sizeof(client_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    int ret = bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    ret = listen(serv_sock, 5);
    assert(ret != -1);

    client_addr_len = sizeof(client_addr);
    int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    assert(client_sock != -1);

    const char *msg = "Hello world!";
    write(client_sock, msg, sizeof(msg));

    close(serv_sock);
    close(client_sock);

    return 0;
}