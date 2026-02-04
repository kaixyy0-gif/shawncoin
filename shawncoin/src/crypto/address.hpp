#ifndef SHAWNCOIN_CRYPTO_ADDRESS_HPP
#define SHAWNCOIN_CRYPTO_ADDRESS_HPP

#include "../core/types.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace shawncoin {

// Shawn Coin address version byte (prefix 'S' in Base58)
constexpr uint8_t ADDRESS_VERSION = 0x3f; // Produces 'S' in Base58Check

/** Encode payload (e.g. 20-byte hash160) to Base58Check with Shawn prefix */
std::string encodeBase58Check(const uint8_t* payload, size_t len, uint8_t version = ADDRESS_VERSION);

/** Decode Base58Check address to payload; returns empty on error */
std::vector<uint8_t> decodeBase58Check(const std::string& address, uint8_t* out_version = nullptr);

/** Create Shawn address from 20-byte hash160 (RIPEMD160(SHA256(pubkey))) */
std::string pubKeyHashToAddress(const uint8_t hash160[20]);

/** Validate and decode address; returns hash160 (20 bytes) or empty */
std::vector<uint8_t> addressToPubKeyHash(const std::string& address);

/** Base58 encode bytes */
std::string encodeBase58(const uint8_t* data, size_t len);

/** Base58 decode; returns bytes or empty */
std::vector<uint8_t> decodeBase58(const std::string& s);

} // namespace shawncoin

#endif // SHAWNCOIN_CRYPTO_ADDRESS_HPP
