#include <gtest/gtest.h>
#include "core/blockchain.hpp"
#include "core/block.hpp"
#include "core/types.hpp"
#include "util/util.hpp"

using namespace shawncoin;

TEST(Blockchain, Genesis) {
    Blockchain chain;
    Block genesis = chain.getGenesisBlock();
    EXPECT_EQ(genesis.transactions.size(), 1u);
    EXPECT_TRUE(genesis.transactions[0].isCoinbase());
    EXPECT_EQ(chain.getHeight(), 0u);
    uint256 hash = genesis.getHash();
    EXPECT_EQ(chain.getBestBlockHash(), hash);
}

TEST(Blockchain, GetBlockByHeight) {
    Blockchain chain;
    auto block = chain.getBlockByHeight(0);
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->getHash(), chain.getBestBlockHash());
}

TEST(Types, GetBlockSubsidy) {
    EXPECT_EQ(getBlockSubsidy(0), 25 * COIN);
    EXPECT_EQ(getBlockSubsidy(209999), 25 * COIN);
    EXPECT_EQ(getBlockSubsidy(210000), 25 * COIN / 2);
    EXPECT_EQ(getBlockSubsidy(419999), 25 * COIN / 2);
    EXPECT_EQ(getBlockSubsidy(420000), 25 * COIN / 4);
}
