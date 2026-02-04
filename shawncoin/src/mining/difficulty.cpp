#include "mining/difficulty.hpp"
#include "../core/types.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace shawncoin {

uint32_t getNextDifficulty(uint32_t currentTarget, uint64_t lastBlockTime, uint64_t firstBlockTime, uint32_t numBlocks) {
    if (numBlocks == 0 || firstBlockTime >= lastBlockTime) return currentTarget;
    uint64_t elapsed = lastBlockTime - firstBlockTime;
    uint64_t targetTime = (uint64_t)BLOCK_TIME_TARGET * (numBlocks - 1);
    if (elapsed < targetTime / 4) elapsed = targetTime / 4;
    if (elapsed > targetTime * 4) elapsed = targetTime * 4;
    uint64_t targetMul = (uint64_t)currentTarget * elapsed;
    uint64_t newTarget = targetMul / targetTime;
    if (newTarget > 0x007fffffUL) newTarget = 0x007fffffUL;
    return (uint32_t)((currentTarget >> 24) << 24) | (uint32_t)newTarget;
}

uint32_t getAdaptiveNextDifficulty(uint32_t currentTarget, uint64_t lastBlockTime, uint64_t firstBlockTime, uint32_t numBlocks, uint64_t currentHeight) {
    // Basic difficulty calculation
    uint32_t newTarget = getNextDifficulty(currentTarget, lastBlockTime, firstBlockTime, numBlocks);
    
    // Apply emergency retargeting for early blocks or extreme conditions
    uint64_t elapsed = lastBlockTime - firstBlockTime;
    uint64_t targetTime = (uint64_t)BLOCK_TIME_TARGET * (numBlocks - 1);
    
    // Emergency adjustment if blocks are coming way too fast or slow
    if (elapsed < targetTime / 8) {
        // Blocks are 8x faster than target - emergency increase
        uint32_t emergencyTarget = newTarget * 4; // MAX_DIFFICULTY_INCREASE
        if (emergencyTarget > newTarget) {
            newTarget = emergencyTarget;
        }
    } else if (elapsed > targetTime * 8) {
        // Blocks are 8x slower than target - emergency decrease
        uint32_t emergencyTarget = newTarget / 4; // MAX_DIFFICULTY_DECREASE
        if (emergencyTarget < newTarget) {
            newTarget = emergencyTarget;
        }
    }
    
    // For early network growth, use more aggressive adjustment
    if (currentHeight < 10000) {
        // Early phase: allow faster adjustment (up to 2x per retarget)
        if (elapsed < targetTime / 2) {
            newTarget = newTarget * 2;
        } else if (elapsed > targetTime * 2) {
            newTarget = newTarget / 2;
        }
    }
    
    // Ensure difficulty stays within bounds
    return newTarget;
}

double calculateHashRate(uint32_t difficulty) {
    // Extract the mantissa from the compact representation
    uint32_t mantissa = difficulty & 0x00ffffff;
    uint32_t exponent = (difficulty >> 24) & 0xff;
    
    if (exponent <= 3) {
        mantissa >>= (3 - exponent);
        exponent = 3;
    }
    
    // Calculate the actual target value
    double target = mantissa * pow(256.0, exponent - 3);
    
    // Hash rate = max_hash / target * seconds_per_block
    // max_hash for SHA-256 is 2^256
    double maxHash = pow(2.0, 256.0);
    return maxHash / target / BLOCK_TIME_TARGET;
}

uint64_t estimateBlockTime(uint32_t difficulty, double networkHashRate) {
    double targetHashes = calculateHashRate(difficulty) * BLOCK_TIME_TARGET;
    if (networkHashRate <= 0) return BLOCK_TIME_TARGET * 1000; // Default fallback
    
    double expectedSeconds = targetHashes / networkHashRate;
    return static_cast<uint64_t>(expectedSeconds);
}

bool isValidDifficulty(uint32_t difficulty) {
    // Extract components
    uint32_t mantissa = difficulty & 0x00ffffff;
    uint32_t exponent = (difficulty >> 24) & 0xff;
    
    // Check bounds: exponent should be reasonable, mantissa positive
    if (exponent == 0 || exponent > 50) return false;
    if (mantissa == 0) return false;
    
    // Check maximum difficulty limit
    if (difficulty < EASY_MINE_DIFFICULTY) return false;
    
    return true;
}

std::string difficultyToString(uint32_t difficulty) {
    std::ostringstream oss;
    
    // Extract mantissa and exponent
    uint32_t mantissa = difficulty & 0x00ffffff;
    uint32_t exponent = (difficulty >> 24) & 0xff;
    
    oss << "0x" << std::hex << std::uppercase << difficulty << std::dec;
    oss << " (exp=" << exponent << ", mantissa=" << mantissa << ")";
    
    // Calculate network hash rate
    double hashRate = calculateHashRate(difficulty);
    
    if (hashRate < 1000) {
        oss << " (~" << std::fixed << std::setprecision(2) << hashRate << " H/s)";
    } else if (hashRate < 1000000) {
        oss << " (~" << std::fixed << std::setprecision(2) << (hashRate / 1000) << " KH/s)";
    } else if (hashRate < 1000000000) {
        oss << " (~" << std::fixed << std::setprecision(2) << (hashRate / 1000000) << " MH/s)";
    } else {
        oss << " (~" << std::fixed << std::setprecision(2) << (hashRate / 1000000000) << " GH/s)";
    }
    
    return oss.str();
}

} // namespace shawncoin
