#include "util/util.hpp"
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace shawncoin {

std::string hexEncode(const uint8_t* data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out += hex[data[i] >> 4];
        out += hex[data[i] & 0xf];
    }
    return out;
}

std::vector<uint8_t> hexDecode(const std::string& hex) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        int a = std::tolower((unsigned char)hex[i]);
        int b = std::tolower((unsigned char)hex[i + 1]);
        a = (a >= 'a' ? a - 'a' + 10 : (a >= '0' ? a - '0' : -1));
        b = (b >= 'a' ? b - 'a' + 10 : (b >= '0' ? b - '0' : -1));
        if (a < 0 || b < 0) return {};
        out.push_back((uint8_t)((a << 4) | b));
    }
    return out;
}

std::string uint256ToHex(const uint256& h) {
    return hexEncode(h.data(), h.size());
}

uint256 hexToUint256(const std::string& hex) {
    std::vector<uint8_t> v = hexDecode(hex);
    uint256 h{};
    if (v.size() != 32) return h;
    std::copy(v.begin(), v.end(), h.begin());
    return h;
}

} // namespace shawncoin
