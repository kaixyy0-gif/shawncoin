#include "rpc/server.hpp"
#include "core/types.hpp"
#include "rpc/api.hpp"
#include <thread>
#include <chrono>

#if defined(HAVE_RESTINIO)
#include <restinio/all.hpp>
#endif

namespace shawncoin {

RpcServer::RpcServer(RpcContext* ctx) : ctx_(ctx) {}

// Simple base64 decode for HTTP Basic auth (username:password)
static std::string base64_decode(const std::string& in) {
    static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[(unsigned char)b[i]] = i;
    int val=0, valb=-8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}

#if defined(HAVE_RESTINIO)

bool RpcServer::start(uint16_t port) {
    if (!ctx_) return false;
    if (running_.exchange(true)) return true;

    // start RESTinio server in background thread using restinio::run
    server_thread_ = std::make_unique<std::thread>([this, port]() {
        try {
            using namespace restinio;
            run(on_thread_pool(1)
                .address("0.0.0.0")
                .port(port)
                .request_handler([this](auto req) {
                    // Basic auth check if configured
                    if (!ctx_->rpcUser.empty() || !ctx_->rpcPassword.empty()) {
                        auto auth_sv = req->header().value_of("authorization");
                        if (auth_sv.empty() || auth_sv.substr(0,6) != "Basic ") {
                            return req->create_response(restinio::status_unauthorized())
                                .append_header("WWW-Authenticate", "Basic realm=\"ShawnCoin RPC\"")
                                .set_body("Unauthorized")
                                .done();
                        }
                        std::string encoded = std::string(auth_sv.substr(6));
                        std::string decoded = base64_decode(encoded);
                        std::string expected = ctx_->rpcUser + ":" + ctx_->rpcPassword;
                        if (decoded != expected) {
                            return req->create_response(restinio::status_forbidden()).set_body("Forbidden").done();
                        }
                    }

                    // Expect POST with JSON-RPC body
                    if (req->header().method() != http_method_post()) {
                        return req->create_response(restinio::status_method_not_allowed()).set_body("Method Not Allowed").done();
                    }
                    auto body = req->body();
                    std::string response = handleRpcRequest(body, ctx_);
                    return req->create_response()
                        .append_header("Content-Type", "application/json")
                        .set_body(response)
                        .done();
                }));
        } catch (const std::exception& e) {
            (void)e;
        }
    });
    return true;
}

void RpcServer::stop() {
    if (!running_.exchange(false)) return;
    // We cannot gracefully stop restinio server without storing server handle; join thread
    if (server_thread_ && server_thread_->joinable()) server_thread_->join();
}

#else

bool RpcServer::start(uint16_t port) {
    (void)port;
    running_.store(true);
    return true; // fallback stub
}

void RpcServer::stop() {
    running_.store(false);
}

#endif

} // namespace shawncoin
