#include "mining/stratum.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

namespace shawncoin {

StratumServer::StratumServer(uint16_t port) : port_(port) {}

StratumServer::~StratumServer() { stop(); }

bool StratumServer::start() {
    if (running_.exchange(true)) return false;
    thread_ = std::thread(&StratumServer::run, this);
    return true;
}

void StratumServer::stop() {
    if (!running_.exchange(false)) return;
    if (thread_.joinable()) thread_.join();
}

void StratumServer::run() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "Stratum: failed to create socket\n";
        running_.store(false);
        return;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Stratum: bind failed\n";
        close(listen_fd);
        running_.store(false);
        return;
    }
    if (listen(listen_fd, 4) < 0) {
        std::cerr << "Stratum: listen failed\n";
        close(listen_fd);
        running_.store(false);
        return;
    }
    std::cout << "Stratum: listening on port " << port_ << "\n";
    while (running_.load()) {
        struct sockaddr_in peer{};
        socklen_t plen = sizeof(peer);
        int fd = accept(listen_fd, (struct sockaddr*)&peer, &plen);
        if (fd < 0) continue;
        // Simple single-request handler: read up to 4096 bytes and echo a generic welcome
        char buf[4096];
        ssize_t r = read(fd, buf, sizeof(buf)-1);
        if (r > 0) {
            buf[r] = '\0';
            // We don't implement full Stratum yet — respond with a minimal JSON message
            const char *resp = "{\"id\": null, \"result\": [\"ShawnStratum\", null], \"error\": null}\n";
            write(fd, resp, strlen(resp));
        }
        close(fd);
    }
    close(listen_fd);
}

} // namespace shawncoin
