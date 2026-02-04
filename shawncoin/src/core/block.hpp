#ifndef SHAWNCOIN_CORE_BLOCK_HPP
#define SHAWNCOIN_CORE_BLOCK_HPP

#include "core/types.hpp"
#include <vector>
#include <cstdint>

namespace shawncoin {

struct Block;
struct BlockHeader;

/** Serialize block to bytes (for storage/network) */
std::vector<uint8_t> serializeBlock(const Block& block);

/** Deserialize block from bytes; returns false on error */
bool deserializeBlock(const uint8_t* data, size_t len, Block& block);

/** Build genesis block for Shawn Coin */
Block makeGenesisBlock();

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_BLOCK_HPP
