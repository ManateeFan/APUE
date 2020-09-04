//
// Created by liuyifan on 2020/9/3.
//

#ifndef YFNET_NETOPS_H
#define YFNET_NETOPS_H

#include "NetworkSocket.h"
#include <poll.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>


namespace yfnet
{
    using nfds_t = int;
    struct PollDescriptor
    {
        NetworkSocket fd;
        int16_t events;
        int16_t revents;
    };

    NetworkSocket accept(NetworkSocket s, sockaddr* addr, socklen_t* addrlen);

    int bind(NetworkSocket s, const sockaddr* name, socklen_t namelen);

    int close(NetworkSocket s);

    int connect(NetworkSocket s, const sockaddr* name, socklen_t namelen);

    int listen(NetworkSocket s, int backlog);

    int poll(PollDescriptor fds[], nfds_t nfds, int timeout);

    int getpeername(NetworkSocket s, sockaddr* name, socklen_t* namelen);

    int getsockname(NetworkSocket s, sockaddr* name, socklen_t* namelen);

    int getsockopt(NetworkSocket s, int level, int optname, void* optval, socklen_t* optlen);

    int inet_aton(const char* cp, in_addr* inp);

    ssize_t recv(NetworkSocket s, void* buf, size_t len, int flags);

    ssize_t recvfrom(NetworkSocket s, void* buf, size_t len, int flags, sockaddr* from, socklen_t* fromlen);

    ssize_t recvmsg(NetworkSocket s, msghdr* message, int flags);

    ssize_t send(NetworkSocket s, const void* buf, size_t len, int flags);

    ssize_t sendto(NetworkSocket s, const void* buf, size_t len, int flags, const sockaddr* to, socklen_t tolen);

    ssize_t sendmsg(NetworkSocket socket, const msghdr* message, int flags);

    int setsockopt(NetworkSocket s, int level, int optname, const void* optval, socklen_t optlen);

    int shutdown(NetworkSocket s, int how);

    NetworkSocket socket(int af, int type, int protocol);

    int socketpair(int domain, int type, int protocol, NetworkSocket sv[2]);

    int set_socket_non_blocking(NetworkSocket s);

    int set_socket_close_on_exec(NetworkSocket s);
}

#endif //YFNET_NETOPS_H
