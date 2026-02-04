#include "core/blockchain.hpp"
#include "core/block.hpp"
#include "core/consensus.hpp"
#include "storage/chainstate.hpp"
#include <algorithm>
#include <cstring>

namespace shawncoin {

Blockchain::Blockchain() {
    Block genesis = makeGenesisBlock();
    bestBlockHash_ = genesis.getHash();
    height_ = 0;
    blockCache_[bestBlockHash_] = genesis;
    heightIndex_[0] = bestBlockHash_;
    connectBlockUTXO(genesis, utxo_);
}

Blockchain::~Blockchain() = default;

bool Blockchain::init(const std::string& dataDir) {
    (void)dataDir;
    if (chainState_) {
        // Try load from chainstate; if empty, we already have genesis from ctor
        uint256 loadedBest;
        uint64_t loadedHeight = 0;
        if (chainState_->getBestBlock(loadedBest, loadedHeight) && loadedHeight > 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            bestBlockHash_ = loadedBest;
            height_ = loadedHeight;
            // UTXO and block cache would be loaded from DB in full impl
        }
    }
    return true;
}

bool Blockchain::connectBlock(const Block& block, uint64_t height) {
    if (!validateBlockStructure(block)) return false;
    // Enforce expected difficulty relative to chain
    if (!checkDifficulty(*this, block.header)) return false;
    for (size_t i = 1; i < block.transactions.size(); ++i) {
        if (!validateTransactionStructure(block.transactions[i])) return false;
        for (const auto& in : block.transactions[i].inputs) {
            OutPoint op{ in.prev_tx_hash, in.output_index };
            if (!utxo_.has(op)) return false; // double-spend or missing
        }
    }
    if (!connectBlockUTXO(block, utxo_)) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    uint256 hash = block.getHash();
    blockCache_[hash] = block;
    heightIndex_[height] = hash;
    bestBlockHash_ = hash;
    height_ = height;
    
    // Save to persistent storage
    if (chainState_) {
        chainState_->putBlock(hash, block);
        chainState_->setBestBlock(hash, height);
        
        // Save blockchain state to disk every 10 blocks
        if (height % 10 == 0) {
            auto* memoryState = dynamic_cast<MemoryChainState*>(chainState_);
            if (memoryState) {
                memoryState->saveToDisk();
            }
        }
    }
    
    return true;
}

bool Blockchain::addBlock(const Block& block, uint64_t height) {
    uint256 hash = block.getHash();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (blockCache_.count(hash)) return true; // already have it
        if (height != height_ + 1) return false;   // must be next block
        if (block.header.previous_hash != bestBlockHash_) return false; // wrong prev
    }
    return connectBlock(block, height);
}

std::optional<Block> Blockchain::getBlock(const uint256& hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = blockCache_.find(hash);
    if (it != blockCache_.end()) return it->second;
    if (chainState_) {
        Block b;
        if (chainState_->getBlock(hash, b)) return b;
    }
    return std::nullopt;
}

std::optional<Block> Blockchain::getBlockByHeight(uint64_t height) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = heightIndex_.find(height);
    if (it == heightIndex_.end()) return std::nullopt;
    auto j = blockCache_.find(it->second);
    if (j != blockCache_.end()) return j->second;
    return std::nullopt;
}

uint256 Blockchain::getBestBlockHash() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bestBlockHash_;
}

uint64_t Blockchain::getHeight() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return height_;
}

Block Blockchain::getGenesisBlock() const {
    return makeGenesisBlock();
}

} // namespace shawncoin
