#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "wallet/wallet.hpp"
#include "util/config.hpp"

using namespace shawncoin;

static void printUsage() {
    std::cout << "wallet_tool - simple local wallet utility (use only locally).\n"
              << "Usage:\n"
              << "  wallet_tool create [--passphrase=PASS] [--datadir=DIR] [--count=N] [--backup]\n"
              << "  wallet_tool restore --mnemonic=\"words...\" [--passphrase=PASS] [--datadir=DIR] [--count=N] [--backup]\n"
              << "Options:\n"
              << "  --count=N     Number of addresses to derive (default 5)\n"
              << "  --backup      Write mnemonic to <datadir>/wallet_backup.txt\n"
              << "\n"
              << "Important: the repository's mnemonic implementation is a simplified/stub version and is NOT secure for real funds.\n"
              << "Do not use these mnemonics for real value.\n";
}

static std::string getArgValue(int argc, char* argv[], const std::string& name) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.find(name + "=") == 0) return a.substr(name.size() + 1);
    }
    return std::string();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string cmd = argv[1];
    std::string passphrase = getArgValue(argc, argv, "--passphrase");
    std::string datadir = getArgValue(argc, argv, "--datadir");
    std::string mnemonicArg = getArgValue(argc, argv, "--mnemonic");
    bool doBackup = false;
    int count = 5;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--backup") doBackup = true;
        if (a.find("--count=") == 0) count = std::stoi(a.substr(8));
    }

    shawncoin::Config cfg;
    if (!datadir.empty()) cfg.loadFromArgs(0, nullptr); // noop, but keep signature
    std::string dataDir = datadir.empty() ? cfg.getDataDir() : datadir;

    shawncoin::Wallet w;
    if (cmd == "create") {
        if (!w.create(passphrase)) {
            std::cerr << "Failed to create wallet.\n";
            return 1;
        }
        std::string mnemonic = w.getMnemonic();
        std::cout << "Generated mnemonic (store this offline):\n" << mnemonic << "\n\n";
        if (doBackup) {
            std::ofstream f(dataDir + "/wallet_backup.txt");
            if (f) f << mnemonic << "\n";
            std::cout << "Mnemonic written to: " << dataDir << "/wallet_backup.txt\n";
        }
    } else if (cmd == "restore") {
        if (mnemonicArg.empty()) {
            std::cerr << "Missing --mnemonic for restore.\n";
            return 1;
        }
        if (!w.restore(mnemonicArg, passphrase)) {
            std::cerr << "Failed to restore wallet from mnemonic.\n";
            return 1;
        }
        std::cout << "Wallet restored from provided mnemonic.\n";
        if (doBackup) {
            std::ofstream f(dataDir + "/wallet_backup.txt");
            if (f) f << mnemonicArg << "\n";
            std::cout << "Mnemonic written to: " << dataDir << "/wallet_backup.txt\n";
        }
    } else {
        printUsage();
        return 1;
    }

    std::cout << "Deriving first " << count << " addresses:\n";
    for (int i = 0; i < count; ++i) {
        std::string addr = w.generateNewAddress();
        // Note: Wallet::generateNewAddress uses internal index based on keys_.size().
        std::cout << "  [" << i << "] " << addr << "\n";
    }

    return 0;
}
