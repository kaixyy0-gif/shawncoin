#include "util/config.hpp"
#include "util/logger.hpp"
#include "util/util.hpp"
#include "core/types.hpp"
#include "crypto/address.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>

#if defined(HAVE_NLOHMANN_JSON)
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

// Color codes for friendly output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

static std::string base64_encode(const std::string &in) {
    static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val=0, valb=-6;
    for (unsigned char c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb>-6) out.push_back(b64[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

static std::string rpcPost(const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& body) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &sa.sin_addr) <= 0) {
        close(sock);
        return "";
    }
    if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) != 0) { close(sock); return ""; }
    std::ostringstream req;
    req << "POST / HTTP/1.1\r\n";
    req << "Host: " << host << ":" << port << "\r\n";
    req << "Content-Type: application/json\r\n";
    req << "Content-Length: " << body.size() << "\r\n";
    if (!user.empty() || !pass.empty()) {
        std::string auth = user + ":" + pass;
        req << "Authorization: Basic " << base64_encode(auth) << "\r\n";
    }
    req << "Connection: close\r\n\r\n";
    req << body;
    std::string s = req.str();
    ssize_t w = write(sock, s.c_str(), s.size());
    (void)w;
    std::string resp;
    char buf[4096];
    ssize_t r;
    while ((r = read(sock, buf, sizeof(buf))) > 0) resp.append(buf, buf + r);
    close(sock);
    size_t pos = resp.find("\r\n\r\n");
    if (pos == std::string::npos) return resp;
    return resp.substr(pos + 4);
}

static void printLogo() {
    std::cout << CYAN << R"(
    ____  _                      ____      _       
   / ___|| |__   __ _ _ __   ___/ ___|___ (_) _ __  
   \___ \| '_ \ / _` | '_ \ / _ \ |   / _ \| | '_ \ 
    ___) | | | | (_| | | | |  __/ |__| (_) | | | | |
   |____/|_| |_|\__,_|_| |_|\___|\____\___/|_|_| |_|
                                                
    )" << RESET << BOLD << YELLOW << "Shawn Coin (SHWN) - Easy Crypto for Everyone!" << RESET << "\n\n";
}

static void printHelp() {
    printLogo();
    std::cout << BOLD << "🚀 SIMPLE COMMANDS:" << RESET << "\n\n";
    
    std::cout << GREEN << "💰 WALLET COMMANDS:" << RESET << "\n";
    std::cout << "  " << BOLD << "getnewaddress" << RESET << "      Get a new SHWN address\n";
    std::cout << "  " << BOLD << "getbalance" << RESET << "         Show your wallet balance\n";
    std::cout << "  " << BOLD << "sendtoaddress" << RESET << "    Send SHWN to someone\n";
    std::cout << "  " << BOLD << "listtransactions" << RESET << "   Show your transaction history\n";
    std::cout << "  " << BOLD << "createwallet" << RESET << "    Create a new wallet\n";
    std::cout << "  " << BOLD << "backupwallet" << RESET << "      Backup your wallet\n";
    
    std::cout << "\n" << GREEN << "⛏️  MINING COMMANDS:" << RESET << "\n";
    std::cout << "  " << BOLD << "getmininginfo" << RESET << "     Show mining status\n";
    std::cout << "  " << BOLD << "getblockcount" << RESET << "     Show current block height\n";
    std::cout << "  " << BOLD << "startmining" << RESET << "      Start mining (threads optional)\n";
    std::cout << "  " << BOLD << "stopmining" << RESET << "       Stop mining\n";
    
    std::cout << "\n" << GREEN << "📊 NETWORK COMMANDS:" << RESET << "\n";
    std::cout << "  " << BOLD << "getblockchaininfo" << RESET << " Show network information\n";
    std::cout << "  " << BOLD << "getnetworkinfo" << RESET << "    Show connection info\n";
    
    std::cout << "\n" << GREEN << "🔧 UTILITIES:" << RESET << "\n";
    std::cout << "  " << BOLD << "validateaddress" << RESET << "   Check if address is valid\n";
    std::cout << "  " << BOLD << "--help" << RESET << "               Show this help\n";
    
    std::cout << "\n" << BLUE << "📖 EXAMPLES:" << RESET << "\n";
    std::cout << "  shawncoin-cli getnewaddress\n";
    std::cout << "  shawncoin-cli getbalance\n";
    std::cout << "  shawncoin-cli sendtoaddress Sxx... 1.5\n";
    std::cout << "  shawncoin-cli createwallet\n";
    std::cout << "  shawncoin-cli startmining 4\n";
    std::cout << "  shawncoin-cli stopmining\n";
    std::cout << "  shawncoin-cli getblockcount\n\n";
    
    std::cout << YELLOW << "💡 TIP: Start mining with: shawncoind --mine=1\n" << RESET << "\n";
}

