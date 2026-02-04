#ifndef SHAWNCOIN_RPC_API_HPP
#define SHAWNCOIN_RPC_API_HPP

#include "../core/blockchain.hpp"
#include "../core/mempool.hpp"
#include "../core/types.hpp"
#include <string>
#include <memory>

namespace shawncoin {

class Wallet;
class Miner;

struct RpcContext {
    Blockchain* chain = nullptr;
    Mempool* mempool = nullptr;
    Wallet* wallet = nullptr;
    Miner* miner = nullptr;
    // RPC credentials (optional)
    std::string rpcUser;
    std::string rpcPassword;
};

/** Handle JSON-RPC 2.0 request; returns JSON string response. */
std::string handleRpcRequest(const std::string& request, RpcContext* ctx);

/** REST: GET /api/v1/blockchain/info */
std::string apiBlockchainInfo(RpcContext* ctx);

/** REST: GET /api/v1/block/:hash */
std::string apiBlockByHash(const std::string& hash, RpcContext* ctx);

/** REST: GET /api/v1/block/height/:height */
std::string apiBlockByHeight(uint64_t height, RpcContext* ctx);

} // namespace shawncoin

#endif // SHAWNCOIN_RPC_API_HPP
