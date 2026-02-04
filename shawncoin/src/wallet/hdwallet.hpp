#ifndef SHAWNCOIN_WALLET_HDWALLET_HPP
#define SHAWNCOIN_WALLET_HDWALLET_HPP

#include "../core/types.hpp"
#include "../crypto/address.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace shawncoin {

/** BIP32-style key derivation (simplified). */
class HDWallet {
public:
    bool setSeed(const std::vector<uint8_t>& seed);
    std::vector<uint8_t> derivePrivateKey(uint32_t index) const;
    std::string deriveAddress(uint32_t index) const;
private:
    std::vector<uint8_t> seed_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_WALLET_HDWALLET_HPP
