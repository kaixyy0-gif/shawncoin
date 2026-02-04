#ifndef SHAWNCOIN_MINING_MINER_HPP
#define SHAWNCOIN_MINING_MINER_HPP

#include "../core/types.hpp"
#include "../core/block.hpp"
#include "../core/blockchain.hpp"
#include "../core/mempool.hpp"
#include "difficulty.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

namespace shawncoin {

class Miner {
public:
    Miner(Blockchain& chain, Mempool& mempool);
    ~Miner();

    void start(uint32_t threadCount = 1);
    void stop();
    bool isMining() const { return mining_.load(); }
    uint64_t getHashrate() const { return hashrate_.load(); }
    bool mineBlock(Block& block, uint32_t difficultyTarget);
    // Set the payout script's pubkey-hash (20 bytes). If empty, miner uses the default
    // placeholder (20 zero bytes).
    void setPayoutHash(const std::vector<uint8_t>& hash);
    // Convenience: set payout address using Base58 address string (decodes and stores hash160)
    void setPayoutAddress(const std::string& address);

private:
    void miningLoop();

    Blockchain* chain_ = nullptr;
    Mempool* mempool_ = nullptr;
    std::atomic<bool> mining_{false};
    std::atomic<uint64_t> hashrate_{0};
    uint32_t threadCount_ = 1;
    std::vector<std::thread> threads_;
    std::vector<uint8_t> payoutHash_; // 20-byte hash160 for coinbase output
};

} // namespace shawncoin

#endif // SHAWNCOIN_MINING_MINER_HPP
