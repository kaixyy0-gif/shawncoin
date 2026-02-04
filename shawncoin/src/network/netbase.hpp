#ifndef SHAWNCOIN_NETWORK_NETBASE_HPP
#define SHAWNCOIN_NETWORK_NETBASE_HPP

#include <string>
#include <cstdint>

namespace shawncoin {

struct CNetAddr {
    std::string host;
    uint16_t port = 0;
    bool isIPv4() const;
    bool isIPv6() const;
};

struct CService : CNetAddr {};

} // namespace shawncoin

#endif // SHAWNCOIN_NETWORK_NETBASE_HPP
