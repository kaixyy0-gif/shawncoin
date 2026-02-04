#ifndef SHAWNCOIN_MINING_STRATUM_HPP
#define SHAWNCOIN_MINING_STRATUM_HPP

#include <thread>
#include <atomic>
#include <string>

namespace shawncoin {

class StratumServer {
public:
    StratumServer(uint16_t port = 3333);
    ~StratumServer();
    bool start();
    void stop();
private:
    void run();
    uint16_t port_;
    std::thread thread_;
    std::atomic<bool> running_{false};
};

} // namespace shawncoin

#endif // SHAWNCOIN_MINING_STRATUM_HPP
