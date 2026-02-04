# Shawn Coin unit tests (optional; requires GTest)
find_package(GTest QUIET)
if(NOT GTest_FOUND)
  return()
endif()

add_executable(test_blockchain test_blockchain.cpp)
target_include_directories(test_blockchain PRIVATE ${SHAWNCOIN_INCLUDE})
target_link_libraries(test_blockchain PRIVATE
  shawncoin_crypto
    GTest::gtest
    GTest::gtest_main
  Boost::system
  Boost::filesystem
)
target_sources(test_blockchain PRIVATE
  ${CMAKE_SOURCE_DIR}/src/core/types.cpp
  ${CMAKE_SOURCE_DIR}/src/core/block.cpp
  ${CMAKE_SOURCE_DIR}/src/core/transaction.cpp
  ${CMAKE_SOURCE_DIR}/src/core/utxo.cpp
  ${CMAKE_SOURCE_DIR}/src/core/consensus.cpp
  ${CMAKE_SOURCE_DIR}/src/core/blockchain.cpp
    ${CMAKE_SOURCE_DIR}/src/mining/difficulty.cpp
  ${CMAKE_SOURCE_DIR}/src/mining/merkle.cpp
  ${CMAKE_SOURCE_DIR}/src/crypto/address.cpp
  ${CMAKE_SOURCE_DIR}/src/storage/chainstate.cpp
  ${CMAKE_SOURCE_DIR}/src/util/realtime.cpp
)
add_test(NAME test_blockchain COMMAND test_blockchain)

add_executable(test_wallet test_wallet.cpp)
target_include_directories(test_wallet PRIVATE ${SHAWNCOIN_INCLUDE})
target_link_libraries(test_wallet PRIVATE
  shawncoin_crypto
    GTest::gtest
    GTest::gtest_main
  Boost::system
  Boost::filesystem
)
target_sources(test_wallet PRIVATE
  ${CMAKE_SOURCE_DIR}/src/wallet/wallet.cpp
  ${CMAKE_SOURCE_DIR}/src/wallet/hdwallet.cpp
  ${CMAKE_SOURCE_DIR}/src/wallet/mnemonic.cpp
  ${CMAKE_SOURCE_DIR}/src/util/util.cpp
  ${CMAKE_SOURCE_DIR}/src/crypto/address.cpp
  ${CMAKE_SOURCE_DIR}/src/util/realtime.cpp
  ${CMAKE_SOURCE_DIR}/src/core/types.cpp
  ${CMAKE_SOURCE_DIR}/src/core/utxo.cpp
  ${CMAKE_SOURCE_DIR}/src/core/transaction.cpp
)
add_test(NAME test_wallet COMMAND test_wallet)

add_executable(test_mempool test_mempool.cpp)
target_include_directories(test_mempool PRIVATE ${SHAWNCOIN_INCLUDE})
target_link_libraries(test_mempool PRIVATE
  shawncoin_crypto
    GTest::gtest
    GTest::gtest_main
  Boost::system
  Boost::filesystem
)
target_sources(test_mempool PRIVATE
  ${CMAKE_SOURCE_DIR}/src/core/mempool.cpp
  ${CMAKE_SOURCE_DIR}/src/core/transaction.cpp
  ${CMAKE_SOURCE_DIR}/src/core/types.cpp
  ${CMAKE_SOURCE_DIR}/src/util/util.cpp
    ${CMAKE_SOURCE_DIR}/src/core/consensus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/block.cpp
    ${CMAKE_SOURCE_DIR}/src/core/blockchain.cpp
    ${CMAKE_SOURCE_DIR}/src/core/utxo.cpp
    ${CMAKE_SOURCE_DIR}/src/mining/merkle.cpp
    ${CMAKE_SOURCE_DIR}/src/mining/difficulty.cpp
    ${CMAKE_SOURCE_DIR}/src/util/realtime.cpp
)
add_test(NAME test_mempool COMMAND test_mempool)

add_executable(test_miner test_miner.cpp)
target_include_directories(test_miner PRIVATE ${SHAWNCOIN_INCLUDE})
target_link_libraries(test_miner PRIVATE
  shawncoin_crypto
    GTest::gtest
    GTest::gtest_main
  Boost::system
  Boost::filesystem
)
target_sources(test_miner PRIVATE
  ${CMAKE_SOURCE_DIR}/src/mining/miner.cpp
  ${CMAKE_SOURCE_DIR}/src/mining/merkle.cpp
  ${CMAKE_SOURCE_DIR}/src/core/block.cpp
  ${CMAKE_SOURCE_DIR}/src/core/types.cpp
  ${CMAKE_SOURCE_DIR}/src/crypto/address.cpp
  ${CMAKE_SOURCE_DIR}/src/util/util.cpp
  ${CMAKE_SOURCE_DIR}/src/wallet/wallet.cpp
  ${CMAKE_SOURCE_DIR}/src/wallet/hdwallet.cpp
  ${CMAKE_SOURCE_DIR}/src/wallet/mnemonic.cpp
    ${CMAKE_SOURCE_DIR}/src/core/blockchain.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mempool.cpp
    ${CMAKE_SOURCE_DIR}/src/core/transaction.cpp
    ${CMAKE_SOURCE_DIR}/src/core/utxo.cpp
    ${CMAKE_SOURCE_DIR}/src/util/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/util/serialize.cpp
    ${CMAKE_SOURCE_DIR}/src/core/consensus.cpp
    ${CMAKE_SOURCE_DIR}/src/mining/difficulty.cpp
    ${CMAKE_SOURCE_DIR}/src/util/realtime.cpp
)
add_test(NAME test_miner COMMAND test_miner)
