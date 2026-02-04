cmake_minimum_required(VERSION 3.16)
project(shawncoin VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Security and hardening
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(
    -Wall -Wextra -Wpedantic
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -fno-strict-overflow
  )
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O3 -DNDEBUG)
  endif()
endif()

if(MSVC)
  add_compile_options(/W4 /sdl)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_CLI "Build shawncoin-cli" ON)
option(USE_RESTINIO "Use RESTinio for RPC" ON)
option(USE_NCURSES "Use ncurses for CLI" ON)

# Dependencies - only essential ones required
find_package(OpenSSL 1.1.1 REQUIRED)

# Boost components - check which are actually needed
find_package(Boost 1.75 REQUIRED COMPONENTS system filesystem thread)
# Only include program_options if actually used in CLI
if(BUILD_CLI)
    find_package(Boost 1.75 REQUIRED COMPONENTS program_options)
endif()

# Optional dependencies - only enable if available
find_package(ZLIB QUIET)
find_package(Gzip QUIET)
find_package(PkgConfig QUIET)

# RocksDB - optimized detection with caching
if(DEFINED ENV{ROCKSDB_ROOT})
    set(ROCKSDB_ROOT $ENV{ROCKSDB_ROOT})
endif()

find_library(ROCKSDB_LIBRARY rocksdb PATHS ${ROCKSDB_ROOT} /usr/local /usr)
find_path(ROCKSDB_INCLUDE rocksdb/db.h PATHS ${ROCKSDB_ROOT}/include /usr/local/include /usr/include)

if(ROCKSDB_LIBRARY AND ROCKSDB_INCLUDE)
  set(ROCKSDB_FOUND TRUE)
  message(STATUS "Found RocksDB: ${ROCKSDB_LIBRARY}")
else()
  find_package(RocksDB QUIET)
endif()

if(NOT ROCKSDB_FOUND)
  message(STATUS "RocksDB not found. Using in-memory chain only (stub).")
  message(STATUS "Install RocksDB: apt-get install librocksdb-dev (Ubuntu) or brew install rocksdb (macOS)")
  set(USE_ROCKSDB OFF)
else()
  set(USE_ROCKSDB ON)
endif()

# libsecp256k1 - optional optimized crypto; fallback to OpenSSL ECDSA
option(USE_SECP256K1 "Use libsecp256k1 for optimized cryptography (recommended)" ON)

