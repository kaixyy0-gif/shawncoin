#ifndef SHAWNCOIN_NETWORK_PEER_HPP
#define SHAWNCOIN_NETWORK_PEER_HPP

#include "network/netbase.hpp"
#include <string>
#include <cstdint>

namespace shawncoin {

struct Peer {
    CService addr;
    std::string id;
    bool connected = false;
    bool inbound = false;
};

} // namespace shawncoin

#endif // SHAWNCOIN_NETWORK_PEER_HPP
