#ifndef SHAWNCOIN_WALLET_MNEMONIC_HPP
#define SHAWNCOIN_WALLET_MNEMONIC_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace shawncoin {

/** Generate 12-word BIP39 mnemonic (128 bits entropy). */
std::string generateMnemonic12();

/** Mnemonic to 64-byte seed (BIP39). */
std::vector<uint8_t> mnemonicToSeed(const std::string& mnemonic, const std::string& passphrase = "");

} // namespace shawncoin

#endif // SHAWNCOIN_WALLET_MNEMONIC_HPP
