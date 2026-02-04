#include "core/blockchain.hpp"
#include "core/block.hpp"
#include "core/mempool.hpp"
#include "storage/chainstate.hpp"
#include "network/node.hpp"
// #include "rpc/server.hpp"
// #include "rpc/api.hpp"
#include "mining/miner.hpp"
#include "wallet/wallet.hpp"
#include "util/config.hpp"
#include "util/logger.hpp"
#include "util/util.hpp"
#include "core/types.hpp"
#include "crypto/address.hpp"
#include <iostream>
#include <fstream>
#include <csignal>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

// Global shutdown flag for signal handling
static std::atomic<bool> g_shutdown{false};

// Signal handler for graceful shutdown
static void signalHandler(int signal) {
    SHAWNCOIN_LOG(Info, "main", "Received signal %d, initiating graceful shutdown", signal);
    g_shutdown.store(true);
}



int main(int argc, char* argv[]) {
    try {
        shawncoin::Config config;
        config.loadFromArgs(argc, argv);
        std::string dataDir = config.getDataDir();
        
        // Validate data directory path
        if (dataDir.empty()) {
            std::cerr << "Error: Data directory cannot be empty" << std::endl;
            return 1;
        }
        
        config.load(dataDir + "/shawncoin.conf");

        shawncoin::Logger::instance().setFile(config.get("debug.logfile", ""));
        SHAWNCOIN_LOG(Info, "main", "Shawn Coin (SHWN) starting - %s", dataDir.c_str());

        // Set up signal handlers for graceful shutdown
        std::signal(SIGINT, signalHandler);
#ifdef SIGTERM
        std::signal(SIGTERM, signalHandler);
#endif
#ifdef SIGHUP
        std::signal(SIGHUP, signalHandler);  // Handle terminal close
#endif

    // Initialize blockchain and chain state with proper RAII
    shawncoin::Blockchain chain;
    shawncoin::MemoryChainState chainState;
    
    if (!chainState.init(dataDir)) {
        SHAWNCOIN_LOG(Error, "main", "Failed to init chain state at %s", dataDir.c_str());
        return 1;
    }
    chain.setChainState(&chainState);
    
    if (!chain.init(dataDir)) {
        SHAWNCOIN_LOG(Error, "main", "Failed to init blockchain at %s", dataDir.c_str());
        return 1;
    }

    // Initialize mempool and P2P node
    shawncoin::Mempool mempool;
    shawncoin::Node node(chain, mempool);
    uint16_t p2pPort = config.getPort("port", shawncoin::P2P_PORT);
    
    node.start(p2pPort);
    SHAWNCOIN_LOG(Info, "main", "P2P node started on port %u", (unsigned)p2pPort);

    // Initialize wallet (file-based or in-memory)
    shawncoin::Wallet wallet;
    
    // Try to load encrypted wallet file first (wallet.dat), fallback to plaintext wallet_backup.txt
    std::string walletDat = dataDir + "/wallet.dat";
    std::string walletBackupPath = dataDir + "/wallet_backup.txt";
    std::string walletPass = config.get("walletpass", "");
    
    if (std::ifstream(walletDat)) {
        if (wallet.loadFromFile(walletDat, walletPass)) {
            SHAWNCOIN_LOG(Info, "main", "Loaded encrypted wallet from %s", walletDat.c_str());
        } else {
            SHAWNCOIN_LOG(Warn, "main", "Failed to load wallet from %s", walletDat.c_str());
            SHAWNCOIN_LOG(Info, "main", "Proceeding with new wallet instance");
        }
    } else if (std::ifstream(walletBackupPath)) {
        std::ifstream wf(walletBackupPath);
        if (wf) {
            std::string mnemonic;
            if (std::getline(wf, mnemonic) && !mnemonic.empty()) {
                if (wallet.restore(mnemonic, "")) {
                    SHAWNCOIN_LOG(Info, "main", "Restored wallet from mnemonic in %s", walletBackupPath.c_str());
                } else {
                    SHAWNCOIN_LOG(Warn, "main", "Failed to restore wallet from %s", walletBackupPath.c_str());
                }
            }
        }
    } else {
        SHAWNCOIN_LOG(Info, "main", "No existing wallet found, using new wallet (run 'createwallet' RPC or CLI to generate)");
    }
    // shawncoin::RpcServer rpcServer(&rpcCtx);
    // uint16_t rpcPort = config.getPort("rpcport", shawncoin::RPC_PORT);
    // rpcServer.start(rpcPort);

    SHAWNCOIN_LOG(Info, "main", "Shawn Coin node running. P2P port %u", (unsigned)p2pPort);
    SHAWNCOIN_LOG(Info, "main", "Best block: %s height %llu", shawncoin::uint256ToHex(chain.getBestBlockHash()).c_str(), (unsigned long long)chain.getHeight());

    std::unique_ptr<shawncoin::Miner> miner;
    bool doMine = config.getInt("gen", 0) != 0 || config.get("mine", "") == "1";
    int mineThreads = config.getInt("genproclimit", 2);
    if (doMine) {
        miner = std::make_unique<shawncoin::Miner>(chain, mempool);
        std::string mineAddr = config.get("mineaddr", "");
        if (!mineAddr.empty()) {
            std::vector<uint8_t> h = shawncoin::addressToPubKeyHash(mineAddr);
                if (h.size() == 20) {
                miner->setPayoutHash(h);
                SHAWNCOIN_LOG(Info, "main", "Miner payout address set: %s", mineAddr.c_str());
            } else {
                SHAWNCOIN_LOG(Warn, "main", "Invalid mineaddr in config: %s", mineAddr.c_str());
            }
        }
    // expose miner to RPC context so RPC can control it
    // rpcCtx.miner = miner.get();
    miner->start(static_cast<uint32_t>(mineThreads));
    SHAWNCOIN_LOG(Info, "main", "Mining started with %d thread(s). Use easy difficulty for CPU.", mineThreads);
    std::cout << "Shawn Coin mining started (" << mineThreads << " thread(s)). Blocks will appear below." << std::endl;
    }

    while (!g_shutdown.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    SHAWNCOIN_LOG(Info, "main", "Shutting down...");
    if (miner) miner->stop();
    uint64_t finalHeight = chain.getHeight();
    uint64_t totalIssued = shawncoin::getTotalSupplyUpTo(finalHeight);
    {
        std::ofstream f(dataDir + "/mined_summary.txt");
        if (f) {
            f << "blocks=" << finalHeight << "\n";
            f << "total_shwn=" << (totalIssued / shawncoin::COIN) << "\n";
        }
    }
    // rpcServer.stop();
    node.stop();
    chainState.shutdown();
    SHAWNCOIN_LOG(Info, "main", "Shawn Coin stopped. Mined %llu blocks, %llu SHWN total.", (unsigned long long)finalHeight, (unsigned long long)(totalIssued / shawncoin::COIN));
    return 0;
    
    } catch (const std::exception& e) {
        SHAWNCOIN_LOG(Error, "main", "Fatal error: %s", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        SHAWNCOIN_LOG(Error, "main", "Unknown fatal error occurred");
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}
