#ifndef SHAWNCOIN_CORE_MEMPOOL_HPP
#define SHAWNCOIN_CORE_MEMPOOL_HPP

#include "core/types.hpp"
#include "transaction.hpp"
#include <vector>
#include <map>
#include <mutex>
#include <optional>
#include <cstddef>

namespace shawncoin {

/** Memory pool: unconfirmed transactions. Size limit and eviction by fee. */
class Mempool {
public:
    static constexpr size_t MAX_MEMPOOL_SIZE = 100000;
    static constexpr size_t MAX_TX_SIZE = 100000;

    bool add(const Transaction& tx, uint64_t fee);
    bool remove(const uint256& txid);
    std::optional<Transaction> get(const uint256& txid) const;
    std::vector<Transaction> getBlockTemplate() const;
    size_t size() const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::map<uint256, Transaction> txs_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_MEMPOOL_HPP
