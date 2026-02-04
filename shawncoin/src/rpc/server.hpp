#ifndef SHAWNCOIN_RPC_SERVER_HPP
#define SHAWNCOIN_RPC_SERVER_HPP

#include "api.hpp"
#include <string>
#include <cstdint>
#include <atomic>
#include <memory>
#include <thread>

namespace shawncoin {

class RpcServer {
public:
    RpcServer(RpcContext* ctx);
    bool start(uint16_t port = RPC_PORT);
    void stop();
    bool isRunning() const { return running_.load(); }

private:
    RpcContext* ctx_ = nullptr;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> server_thread_;
};

} // namespace shawncoin

#endif // SHAWNCOIN_RPC_SERVER_HPP
