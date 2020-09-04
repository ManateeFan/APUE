//
// Created by liuyifan on 2020/9/3.
//

#ifndef YFNET_NETWORKSOCKET_H
#define YFNET_NETWORKSOCKET_H

#include <iostream>


namespace yfnet
{
    struct NetworkSocket
    {
        using native_handle_type = int;

        static constexpr native_handle_type invaild_handle_value = -1;

        native_handle_type data;

        constexpr NetworkSocket() : data(invaild_handle_value) {}

        constexpr explicit NetworkSocket(native_handle_type d) : data(d) {}

        friend constexpr bool operator==(const NetworkSocket& a, const NetworkSocket& b) noexcept
        {
            return a.data == b.data;
        }

        friend constexpr bool operator!=(const NetworkSocket& a, const NetworkSocket& b) noexcept
        {
            return !(a.data == b.data);
        }
    };

    template<typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const NetworkSocket& addr)
    {
        os << "netops::NetworkSocket(" << addr.data << ")";
        return os;
    }


}
namespace std
{
    template<>
    struct hash<yfnet::NetworkSocket>
    {
        ssize_t operator()(const yfnet::NetworkSocket& s) const noexcept
        {
            return std::hash<yfnet::NetworkSocket::native_handle_type>()(s.data);
        }
    };
}

#endif //YFNET_NETWORKSOCKET_H
