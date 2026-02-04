#ifndef SHAWNCOIN_MINING_DIFFICULTY_HPP
#define SHAWNCOIN_MINING_DIFFICULTY_HPP

#include "../core/types.hpp"
#include <cstdint>

namespace shawncoin {

/** Get next difficulty target from last 2016 blocks. */
uint32_t getNextDifficulty(uint32_t currentTarget, uint64_t lastBlockTime, uint64_t firstBlockTime, uint32_t numBlocks = DIFFICULTY_INTERVAL);

/** Enhanced adaptive difficulty adjustment algorithm. */
uint32_t getAdaptiveNextDifficulty(uint32_t currentTarget, uint64_t lastBlockTime, uint64_t firstBlockTime, uint32_t numBlocks, uint64_t currentHeight);

/** Calculate network hash rate from difficulty. */
double calculateHashRate(uint32_t difficulty);

/** Estimate time to find next block at current hashrate. */
uint64_t estimateBlockTime(uint32_t difficulty, double networkHashRate);

/** Validate difficulty is within acceptable bounds. */
bool isValidDifficulty(uint32_t difficulty);

/** Convert difficulty to human-readable format. */
std::string difficultyToString(uint32_t difficulty);

/** Initial difficulty (genesis). */
constexpr uint32_t INITIAL_DIFFICULTY = 0x1d00ffff;

/** Easy difficulty for CPU mining (--mine): ~65k hashes per block, findable in under a second. */
constexpr uint32_t EASY_MINE_DIFFICULTY = 0x0400ffff;

/** Maximum allowed difficulty increase per retarget (4x). */
constexpr uint32_t MAX_DIFFICULTY_INCREASE = 4;

/** Maximum allowed difficulty decrease per retarget (1/4x). */
constexpr uint32_t MAX_DIFFICULTY_DECREASE = 4;

/** Emergency retarget interval for fast networks. */
constexpr uint32_t EMERGENCY_RETARGET_INTERVAL = 144; // ~1 day at 10min blocks

} // namespace shawncoin

#endif // SHAWNCOIN_MINING_DIFFICULTY_HPP
