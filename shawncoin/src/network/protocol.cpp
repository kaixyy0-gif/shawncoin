#include "network/protocol.hpp"
#include "crypto/hash.h"
#include <cstring>
#include <array>

namespace shawncoin {

std::string messageTypeToString(MessageType t) {
    switch (t) {
        case MSG_VERSION: return "version";
        case MSG_VERACK: return "verack";
        case MSG_GETADDR: return "getaddr";
        case MSG_ADDR: return "addr";
        case MSG_INV: return "inv";
        case MSG_GETDATA: return "getdata";
        case MSG_BLOCK: return "block";
        case MSG_TX: return "tx";
        case MSG_PING: return "ping";
        case MSG_PONG: return "pong";
        case MSG_GETBLOCKS: return "getblocks";
        case MSG_GETHEADERS: return "getheaders";
        default: return "unknown";
    }
}

bool serializeMessageHeader(const CMessageHeader& h, std::vector<uint8_t>& out) {
    out.resize(24);
    memcpy(out.data(), &h.magic, 4);
    memcpy(out.data() + 4, h.command, 12);
    memcpy(out.data() + 16, &h.length, 4);
    memcpy(out.data() + 20, &h.checksum, 4);
    return true;
}

bool deserializeMessageHeader(const uint8_t* data, size_t len, CMessageHeader& h) {
    if (len < 24) return false;
    memcpy(&h.magic, data, 4);
    memcpy(h.command, data + 4, 12);
    memcpy(&h.length, data + 16, 4);
    memcpy(&h.checksum, data + 20, 4);
    return true;
}

} // namespace shawncoin