if(USE_SECP256K1)
  set(SECP256K1_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/secp256k1/include" CACHE PATH "secp256k1 include")
  set(SECP256K1_LIBRARY "" CACHE FILEPATH "secp256k1 library")
  set(HAVE_SECP256K1 FALSE)
  
  # Prefer system lib via pkg-config for faster builds
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(PC_SECP256K1 QUIET libsecp256k1)
    if(PC_SECP256K1_FOUND)
      set(SECP256K1_LIB ${PC_SECP256K1_LIBRARIES})
      set(SECP256K1_INCLUDE_DIR ${PC_SECP256K1_INCLUDE_DIRS})
      set(HAVE_SECP256K1 TRUE)
    endif()
  endif()
  
  # Fallback to bundled or system library
  if(NOT HAVE_SECP256K1)
    if(EXISTS "${SECP256K1_INCLUDE_DIR}/secp256k1.h")
      find_library(SECP256K1_LIB secp256k1 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/depends/secp256k1" /usr/local /usr)
      if(SECP256K1_LIB)
        set(HAVE_SECP256K1 TRUE)
      endif()
    endif()
  endif()
  
  if(NOT HAVE_SECP256K1)
    message(STATUS "libsecp256k1 not found. Using OpenSSL ECDSA (slower but functional).")
    set(HAVE_SECP256K1 FALSE)
  else()
    message(STATUS "libsecp256k1 found; enabling optimized cryptography")
  endif()
else()
  set(HAVE_SECP256K1 FALSE)
  message(STATUS "libsecp256k1 disabled by user. Using OpenSSL ECDSA.")
endif()

# Optional dependencies - use parallel search for faster configuration
if(USE_NCURSES)
  find_package(Curses QUIET)
  set(HAVE_NCURSES ${CURSES_FOUND})
endif()

if(USE_RESTINIO)
  find_package(fmt QUIET)
  find_package(restinio QUIET)
  set(HAVE_RESTINIO ${restinio_FOUND})
endif()

# QR code generation for CLI
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(QRENCODE QUIET libqrencode)
endif()
set(HAVE_QRENCODE ${QRENCODE_FOUND})

# JSON library for RPC (lightweight alternative preferred)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
  set(HAVE_NLOHMANN_JSON TRUE)
  add_compile_definitions(HAVE_NLOHMANN_JSON=1)
  message(STATUS "Found nlohmann_json for RPC")
else()
  set(HAVE_NLOHMANN_JSON FALSE)
  message(STATUS "nlohmann_json not found, RPC will be limited")
endif()

# Compile definitions for feature flags
add_compile_definitions(
  SHAWNCOIN_VERSION_MAJOR=1
  SHAWNCOIN_VERSION_MINOR=0
  SHAWNCOIN_VERSION_PATCH=0
)
if(USE_ROCKSDB)
  add_compile_definitions(HAVE_ROCKSDB=1)
endif()
if(HAVE_SECP256K1)
  add_compile_definitions(HAVE_SECP256K1=1)
endif()
if(HAVE_NCURSES)
  add_compile_definitions(HAVE_NCURSES=1)
endif()
if(HAVE_RESTINIO)
  add_compile_definitions(HAVE_RESTINIO=1)
endif()
if(HAVE_QRENCODE)
  add_compile_definitions(HAVE_QRENCODE=1)
endif()

# Include directories - use target-based approach for better dependency management
set(SHAWNCOIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SHAWNCOIN_INCLUDE ${SHAWNCOIN_SRC})

# Modern CMake approach - avoid include_directories where possible
set(COMMON_INCLUDE_DIRS
  ${SHAWNCOIN_INCLUDE}
  ${OPENSSL_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)

if(USE_ROCKSDB)
  list(APPEND COMMON_INCLUDE_DIRS ${ROCKSDB_INCLUDE})
endif()

if(HAVE_SECP256K1)
  list(APPEND COMMON_INCLUDE_DIRS ${SECP256K1_INCLUDE_DIR})
endif()

# Crypto (C) - static library with proper target configuration
set(CRYPTO_SOURCES
  src/crypto/hash.c
  src/crypto/keys.c
  src/crypto/signatures.c
  src/crypto/secure_random.c
)

add_library(shawncoin_crypto STATIC ${CRYPTO_SOURCES})
target_include_directories(shawncoin_crypto PUBLIC ${COMMON_INCLUDE_DIRS})
target_link_libraries(shawncoin_crypto PUBLIC OpenSSL::SSL OpenSSL::Crypto)

if(HAVE_SECP256K1)
  target_include_directories(shawncoin_crypto PRIVATE ${SECP256K1_INCLUDE_DIR})
  target_link_libraries(shawncoin_crypto PRIVATE ${SECP256K1_LIB})
endif()

# Set compile options for crypto library
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(shawncoin_crypto PRIVATE -O3 -fomit-frame-pointer)
endif()

# Core and util sources (C++)
set(CORE_SOURCES
  src/core/types.cpp
  src/core/block.cpp
  src/core/transaction.cpp
  src/core/utxo.cpp
  src/core/consensus.cpp
  src/core/blockchain.cpp
  src/core/mempool.cpp
)
set(UTIL_SOURCES
  src/util/config.cpp
  src/util/logger.cpp
  src/util/serialize.cpp
  src/util/util.cpp
)
list(APPEND UTIL_SOURCES src/util/realtime.cpp)
set(CRYPTO_CPP_SOURCES src/crypto/address.cpp)
set(STORAGE_SOURCES
  src/storage/database.cpp
  src/storage/chainstate.cpp
)
set(MINING_SOURCES
  src/mining/merkle.cpp
  src/mining/difficulty.cpp
  src/mining/miner.cpp
  src/mining/stratum.cpp
)
set(NETWORK_SOURCES
  src/network/netbase.cpp
  src/network/protocol.cpp
  src/network/peer.cpp
  src/network/node.cpp
)
set(WALLET_SOURCES
  src/wallet/mnemonic.cpp
  src/wallet/hdwallet.cpp
  src/wallet/wallet.cpp
)
set(RPC_SOURCES
  src/rpc/api.cpp
  src/rpc/server.cpp
)

# Main daemon executable with optimized linking
add_executable(shawncoind
  src/main.cpp
  ${CORE_SOURCES}
  ${UTIL_SOURCES}
  ${CRYPTO_CPP_SOURCES}
  ${STORAGE_SOURCES}
  ${MINING_SOURCES}
  ${NETWORK_SOURCES}
  ${WALLET_SOURCES}
  ${RPC_SOURCES}
)

target_include_directories(shawncoind PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(shawncoind PRIVATE
  shawncoin_crypto
  Boost::system
  Boost::filesystem
)

# Only link Boost::thread if actually used
if(BUILD_CLI OR HAVE_RESTINIO)
  target_link_libraries(shawncoind PRIVATE Boost::thread)
endif()

# Conditional dependencies
if(nlohmann_json_FOUND)
  target_link_libraries(shawncoind PRIVATE nlohmann_json::nlohmann_json)
endif()

if(USE_ROCKSDB)
  target_link_libraries(shawncoind PRIVATE ${ROCKSDB_LIBRARY} pthread)
endif()

if(HAVE_RESTINIO)
  target_link_libraries(shawncoind PRIVATE restinio::restinio)
endif()

if(TARGET fmt::fmt)
  target_link_libraries(shawncoind PRIVATE fmt::fmt)
endif()

# Optimize build for release builds
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shawncoind PRIVATE -flto)
    target_link_options(shawncoind PRIVATE -flto)
  endif()
endif()

# CLI
if(BUILD_CLI)
  add_executable(shawncoin-cli cli/shawncoin-cli.cpp)
  target_link_libraries(shawncoin-cli PRIVATE
    shawncoin_crypto
    Boost::system
    Boost::filesystem
    Boost::program_options
  )
  if(HAVE_NCURSES)
    target_link_libraries(shawncoin-cli PRIVATE ${CURSES_LIBRARIES})
    target_include_directories(shawncoin-cli PRIVATE ${CURSES_INCLUDE_DIR})
  endif()
  # Link minimal core for CLI (types, util, config, logger)
  target_sources(shawncoin-cli PRIVATE
    src/util/config.cpp
    src/util/logger.cpp
    src/util/util.cpp
    src/core/types.cpp
    src/crypto/address.cpp
  )
endif()

# Wallet tool: small utility to create/restore a wallet and derive addresses
add_executable(wallet_tool tools/wallet_tool.cpp)
target_link_libraries(wallet_tool PRIVATE
  shawncoin_crypto
  Boost::system
  Boost::filesystem
)
if(nlohmann_json_FOUND)
  target_link_libraries(wallet_tool PRIVATE nlohmann_json::nlohmann_json)
endif()
target_sources(wallet_tool PRIVATE
  src/util/config.cpp
  src/util/logger.cpp
  src/util/util.cpp
  src/core/types.cpp
  src/core/utxo.cpp
  src/crypto/address.cpp
  src/wallet/mnemonic.cpp
  src/wallet/hdwallet.cpp
  src/wallet/wallet.cpp
  src/core/transaction.cpp
)
install(TARGETS wallet_tool RUNTIME DESTINATION bin)

# Tests
if(BUILD_TESTS)
  enable_testing()
  # Prefer system-provided GoogleTest; if missing, fetch it so tests can run
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GTest_FOUND TRUE)
  endif()
  if(GTest_FOUND)
    add_subdirectory(tests)
  endif()
endif()

# Install
install(TARGETS shawncoind RUNTIME DESTINATION bin)
if(BUILD_CLI)
  install(TARGETS shawncoin-cli RUNTIME DESTINATION bin)
endif()
