#ifndef SHAWNCOIN_UTIL_UTIL_HPP
#define SHAWNCOIN_UTIL_UTIL_HPP

#include "../core/types.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace shawncoin {

std::string hexEncode(const uint8_t* data, size_t len);
std::vector<uint8_t> hexDecode(const std::string& hex);
std::string uint256ToHex(const uint256& h);
uint256 hexToUint256(const std::string& hex);

} // namespace shawncoin

#endif // SHAWNCOIN_UTIL_UTIL_HPP
