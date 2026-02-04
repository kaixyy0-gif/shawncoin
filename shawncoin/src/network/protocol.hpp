#ifndef SHAWNCOIN_NETWORK_PROTOCOL_HPP
#define SHAWNCOIN_NETWORK_PROTOCOL_HPP

#include "../core/types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace shawncoin {

constexpr uint32_t PROTOCOL_MAGIC_NET = 0x53484157; // SHAW
constexpr uint32_t PROTOCOL_VERSION = 1;

enum MessageType {
    MSG_VERSION, MSG_VERACK, MSG_GETADDR, MSG_ADDR,
    MSG_INV, MSG_GETDATA, MSG_BLOCK, MSG_TX,
    MSG_PING, MSG_PONG, MSG_GETBLOCKS, MSG_GETHEADERS
};

struct CMessageHeader {
    uint32_t magic = PROTOCOL_MAGIC_NET;
    char command[12] = {0};
    uint32_t length = 0;
    uint32_t checksum = 0;
};

std::string messageTypeToString(MessageType t);
bool serializeMessageHeader(const CMessageHeader& h, std::vector<uint8_t>& out);
bool deserializeMessageHeader(const uint8_t* data, size_t len, CMessageHeader& h);

} // namespace shawncoin

#endif // SHAWNCOIN_NETWORK_PROTOCOL_HPP
