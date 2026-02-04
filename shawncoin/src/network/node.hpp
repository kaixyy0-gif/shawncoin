#ifndef SHAWNCOIN_NETWORK_NODE_HPP
#define SHAWNCOIN_NETWORK_NODE_HPP

#include "core/blockchain.hpp"
#include "core/block.hpp"
#include "core/mempool.hpp"
#include "core/types.hpp"
#include "network/peer.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <atomic>

namespace shawncoin {

class Node {
public:
    Node(Blockchain& chain, Mempool& mempool);
    ~Node();

    void start(uint16_t port = P2P_PORT);
    void stop();
    void connectToPeer(const std::string& address, uint16_t port);
    void broadcastBlock(const Block& block);
    void broadcastTransaction(const Transaction& tx);
    std::vector<Peer> getPeers() const;
    bool isRunning() const { return running_.load(); }

private:
    Blockchain* chain_ = nullptr;
    Mempool* mempool_ = nullptr;
    std::atomic<bool> running_{false};
    std::vector<Peer> peers_;
    uint16_t listenPort_ = P2P_PORT;
};

} // namespace shawncoin

#endif // SHAWNCOIN_NETWORK_NODE_HPP
