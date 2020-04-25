#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc <= 2) {
        printf("usage:%s ip_address port_number\n", argv[0]);
        return 1;
    }
    int sockfd;
    struct sockaddr_in serverAddr;
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    int ret = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    char buf[BUFSIZ];
    while (fgets(buf, sizeof(buf), stdin)) {
        int m = write(sockfd, buf, sizeof(buf));
        int n = read(sockfd, buf, sizeof(buf));
        fputs(buf, stdout);
    }

    exit(0);
}