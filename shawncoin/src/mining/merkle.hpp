#ifndef SHAWNCOIN_MINING_MERKLE_HPP
#define SHAWNCOIN_MINING_MERKLE_HPP

#include "../core/types.hpp"
#include <vector>

namespace shawncoin {

/** Compute Merkle root from transaction list (SHA256d of txids, then hash pairs). */
uint256 computeMerkleRoot(const std::vector<Transaction>& transactions);

/** Compute Merkle root from a list of 32-byte hashes. */
uint256 computeMerkleRootFromHashes(const std::vector<uint256>& hashes);

} // namespace shawncoin

#endif // SHAWNCOIN_MINING_MERKLE_HPP
