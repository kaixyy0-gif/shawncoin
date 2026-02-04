#include "mining/merkle.hpp"
#include "core/transaction.hpp"
#include "crypto/hash.h"
#include <cstring>
#include <algorithm>

namespace shawncoin {

uint256 computeMerkleRootFromHashes(const std::vector<uint256>& hashes) {
    if (hashes.empty()) return uint256{};
    std::vector<uint256> row = hashes;
    while (row.size() > 1) {
        std::vector<uint256> next;
        for (size_t i = 0; i < row.size(); i += 2) {
            unsigned char concat[64];
            memcpy(concat, row[i].data(), 32);
            if (i + 1 < row.size())
                memcpy(concat + 32, row[i + 1].data(), 32);
            else
                memcpy(concat + 32, row[i].data(), 32); // duplicate if odd
            uint256 h;
            shawncoin_sha256d(concat, 64, h.data());
            next.push_back(h);
        }
        row = std::move(next);
    }
    return row[0];
}

uint256 computeMerkleRoot(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) return uint256{};
    std::vector<uint256> hashes;
    hashes.reserve(transactions.size());
    for (const auto& tx : transactions)
        hashes.push_back(tx.getTxid());
    return computeMerkleRootFromHashes(hashes);
}

} // namespace shawncoin
