#ifndef SHAWNCOIN_STORAGE_CHAINSTATE_HPP
#define SHAWNCOIN_STORAGE_CHAINSTATE_HPP

#include "../core/types.hpp"
#include "../core/block.hpp"
#include <string>
#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

namespace shawncoin {

/** Abstract chain state / storage (RocksDB or in-memory). */
class ChainState {
public:
    virtual ~ChainState() = default;
    virtual bool init(const std::string& path) = 0;
    virtual void shutdown() = 0;
    virtual bool getBestBlock(uint256& hash, uint64_t& height) const = 0;
    virtual void setBestBlock(const uint256& hash, uint64_t height) = 0;
    virtual bool getBlock(const uint256& hash, Block& block) const = 0;
    virtual bool putBlock(const uint256& hash, const Block& block) = 0;
};

/** In-memory chain state (no persistence). */
class MemoryChainState : public ChainState {
public:
    bool init(const std::string& path) override;
    void shutdown() override;
    bool getBestBlock(uint256& hash, uint64_t& height) const override;
    void setBestBlock(const uint256& hash, uint64_t height) override;
    bool getBlock(const uint256& hash, Block& block) const override;
    bool putBlock(const uint256& hash, const Block& block) override;
    bool saveToDisk() const;
    bool loadFromDisk();
private:
    std::string path_;
    uint256 bestHash_;
    uint64_t bestHeight_ = 0;
    std::map<uint256, std::vector<uint8_t>> blocks_;
    mutable std::mutex mutex_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_STORAGE_CHAINSTATE_HPP
