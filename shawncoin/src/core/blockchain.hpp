#ifndef SHAWNCOIN_CORE_BLOCKCHAIN_HPP
#define SHAWNCOIN_CORE_BLOCKCHAIN_HPP

#include "core/types.hpp"
#include "core/block.hpp"
#include "core/utxo.hpp"
#include "core/consensus.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <cstdint>
#include <functional>

namespace shawncoin {

class ChainState;
struct BlockIndex;

/** In-memory blockchain with optional persistent storage. */
class Blockchain {
public:
    Blockchain();
    ~Blockchain();

    /** Initialize with genesis; load from storage if available. */
    bool init(const std::string& dataDir);

    /** Add block; returns true if accepted (best or side chain). */
    bool addBlock(const Block& block, uint64_t height);

    /** Get block by hash (from storage/cache). */
    std::optional<Block> getBlock(const uint256& hash) const;

    /** Get block by height. */
    std::optional<Block> getBlockByHeight(uint64_t height) const;

    /** Get best block hash and height. */
    uint256 getBestBlockHash() const;
    uint64_t getHeight() const;

    /** Get genesis block. */
    Block getGenesisBlock() const;

    /** UTXO set (after all blocks applied). */
    UTXOSet& utxo() { return utxo_; }
    const UTXOSet& utxo() const { return utxo_; }

    /** Validate and connect a block (consensus + UTXO). */
    bool connectBlock(const Block& block, uint64_t height);

    /** Chainstate / storage backend (optional). */
    void setChainState(ChainState* state) { chainState_ = state; }
    ChainState* getChainState() const { return chainState_; }

private:
    UTXOSet utxo_;
    mutable std::mutex mutex_;
    uint256 bestBlockHash_;
    uint64_t height_ = 0;
    std::map<uint256, Block> blockCache_;
    std::map<uint64_t, uint256> heightIndex_;
    ChainState* chainState_ = nullptr;
};

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_BLOCKCHAIN_HPP
