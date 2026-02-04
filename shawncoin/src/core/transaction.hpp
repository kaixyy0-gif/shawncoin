#ifndef SHAWNCOIN_CORE_TRANSACTION_HPP
#define SHAWNCOIN_CORE_TRANSACTION_HPP

#include "core/types.hpp"
#include <vector>
#include <cstddef>

namespace shawncoin {

/** Append serialized transaction to buffer */
void serializeTransaction(const Transaction& tx, std::vector<uint8_t>& out);

/** Deserialize transaction from data at pos; pos is updated. Returns false on error. */
bool deserializeTransaction(const uint8_t* data, size_t len, size_t& pos, Transaction& tx);

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_TRANSACTION_HPP
