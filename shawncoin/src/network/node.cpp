#include "network/node.hpp"
#include "core/block.hpp"
#include <algorithm>
#include <thread>

namespace shawncoin {

Node::Node(Blockchain& chain, Mempool& mempool) : chain_(&chain), mempool_(&mempool) {}

Node::~Node() { stop(); }

void Node::start(uint16_t port) {
    if (running_.exchange(true)) return;
    listenPort_ = port;
    // Stub: no actual Boost.Asio listener; would start acceptor here
}

void Node::stop() {
    running_.store(false);
}

void Node::connectToPeer(const std::string& address, uint16_t port) {
    Peer p;
    p.addr.host = address;
    p.addr.port = port;
    p.connected = false;
    p.inbound = false;
    peers_.push_back(p);
}

void Node::broadcastBlock(const Block& block) {
    (void)block;
    // Stub: would send to all connected peers
}

void Node::broadcastTransaction(const Transaction& tx) {
    (void)tx;
    // Stub: would send to all connected peers
}

std::vector<Peer> Node::getPeers() const {
    return peers_;
}

} // namespace shawncoin
