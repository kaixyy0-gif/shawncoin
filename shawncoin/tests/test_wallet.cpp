#include <gtest/gtest.h>
#include "wallet/wallet.hpp"

using namespace shawncoin;

TEST(WalletTest, CreateSaveLoad) {
    Wallet w;
    ASSERT_TRUE(w.create(""));
    std::string m1 = w.getMnemonic();
    ASSERT_FALSE(m1.empty());
    // save to temp file (write into current working directory used by tests)
    std::string path = "wallet_test.dat";
    ASSERT_TRUE(w.saveToFile(path, "testpass"));
    Wallet w2;
    ASSERT_TRUE(w2.loadFromFile(path, "testpass"));
    std::string m2 = w2.getMnemonic();
    ASSERT_EQ(m1, m2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
