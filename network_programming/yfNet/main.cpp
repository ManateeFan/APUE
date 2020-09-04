#include <iostream>
#include "NetOps.h"

int main(int argc, char** argv)
{


    {
        using namespace yfnet;
        auto listen_sock = yfnet::socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serv_addr;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;

        memset(&serv_addr, 0, sizeof(client_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(11111);

        yfnet::bind(listen_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

        yfnet::listen(listen_sock, 5);


        client_addr_len = sizeof(client_addr);
        auto client_sock = yfnet::accept(listen_sock, (struct sockaddr*) &client_addr, &client_addr_len);

        printf("Hello\n");
        const char* msg = "Hello world!";
        yfnet::send(client_sock, msg, strlen(msg), 0);

        yfnet::close(listen_sock);
        yfnet::close(client_sock);
//        NetworkSocket sk(10);
//        std::hash<NetworkSocket> tmp;
//        std::cout << tmp(sk) << "\n";
    }

}
