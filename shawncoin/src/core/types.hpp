#ifndef SHAWNCOIN_CORE_TYPES_HPP
#define SHAWNCOIN_CORE_TYPES_HPP

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

namespace shawncoin {

// SHA-256 hash: 32 bytes
using uint256 = std::array<uint8_t, 32>;

// Outpoint: reference to a UTXO
struct OutPoint {
    uint256 hash;
    uint32_t index = 0;
    bool operator==(const OutPoint& o) const;
    bool operator<(const OutPoint& o) const;
};

// Script (for P2PKH we use simple pubkey hash in script_pubkey)
using Script = std::vector<uint8_t>;

// Transaction output
struct TxOutput {
    uint64_t amount = 0;
    Script script_pubkey;
};

// Transaction input
struct TxInput {
    uint256 prev_tx_hash;
    uint32_t output_index = 0;
    std::vector<uint8_t> signature;
    std::vector<uint8_t> pubkey;
};

// Transaction
struct Transaction {
    uint32_t version = 1;
    std::vector<TxInput> inputs;
    std::vector<TxOutput> outputs;
    uint32_t lock_time = 0;
    mutable std::optional<uint256> cached_txid;

    uint256 getTxid() const;
    bool isCoinbase() const;
    uint64_t getTotalOutput() const;
    uint64_t getTotalInput() const; // requires UTXO lookup; 0 for coinbase
};

// Block header (80 bytes for hashing, minus variable tx list)
struct BlockHeader {
    uint32_t version = 1;
    uint256 previous_hash;
    uint256 merkle_root;
    uint64_t timestamp = 0;
    uint32_t difficulty_target = 0;
    uint32_t nonce = 0;
};

// Full block
struct Block {
    BlockHeader header;
    std::vector<Transaction> transactions;

    uint256 getHash() const;
    uint256 getHeaderHash() const;
};

// Coin parameters
constexpr uint64_t COIN = 100000000;           // 1 SHWN = 10^8 satoshis
constexpr uint64_t MAX_MONEY = 12000000 * COIN; // 12M SHWN
constexpr uint32_t BLOCK_TIME_TARGET = 600;     // 10 minutes
constexpr uint32_t DIFFICULTY_INTERVAL = 2016;
constexpr uint64_t SUBSIDY_HALVING_INTERVAL = 210000;
constexpr uint64_t INITIAL_BLOCK_SUBSIDY = 25 * COIN; // 25 SHWN
constexpr uint16_t P2P_PORT = 7333;
constexpr uint16_t RPC_PORT = 7332;
constexpr uint32_t PROTOCOL_MAGIC = 0x53484157; // "SHAW"

inline uint64_t getBlockSubsidy(uint64_t height) {
    uint64_t halvings = height / SUBSIDY_HALVING_INTERVAL;
    if (halvings >= 32) return 0;
    uint64_t subsidy = INITIAL_BLOCK_SUBSIDY >> halvings;
    return subsidy;
}

/** Total SHWN issued from block 0 up to and including height (for display). */
inline uint64_t getTotalSupplyUpTo(uint64_t height) {
    uint64_t total = 0;
    for (uint64_t h = 0; h <= height; ++h)
        total += getBlockSubsidy(h);
    return total;
}

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_TYPES_HPP
