//
// Created by liuyifan on 2020/9/3.
//

#include "NetOps.h"

namespace yfnet
{

    template<typename R, typename F, typename... Args>
    inline R wrapSocketFunction(F f, NetworkSocket s, Args... args)
    {
        R ret = f(s.data, args...);
        return ret;
    }

    NetworkSocket accept(NetworkSocket s, sockaddr* addr, socklen_t* addrlen)
    {
        return NetworkSocket(wrapSocketFunction<NetworkSocket::native_handle_type>(::accept, s, addr, addrlen));
    }

    int bind(NetworkSocket s, const sockaddr* name, socklen_t namelen)
    {
        return wrapSocketFunction<int>(::bind, s, name, namelen);
    }

    int close(NetworkSocket s)
    {
        return wrapSocketFunction<int>(::close, s);
    }

    int listen(NetworkSocket s, int backlog)
    {
        return wrapSocketFunction<int>(::listen, s, backlog);
    }

    int connect(NetworkSocket s, const sockaddr* name, socklen_t namelen)
    {
        return wrapSocketFunction<NetworkSocket::native_handle_type>(::connect, s, name, namelen);
    }

    int poll(PollDescriptor fds[], nfds_t nfds, int timeout)
    {
        // reinterpret_cast<T>(expression)
        // 编译期指令，将expression类型当作T类型处理
        auto files = reinterpret_cast<pollfd*>(reinterpret_cast<void*>(fds));
        return ::poll(files, nfds, timeout);
    }

    int getpeername(NetworkSocket s, sockaddr* name, socklen_t* namelen)
    {
        return wrapSocketFunction<int>(::getpeername, s, name, namelen);
    }

    int getsockname(NetworkSocket s, sockaddr* name, socklen_t* namelen)
    {
        return wrapSocketFunction<int>(::getsockname, s, name, namelen);
    }

    int inet_aton(const char* cp, in_addr* inp)
    {
        inp->s_addr = inet_addr(cp);
        return inp->s_addr == INADDR_NONE ? 0 : 1;
    }

    ssize_t recv(NetworkSocket s, void* buf, size_t len, int flags)
    {
        return wrapSocketFunction<ssize_t>(::recv, s, buf, len, flags);
    }

    ssize_t recvfrom(NetworkSocket s, void* buf, size_t len, int flags, sockaddr* from, socklen_t* fromlen)
    {
        return wrapSocketFunction<ssize_t>(::recvfrom, s, buf, len, flags, from, fromlen);
    }

    ssize_t recvmsg(NetworkSocket s, msghdr* message, int flags)
    {
        return wrapSocketFunction<ssize_t>(::recvmsg, s, message, flags);
    }

    ssize_t send(NetworkSocket s, const void* buf, size_t len, int flags)
    {
        return wrapSocketFunction<ssize_t>(::send, s, buf, len, flags);
    }

    ssize_t sendto(NetworkSocket s, const void* buf, size_t len, int flags, const sockaddr* to, socklen_t tolen)
    {
        return wrapSocketFunction<ssize_t>(::sendto, s, buf, len, flags, to, tolen);
    }

    ssize_t sendmsg(NetworkSocket socket, const msghdr* message, int flags)
    {
        return wrapSocketFunction<ssize_t>(::sendmsg, socket, message, flags);
    }

    int setsockopt(NetworkSocket s, int level, int optname, const void* optval, socklen_t optlen)
    {
        return wrapSocketFunction<int>(::setsockopt, s, level, optname, optval, optlen);
    }

    int shutdown(NetworkSocket s, int how)
    {
        return wrapSocketFunction<int>(::shutdown, s, how);
    }

    NetworkSocket socket(int af, int type, int protocol)
    {
        return NetworkSocket(::socket(af, type, protocol));
    }

    int socketpair(int domain, int type, int protocol, NetworkSocket sv[2])
    {
        int pip[2];
        auto r = ::socketpair(domain, type, protocol, pip);
        if (r == -1)
        {
            return r;
        }
        sv[0] = NetworkSocket(pip[0]);
        sv[1] = NetworkSocket(pip[1]);
        return r;
    }

    int set_socket_non_blocking(NetworkSocket s)
    {
        int flags = fcntl(s.data, F_GETFL, 0);
        if (flags == -1)
        {
            return flags;
        }
        return fcntl(s.data, F_SETFL, flags | O_NONBLOCK);
    }

    int set_socket_close_on_exec(NetworkSocket s)
    {
        return fcntl(s.data, F_SETFD, O_CLOEXEC);
    }
}
