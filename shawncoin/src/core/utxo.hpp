#ifndef SHAWNCOIN_CORE_UTXO_HPP
#define SHAWNCOIN_CORE_UTXO_HPP

#include "core/types.hpp"
#include <map>
#include <mutex>
#include <optional>

namespace shawncoin {

/** UTXO set: outpoint -> (amount, script_pubkey). Thread-safe. */
class UTXOSet {
public:
    struct Entry {
        uint64_t amount = 0;
        Script script_pubkey;
    };
    using Map = std::map<OutPoint, Entry>;

    void put(const OutPoint& out, uint64_t amount, Script script_pubkey);
    std::optional<Entry> get(const OutPoint& out) const;
    bool spend(const OutPoint& out);
    bool has(const OutPoint& out) const;
    Map snapshot() const;
    size_t size() const;
    void clear();

private:
    mutable std::mutex mutex_;
    Map map_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_UTXO_HPP
