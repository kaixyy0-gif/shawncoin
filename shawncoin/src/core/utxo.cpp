#include "core/utxo.hpp"
#include <algorithm>

namespace shawncoin {

void UTXOSet::put(const OutPoint& out, uint64_t amount, Script script_pubkey) {
    std::lock_guard<std::mutex> lock(mutex_);
    map_[out] = { amount, std::move(script_pubkey) };
}

std::optional<UTXOSet::Entry> UTXOSet::get(const OutPoint& out) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(out);
    if (it == map_.end()) return std::nullopt;
    return it->second;
}

bool UTXOSet::spend(const OutPoint& out) {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_.erase(out) > 0;
}

bool UTXOSet::has(const OutPoint& out) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_.find(out) != map_.end();
}

UTXOSet::Map UTXOSet::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_;
}

size_t UTXOSet::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_.size();
}

void UTXOSet::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    map_.clear();
}

} // namespace shawncoin
