#include <gtest/gtest.h>
#include "core/blockchain.hpp"
#include "core/mempool.hpp"
#include "mining/miner.hpp"
#include "wallet/wallet.hpp"
#include "crypto/address.hpp"
#include <iostream>
// For direct derivation in tests
#include "wallet/mnemonic.hpp"
#include "crypto/keys.h"
#include "crypto/hash.h"

using namespace shawncoin;

TEST(MinerTest, CoinbasePayoutAndAccept) {
    Blockchain chain;
    Mempool mempool;
    Miner miner(chain, mempool);
    Wallet w;
    ASSERT_TRUE(w.create(""));
    std::string addr = w.generateNewAddress();
    std::cerr << "DEBUG: mnemonic=" << w.getMnemonic() << " addr='" << addr << "'\n";
    // Some address decoding in-tests can be flaky across builds; derive the
    // pubkey-hash directly from the HD private key to avoid Base58 decode
    // inconsistencies in the test environment.
    auto seed = mnemonicToSeed(w.getMnemonic(), "");
    HDWallet hd; hd.setSeed(seed);
    std::vector<uint8_t> priv = hd.derivePrivateKey(0);
    ASSERT_EQ(priv.size(), 32);
    shawncoin_keypair_t* key = shawncoin_keypair_from_priv(priv.data());
    ASSERT_NE(key, nullptr);
    unsigned char pub[33];
    ASSERT_EQ(shawncoin_pubkey_get(key, pub), 1);
    unsigned char hash160[20];
    shawncoin_hash160(pub, 33, hash160);
    std::vector<uint8_t> hash(hash160, hash160 + 20);
    shawncoin_keypair_destroy(key);

    ASSERT_EQ(hash.size(), 20);

    // Build a block with coinbase paying this address (hash)
    Block block;
    block.header.version = 1;
    block.header.previous_hash = chain.getBestBlockHash();
    block.header.timestamp = (uint64_t)std::time(nullptr);
    Transaction coinbase;
    coinbase.version = 1;
    coinbase.inputs.resize(1);
    coinbase.inputs[0].prev_tx_hash = {};
    coinbase.inputs[0].output_index = 0xffffffffu;
    coinbase.outputs.resize(1);
    coinbase.outputs[0].amount = getBlockSubsidy(chain.getHeight() + 1);
    coinbase.outputs[0].script_pubkey = {0x76, 0xa9, 0x14};
    coinbase.outputs[0].script_pubkey.insert(coinbase.outputs[0].script_pubkey.end(), hash.begin(), hash.end());
    coinbase.outputs[0].script_pubkey.push_back(0x88);
    coinbase.outputs[0].script_pubkey.push_back(0xac);
    block.transactions.push_back(coinbase);

    // Try mining with easy difficulty - should find a nonce quickly
    ASSERT_TRUE(miner.mineBlock(block, EASY_MINE_DIFFICULTY));

    uint64_t newHeight = chain.getHeight() + 1;
    bool ok = chain.addBlock(block, newHeight);
    ASSERT_TRUE(ok);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