static void printSuccess(const std::string& message) {
    std::cout << GREEN << "✅ " << message << RESET << "\n";
}

static void printError(const std::string& message) {
    std::cout << RED << "❌ " << message << RESET << "\n";
}

static void printInfo(const std::string& message) {
    std::cout << BLUE << "ℹ️  " << message << RESET << "\n";
}

static double satoshiToSHWN(uint64_t satoshis) {
    return static_cast<double>(satoshis) / 100000000.0;
}

static uint64_t shwnToSatoshi(double shwn) {
    return static_cast<uint64_t>(std::round(shwn * 100000000.0));
}

static std::string formatAmount(uint64_t satoshis) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8) << satoshiToSHWN(satoshis) << " SHWN";
    return ss.str();
}

static bool isValidAmount(const std::string& amount) {
    try {
        double val = std::stod(amount);
        return val > 0 && val <= 12000000; // Max supply check
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    // Check for help first
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printHelp();
            return 0;
        }
    }

    shawncoin::Config config;
    config.loadFromArgs(argc, argv);
    
    std::string rpcHost = "127.0.0.1";
    uint16_t rpcPort = config.getPort("rpcport", shawncoin::RPC_PORT);
    std::string rpcUser = config.get("rpcuser", "");
    std::string rpcPass = config.get("rpcpassword", "");

    // Find command (first non-argument)
    std::string command;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' && command.empty()) {
            command = argv[i];
            break;
        }
    }
    
    if (command.empty()) {
        printHelp();
        return 0;
    }

    // Simple wallet commands
    if (command == "getnewaddress") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.getnewaddress\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("Your new SHWN address:");
            std::cout << BOLD << CYAN << "   " << res << RESET << "\n";
            std::cout << YELLOW << "💡 Share this address to receive SHWN payments!" << RESET << "\n";
        } else {
            printError("Could not get new address. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "getbalance") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.getbalance\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "💰 Your Wallet Balance:" << RESET << "\n";
            std::cout << BOLD << "   " << formatAmount(std::stoull(res)) << RESET << "\n\n";
            std::cout << YELLOW << "💡 Start mining to earn more SHWN: shawncoind --mine=1" << RESET << "\n";
        } else {
            printError("Could not get balance. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "sendtoaddress") {
        if (argc < 4) {
            printError("Usage: shawncoin-cli sendtoaddress <address> <amount>");
            std::cout << YELLOW << "💡 Example: shawncoin-cli sendtoaddress Sabc123def456 1.5" << RESET << "\n";
            return 1;
        }
        
        std::string address = argv[2];
        std::string amountStr = argv[3];
        
        if (!isValidAmount(amountStr)) {
            printError("Invalid amount. Please use a number between 0.00000001 and 12000000");
            return 1;
        }
        
        uint64_t amount = shwnToSatoshi(std::stod(amountStr));
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.sendtoaddress\",\"params\":[\"" + address + "\"," + std::to_string(amount) + ",0]}";
        
        std::cout << BLUE << "📤 Sending " << formatAmount(amount) << " to " << address << "..." << RESET << "\n";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("Transaction sent successfully!");
            std::cout << GREEN << "🔗 Transaction ID: " << CYAN << res << RESET << "\n";
            std::cout << YELLOW << "💡 It may take a few minutes to confirm." << RESET << "\n";
        } else {
            printError("Transaction failed! Check your balance and the address.");
            std::cout << RED << "Details: " << res << RESET << "\n";
        }
        return 0;
    }

    if (command == "listtransactions") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.listtransactions\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "📋 Your Transaction History:" << RESET << "\n";
            std::cout << BOLD << "   " << res << RESET << "\n";
        } else {
            printError("Could not get transactions. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "backupwallet") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.backup\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            printSuccess("Wallet backed up successfully!");
            std::cout << YELLOW << "💡 Keep your backup safe and private!" << RESET << "\n";
        } else {
            printError("Wallet backup failed!");
        }
        return 0;
    }

    // Mining commands
    if (command == "getmininginfo") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"mining.getinfo\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "⛏️  Mining Information:" << RESET << "\n";
            std::cout << BOLD << "   " << res << RESET << "\n";
        } else {
            printError("Could not get mining info. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "getblockcount") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"blockchain.info\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "🧱 Blockchain Info:" << RESET << "\n";
            std::cout << BOLD << "   " << res << RESET << "\n\n";
            std::cout << BLUE << "🏆 Network is active! 🚀" << RESET << "\n";
        } else {
            printError("Could not get block count. Make sure shawncoind is running!");
        }
        return 0;
    }

    // Mining control commands
    if (command == "startmining") {
        int threads = 2;
        if (argc >= 3) {
            try { threads = std::stoi(argv[2]); }
            catch (...) { threads = 2; }
        }
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"mining.start\",\"params\":{\"threads\":" + std::to_string(threads) + "}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("Mining started with " + std::to_string(threads) + " threads!");
            std::cout << YELLOW << "💡 Use 'stopmining' to stop, 'getmininginfo' to check status" << RESET << "\n";
        } else {
            printError("Could not start mining. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "stopmining") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"mining.stop\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("Mining stopped!");
            std::cout << YELLOW << "💡 Use 'startmining' to start again" << RESET << "\n";
        } else {
            printError("Could not stop mining. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "createwallet") {
        std::string password = "";
        if (argc >= 3) password = argv[2];
        std::cout << BLUE << "🔐 Creating new wallet..." << RESET << "\n";
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.create\",\"params\":{\"passphrase\":\"" + password + "\"}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("New wallet created successfully!");
            std::cout << YELLOW << "💡 Use 'getnewaddress' to get your first address" << RESET << "\n";
        } else {
            printError("Could not create wallet!");
        }
        return 0;
    }

    if (command == "backupwallet") {
        std::string path = "wallet_backup.txt";
        if (argc >= 3) path = argv[2];
        std::cout << BLUE << "💾 Backing up wallet to " + path << "..." << RESET << "\n";
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"wallet.backup\",\"params\":{\"path\":\"" + path + "\"}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty() && res.find("\"result\"") != std::string::npos) {
            printSuccess("Wallet backed up successfully!");
            std::cout << YELLOW << "💡 Keep your backup file safe!" << RESET << "\n";
        } else {
            printError("Could not backup wallet!");
        }
        return 0;
    }

    // Network commands
    if (command == "getblockchaininfo") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"blockchain.getinfo\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "📊 Blockchain Information:" << RESET << "\n";
            std::cout << BOLD << "   " << res << RESET << "\n";
        } else {
            printError("Could not get blockchain info. Make sure shawncoind is running!");
        }
        return 0;
    }

    if (command == "getnetworkinfo") {
        std::string body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"network.getinfo\",\"params\":{}}";
        std::string res = rpcPost(rpcHost, rpcPort, rpcUser, rpcPass, body);
        if (!res.empty()) {
            std::cout << GREEN << BOLD << "🌐 Network Information:" << RESET << "\n";
            std::cout << BOLD << "   " << res << RESET << "\n";
        } else {
            printError("Could not get network info. Make sure shawncoind is running!");
        }
        return 0;
    }

    // Utilities
    if (command == "validateaddress") {
        if (argc < 3) {
            printError("Usage: shawncoin-cli validateaddress <address>");
            return 1;
        }
        
        std::string address = argv[2];
        std::vector<uint8_t> hash = shawncoin::addressToPubKeyHash(address);
        
        if (hash.size() == 20) {
            printSuccess("Address is valid!");
            std::cout << GREEN << "✅ " << address << " is a valid SHWN address" << RESET << "\n";
        } else {
            printError("Address is invalid!");
            std::cout << RED << "❌ " << address << " is NOT a valid SHWN address" << RESET << "\n";
        }
        return 0;
    }

    // Special command to show mining results
    if (command == "mined") {
        std::string dataDir = config.getDataDir();
        std::ifstream f(dataDir + "/mined_summary.txt");
        if (f) {
            std::string line;
            long long blocks = 0, totalShwn = 0;
            while (std::getline(f, line)) {
                if (line.compare(0, 7, "blocks=") == 0) blocks = std::stoll(line.substr(7));
                if (line.compare(0, 11, "total_shwn=") == 0) totalShwn = std::stoll(line.substr(11));
            }
            std::cout << GREEN << BOLD << "🏆 Mining Summary:" << RESET << "\n";
            std::cout << BOLD << "   🧱 Blocks mined: " << YELLOW << blocks << RESET << "\n";
            std::cout << BOLD << "   💰 Total SHWN: " << YELLOW << totalShwn << " SHWN" << RESET << "\n\n";
            std::cout << BLUE << "🎉 Amazing! You're helping build the Shawn Coin network!" << RESET << "\n";
            return 0;
        }
        printError("No mining summary found. Start mining first: shawncoind --mine=1");
        return 1;
    }

    // Default: unknown command
    printError("Unknown command: " + command);
    std::cout << YELLOW << "💡 Use --help to see all available commands" << RESET << "\n";
    return 1;
}