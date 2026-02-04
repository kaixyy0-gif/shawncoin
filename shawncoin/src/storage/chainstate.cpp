#include "chainstate.hpp"
#include "core/block.hpp"
#include "core/transaction.hpp"
#include <map>
#include <mutex>
#include <cstring>
#include <fstream>

namespace shawncoin {

bool MemoryChainState::init(const std::string& path) {
    path_ = path;
    return true;
}

void MemoryChainState::shutdown() {}

bool MemoryChainState::getBestBlock(uint256& hash, uint64_t& height) const {
    std::lock_guard<std::mutex> lock(mutex_);
    hash = bestHash_;
    height = bestHeight_;
    return true;
}

void MemoryChainState::setBestBlock(const uint256& hash, uint64_t height) {
    std::lock_guard<std::mutex> lock(mutex_);
    bestHash_ = hash;
    bestHeight_ = height;
}

bool MemoryChainState::getBlock(const uint256& hash, Block& block) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = blocks_.find(hash);
    if (it == blocks_.end()) return false;
    return deserializeBlock(it->second.data(), it->second.size(), block);
}

bool MemoryChainState::putBlock(const uint256& hash, const Block& block) {
    std::vector<uint8_t> data = serializeBlock(block);
    std::lock_guard<std::mutex> lock(mutex_);
    blocks_[hash] = std::move(data);
    return true;
}

// Enhanced persistence methods
bool MemoryChainState::saveToDisk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    FILE* file = fopen((path_ + "/blockchain.dat").c_str(), "wb");
    if (!file) return false;
    
    // Write header
    fwrite("SHWN", 4, 1, file);
    uint32_t version = 1;
    fwrite(&version, sizeof(version), 1, file);
    uint64_t blockCount = blocks_.size();
    fwrite(&blockCount, sizeof(blockCount), 1, file);
    
    // Write best block info
    fwrite(bestHash_.data(), 32, 1, file);
    fwrite(&bestHeight_, sizeof(bestHeight_), 1, file);
    
    // Write all blocks
    for (const auto& [hash, data] : blocks_) {
        uint64_t dataSize = data.size();
        fwrite(&dataSize, sizeof(dataSize), 1, file);
        fwrite(data.data(), dataSize, 1, file);
    }
    
    fclose(file);
    return true;
}

bool MemoryChainState::loadFromDisk() {
    std::lock_guard<std::mutex> lock(mutex_);
    FILE* file = fopen((path_ + "/blockchain.dat").c_str(), "rb");
    if (!file) return false;
    
    // Read and verify header
    char header[4];
    if (fread(header, 4, 1, file) != 1 || memcmp(header, "SHWN", 4) != 0) {
        fclose(file);
        return false;
    }
    
    uint32_t version;
    if (fread(&version, sizeof(version), 1, file) != 1 || version != 1) {
        fclose(file);
        return false;
    }
    
    uint64_t blockCount;
    if (fread(&blockCount, sizeof(blockCount), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Read best block info
    if (fread(bestHash_.data(), 32, 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    if (fread(&bestHeight_, sizeof(bestHeight_), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Read all blocks
    blocks_.clear();
    for (uint64_t i = 0; i < blockCount; ++i) {
        uint64_t dataSize;
        if (fread(&dataSize, sizeof(dataSize), 1, file) != 1) {
            fclose(file);
            return false;
        }
        
        std::vector<uint8_t> data(dataSize);
        if (fread(data.data(), dataSize, 1, file) != 1) {
            fclose(file);
            return false;
        }
        
        uint256 hash;
        // TODO: Calculate hash from block data
        blocks_[hash] = std::move(data);
    }
    
    fclose(file);
    return true;
}

} // namespace shawncoin
