#include "wallet/hdwallet.hpp"
#include "wallet/mnemonic.hpp"
#include "crypto/hash.h"
#include "crypto/keys.h"
#include "crypto/address.hpp"
#include <cstring>
#include <memory>

namespace shawncoin {

bool HDWallet::setSeed(const std::vector<uint8_t>& seed) {
    if (seed.size() < 32) return false;
    seed_ = seed;
    return true;
}

std::vector<uint8_t> HDWallet::derivePrivateKey(uint32_t index) const {
    unsigned char buf[36];
    if (seed_.size() < 32) return {};
    memcpy(buf, seed_.data(), 32);
    buf[32] = (index >> 0) & 0xff;
    buf[33] = (index >> 8) & 0xff;
    buf[34] = (index >> 16) & 0xff;
    buf[35] = (index >> 24) & 0xff;
    unsigned char out[32];
    shawncoin_sha256d(buf, 36, out);
    return std::vector<uint8_t>(out, out + 32);
}

std::string HDWallet::deriveAddress(uint32_t index) const {
    std::vector<uint8_t> priv = derivePrivateKey(index);
    if (priv.size() != 32) return "";
    shawncoin_keypair_t* key = shawncoin_keypair_from_priv(priv.data());
    if (!key) return "";
    unsigned char pub[33];
    if (shawncoin_pubkey_get(key, pub) != 1) {
        shawncoin_keypair_destroy(key);
        return "";
    }
    unsigned char hash160[20];
    shawncoin_hash160(pub, 33, hash160);
    std::string addr = pubKeyHashToAddress(hash160);
    shawncoin_keypair_destroy(key);
    return addr;
}

} // namespace shawncoin
