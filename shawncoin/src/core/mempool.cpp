#include "core/mempool.hpp"
#include "core/consensus.hpp"
#include "util/util.hpp"
#include "util/realtime.hpp"
#include <algorithm>
#include <vector>

namespace shawncoin {

bool Mempool::add(const Transaction& tx, uint64_t fee) {
    if (!validateTransactionStructure(tx)) return false;
    uint256 txid = tx.getTxid();
    std::lock_guard<std::mutex> lock(mutex_);
    if (txs_.size() >= MAX_MEMPOOL_SIZE && txs_.find(txid) == txs_.end())
        return false; // evict lowest fee would go here
    txs_[txid] = tx;
    (void)fee;
    // emit realtime event for new transaction (append to realtime feed)
    try {
        std::string id = shawncoin::uint256ToHex(txid);
        uint64_t value = tx.getTotalOutput();
        std::string evt = "{\"type\":\"tx\",\"txid\":\"" + id + "\",\"value\":" + std::to_string(value) + "}";
        shawncoin::appendRealtimeEvent(evt);
    } catch (...) { }
    return true;
}

bool Mempool::remove(const uint256& txid) {
    std::lock_guard<std::mutex> lock(mutex_);
    return txs_.erase(txid) > 0;
}

std::optional<Transaction> Mempool::get(const uint256& txid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = txs_.find(txid);
    if (it == txs_.end()) return std::nullopt;
    return it->second;
}

std::vector<Transaction> Mempool::getBlockTemplate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> out;
    for (const auto& p : txs_)
        out.push_back(p.second);
    return out;
}

size_t Mempool::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return txs_.size();
}

void Mempool::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    txs_.clear();
}

} // namespace shawncoin
