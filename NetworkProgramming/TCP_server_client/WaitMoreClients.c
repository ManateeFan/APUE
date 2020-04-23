#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

void str_cli(FILE *fp, int sockfd)
{
    char line[BUFSIZ];
    char get[BUFSIZ];

    while (fgets(line, BUFSIZ, fp) != NULL)
    {
        write(sockfd, line, sizeof(line));
        if (read(sockfd, get, BUFSIZ) > 0)
        {
            fputs(get, stdout);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        printf("usage:%s ip_address port_number\n", argv[0]);
        return 1;
    }
    int sockfds[5];
    struct sockaddr_in serverAddr;
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    for (int i = 0; i < 5; i++)
    {
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &serverAddr.sin_addr);

        sockfds[i] = socket(PF_INET, SOCK_STREAM, 0);
        assert(sockfds[i] >= 0);

        int ret = connect(sockfds[i], (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        assert(ret != -1);
    }

    str_cli(stdin, sockfds[0]);
    exit(0);
}