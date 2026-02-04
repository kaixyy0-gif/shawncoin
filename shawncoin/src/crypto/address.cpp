#include "crypto/address.hpp"
#include "crypto/hash.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace shawncoin {

static const char BASE58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string encodeBase58(const uint8_t* data, size_t len) {
    if (!data || len == 0) return "";
    std::vector<uint8_t> digits;
    digits.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        uint32_t carry = data[i];
        for (auto& d : digits) {
            carry += (uint32_t)d << 8;
            d = (uint8_t)(carry % 58);
            carry /= 58;
        }
        while (carry) {
            digits.push_back((uint8_t)(carry % 58));
            carry /= 58;
        }
    }
    std::string result;
    for (size_t i = 0; i < len && data[i] == 0; ++i)
        result += '1';
    for (auto it = digits.rbegin(); it != digits.rend(); ++it)
        result += BASE58_ALPHABET[*it];
    return result;
}

std::vector<uint8_t> decodeBase58(const std::string& s) {
    std::vector<uint8_t> result;
    for (char c : s) {
        const char* p = strchr(BASE58_ALPHABET, c);
        if (!p) return {};
        uint32_t carry = (uint32_t)(p - BASE58_ALPHABET);
        for (size_t j = 0; j < result.size(); ++j) {
            carry += (uint32_t)result[j] * 58;
            result[j] = (uint8_t)(carry & 0xff);
            carry >>= 8;
        }
        while (carry) {
            result.push_back((uint8_t)(carry & 0xff));
            carry >>= 8;
        }
    }
    for (size_t i = 0; i < s.size() && s[i] == '1'; ++i)
        result.insert(result.begin(), 0);
    return result;
}

std::string encodeBase58Check(const uint8_t* payload, size_t len, uint8_t version) {
    if (!payload) return "";
    std::vector<uint8_t> buf;
    buf.reserve(1 + len + 4);
    buf.push_back(version);
    buf.insert(buf.end(), payload, payload + len);
    unsigned char hash[32];
    shawncoin_sha256d(buf.data(), buf.size(), hash);
    buf.insert(buf.end(), hash, hash + 4);
    return encodeBase58(buf.data(), buf.size());
}

std::vector<uint8_t> decodeBase58Check(const std::string& address, uint8_t* out_version) {
    std::vector<uint8_t> decoded = decodeBase58(address);
    if (decoded.size() < 5) return {};
    if (out_version) *out_version = decoded[0];
    std::vector<uint8_t> payload(decoded.begin(), decoded.end() - 4);
    unsigned char hash[32];
    shawncoin_sha256d(payload.data(), payload.size(), hash);
    if (memcmp(hash, &decoded[decoded.size() - 4], 4) != 0) return {};
    return std::vector<uint8_t>(decoded.begin() + 1, decoded.end() - 4);
}

std::string pubKeyHashToAddress(const uint8_t hash160[20]) {
    if (!hash160) return "";
    return encodeBase58Check(hash160, 20, ADDRESS_VERSION);
}

std::vector<uint8_t> addressToPubKeyHash(const std::string& address) {
    uint8_t ver = 0;
    std::vector<uint8_t> hash = decodeBase58Check(address, &ver);
    if (hash.size() != 20 || ver != ADDRESS_VERSION) return {};
    return hash;
}

} // namespace shawncoin
