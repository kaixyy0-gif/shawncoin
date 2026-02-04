#include <gtest/gtest.h>
#include "core/mempool.hpp"
#include "core/types.hpp"

using namespace shawncoin;

TEST(MempoolTest, AddAndTemplate) {
    Mempool mp;
    Transaction tx;
    tx.version = 1;
    TxInput in;
    // dummy prev hash
    in.prev_tx_hash = {};
    in.output_index = 0;
    tx.inputs.push_back(in);
    TxOutput out;
    out.amount = 1 * COIN;
    out.script_pubkey = {0x76, 0xa9, 0x14};
    out.script_pubkey.insert(out.script_pubkey.end(), 20, 0);
    out.script_pubkey.push_back(0x88); out.script_pubkey.push_back(0xac);
    tx.outputs.push_back(out);
    ASSERT_TRUE(mp.add(tx, 1000));
    ASSERT_EQ(mp.size(), 1);
    auto tmpl = mp.getBlockTemplate();
    ASSERT_EQ(tmpl.size(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
