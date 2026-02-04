#ifndef SHAWNCOIN_WALLET_WALLET_HPP
#define SHAWNCOIN_WALLET_WALLET_HPP

#include "../core/types.hpp"
#include "../core/transaction.hpp"
#include "hdwallet.hpp"
#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace shawncoin {

class UTXOSet;

class Wallet {
public:
    Wallet();
    bool create(const std::string& passphrase = "");
    bool restore(const std::string& mnemonic, const std::string& passphrase = "");
    std::string generateNewAddress();
    // Sign a transaction's inputs using this wallet's keys. Requires UTXO set to find previous outputs.
    bool signTransaction(Transaction& tx, const UTXOSet* utxo);
    uint64_t getBalance(const UTXOSet* utxo) const;
    void backup(const std::string& path) const;
    std::string getMnemonic() const { return mnemonic_; }
    // Helpers for RPC and callers to query derived addresses without exposing internals
    size_t getAddressCount() const;
    std::string deriveAddressAt(uint32_t index) const;
    bool hasHash160(const std::vector<uint8_t>& h) const;
    // Persistent wallet storage (encrypted mnemonic)
    bool saveToFile(const std::string& path, const std::string& passphrase) const;
    bool loadFromFile(const std::string& path, const std::string& passphrase);

private:
    std::string mnemonic_;
    HDWallet hd_;
    std::map<std::string, std::vector<uint8_t>> keys_;
    mutable std::mutex mutex_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_WALLET_WALLET_HPP
