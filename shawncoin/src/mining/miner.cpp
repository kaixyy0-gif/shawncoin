#include "mining/miner.hpp"
#include "core/block.hpp"
#include "core/blockchain.hpp"
#include "core/transaction.hpp"
#include "core/types.hpp"
#include "util/logger.hpp"
#include "util/realtime.hpp"
#include "util/util.hpp"
#include "crypto/address.hpp"
#include "crypto/hash.h"
#include "mining/merkle.hpp"
#include "mining/difficulty.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <cstring>
#include <iostream>

namespace shawncoin {

Miner::Miner(Blockchain& chain, Mempool& mempool) : chain_(&chain), mempool_(&mempool) {
    // Initialize with empty payout hash (will be 20 zero bytes)
    payoutHash_.resize(20, 0);
}

Miner::~Miner() { stop(); }

void Miner::stop() {
    mining_.store(false);
    for (auto& t : threads_)
        if (t.joinable()) t.join();
    threads_.clear();
}

void Miner::setPayoutHash(const std::vector<uint8_t>& hash) {
    if (hash.size() == 20) payoutHash_ = hash;
}

void Miner::setPayoutAddress(const std::string& address) {
    std::vector<uint8_t> h = shawncoin::addressToPubKeyHash(address);
    if (h.size() == 20) payoutHash_ = h;
}

void Miner::start(uint32_t threadCount) {
    if (mining_.exchange(true)) return;
    threadCount_ = threadCount ? threadCount : 1;
    for (uint32_t i = 0; i < threadCount_; ++i)
        threads_.emplace_back(&Miner::miningLoop, this);
}

bool Miner::mineBlock(Block& block, uint32_t difficultyTarget) {
    block.header.difficulty_target = difficultyTarget;
    block.header.merkle_root = computeMerkleRoot(block.transactions);
    uint256 target{};
    uint32_t compact = difficultyTarget;
    int nSize = (int)(compact >> 24);
    uint32_t nWord = compact & 0x007fffff;
    if (nSize <= 3) {
        nWord >>= 8 * (3 - nSize);
        target[31] = (nWord >> 0) & 0xff;
        target[30] = (nWord >> 8) & 0xff;
        target[29] = (nWord >> 16) & 0xff;
    } else {
        int shift = 8 * (nSize - 3);
        int bytePos = 32 - (shift / 8) - 1;
        if (bytePos >= 0) {
            target[bytePos] = (nWord >> 0) & 0xff;
            if (bytePos > 0) target[bytePos - 1] = (nWord >> 8) & 0xff;
            if (bytePos > 1) target[bytePos - 2] = (nWord >> 16) & 0xff;
        }
    }
    uint32_t nonce = 0;
    do {
        block.header.nonce = nonce;
        uint256 h = block.getHeaderHash();
        bool ok = true;
        for (int i = 31; i >= 0 && ok; --i) {
            if (h[i] > target[i]) ok = false;
            if (h[i] < target[i]) break;
        }
        if (ok) return true;
        ++nonce;
    } while (nonce != 0);  // try all 4B nonces
    return false;
}

void Miner::miningLoop() {
    uint64_t hashes = 0;
    auto start = std::chrono::steady_clock::now();
    while (mining_.load()) {
        Block block;
        block.header.version = 1;
        block.header.previous_hash = chain_->getBestBlockHash();
        block.header.timestamp = (uint64_t)std::time(nullptr);
        block.header.difficulty_target = EASY_MINE_DIFFICULTY; // CPU-friendly when using --mine
        Transaction coinbase;
        coinbase.version = 1;
        coinbase.inputs.resize(1);
        coinbase.inputs[0].prev_tx_hash = {};
        coinbase.inputs[0].output_index = 0xffffffffu;
        // Include a small coinbase script (scriptSig) with the block height
        uint64_t height = chain_->getHeight() + 1;
        std::vector<uint8_t> cbscript;
        // simple varint encoding for small heights
        if (height < 0xfd) {
            cbscript.push_back((uint8_t)height);
        } else if (height <= 0xffff) {
            cbscript.push_back(0xfd);
            cbscript.push_back((uint8_t)(height & 0xff));
            cbscript.push_back((uint8_t)((height >> 8) & 0xff));
        } else {
            // larger heights are unlikely in tests; write 4-byte little-endian
            cbscript.push_back(0xfe);
            cbscript.push_back((uint8_t)(height & 0xff));
            cbscript.push_back((uint8_t)((height >> 8) & 0xff));
            cbscript.push_back((uint8_t)((height >> 16) & 0xff));
            cbscript.push_back((uint8_t)((height >> 24) & 0xff));
        }
        coinbase.inputs[0].signature = std::move(cbscript);
        coinbase.outputs.resize(1);
        coinbase.outputs[0].amount = getBlockSubsidy(chain_->getHeight() + 1);
        coinbase.outputs[0].script_pubkey = { 0x76, 0xa9, 0x14 };
            // Insert the 20-byte hash160 to receive the coinbase reward. If a payout
            // hash was configured, use it; otherwise use the placeholder (zeros).
            if (payoutHash_.size() == 20) {
                coinbase.outputs[0].script_pubkey.insert(coinbase.outputs[0].script_pubkey.end(), payoutHash_.begin(), payoutHash_.end());
            } else {
                coinbase.outputs[0].script_pubkey.insert(coinbase.outputs[0].script_pubkey.end(), 20, 0);
            }
        coinbase.outputs[0].script_pubkey.push_back(0x88);
        coinbase.outputs[0].script_pubkey.push_back(0xac);
        block.transactions.push_back(coinbase);
        for (const auto& tx : mempool_->getBlockTemplate())
            block.transactions.push_back(tx);
        uint32_t target = EASY_MINE_DIFFICULTY;
        if (mineBlock(block, target)) {
            uint64_t newHeight = chain_->getHeight() + 1;
            bool ok = chain_->addBlock(block, newHeight);
            if (ok) {
                uint64_t totalIssued = getTotalSupplyUpTo(newHeight);
                SHAWNCOIN_LOG(Info, "miner", "Mined block %llu (hash=%s) reward=%llu SHWN total=%llu",
                    (unsigned long long)newHeight,
                    uint256ToHex(block.getHash()).c_str(),
                    (unsigned long long)(block.transactions[0].outputs[0].amount / COIN),
                    (unsigned long long)(totalIssued / COIN));
                std::cout << "[MINED] Block " << newHeight << " | Reward: " << (block.transactions[0].outputs[0].amount / COIN) << " SHWN | Total issued: " << (totalIssued / COIN) << " SHWN" << std::endl;
                // Append a realtime JSON event (single-line) for tailing
                try {
                    std::string evt = "{\"type\":\"block\",\"height\":" + std::to_string(newHeight)
                        + ",\"hash\":\"" + uint256ToHex(block.getHash()) + "\""
                        + ",\"reward\":" + std::to_string(block.transactions[0].outputs[0].amount)
                        + ",\"txcount\":" + std::to_string(block.transactions.size())
                        + ",\"difficulty\":\"" + difficultyToString(block.header.difficulty_target) + "\""
                        + ",\"hashrate\":" + std::to_string(hashrate_.load()) + "}";
                    shawncoin::appendRealtimeEvent(evt);
                    
                    // Log difficulty adjustment info at retarget points
                    if (newHeight > 0 && newHeight % DIFFICULTY_INTERVAL == 0) {
                        double networkHashRate = calculateHashRate(block.header.difficulty_target);
                        std::string diffInfo = "{\"type\":\"difficulty_adjustment\",\"height\":" + std::to_string(newHeight)
                            + ",\"difficulty\":\"" + difficultyToString(block.header.difficulty_target) + "\""
                            + ",\"network_hashrate\":" + std::to_string(networkHashRate) + "}";
                        shawncoin::appendRealtimeEvent(diffInfo);
                    }
                } catch (...) { /* ignore errors */ }
            }
            hashrate_.store(hashes);
        }
        hashes++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= 1) {
            hashrate_.store(hashes);
            start = now;
            hashes = 0;
        }
    }
}

} // namespace shawncoin
