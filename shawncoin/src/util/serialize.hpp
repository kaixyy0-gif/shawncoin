#ifndef SHAWNCOIN_UTIL_SERIALIZE_HPP
#define SHAWNCOIN_UTIL_SERIALIZE_HPP

#include "../core/types.hpp"
#include <vector>
#include <cstdint>

namespace shawncoin {

inline void serializeU32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(v & 0xff);
    out.push_back((v >> 8) & 0xff);
    out.push_back((v >> 16) & 0xff);
    out.push_back((v >> 24) & 0xff);
}

inline void serializeU64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back((v >> (i * 8)) & 0xff);
}

inline void serializeVarInt(std::vector<uint8_t>& out, size_t n) {
    if (n < 0xfd) out.push_back((uint8_t)n);
    else if (n <= 0xffff) {
        out.push_back(0xfd);
        out.push_back(n & 0xff);
        out.push_back((n >> 8) & 0xff);
    } else {
        out.push_back(0xfe);
        out.push_back(n & 0xff);
        out.push_back((n >> 8) & 0xff);
        out.push_back((n >> 16) & 0xff);
        out.push_back((n >> 24) & 0xff);
    }
}

inline void serializeUint256(std::vector<uint8_t>& out, const uint256& h) {
    out.insert(out.end(), h.begin(), h.end());
}

} // namespace shawncoin

#endif // SHAWNCOIN_UTIL_SERIALIZE_HPP
