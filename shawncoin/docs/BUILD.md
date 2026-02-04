# Shawn Coin - Build Instructions

## Requirements

- **C++17** compiler (GCC 8+, Clang 7+, or MSVC 2019+)
- **C11** for crypto layer
- **CMake** 3.16+
- **OpenSSL** 1.1.1 or 3.x
- **Boost** 1.75+ (system, filesystem, thread; program_options for CLI)
- **RocksDB** 6.0+ (optional; in-memory stub used if not found)
- **libsecp256k1** (optional; OpenSSL ECDSA used if not found)

## Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libboost-all-dev
# Optional: RocksDB, libsecp256k1, ncurses, libqrencode
sudo apt install librocksdb-dev libsecp256k1-dev libncurses-dev libqrencode-dev

cd shawncoin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install  # optional
```

## macOS

```bash
brew install cmake openssl boost
# Optional: rocksdb, secp256k1, ncurses, qrencode
brew install rocksdb secp256k1 ncurses qrencode

cd shawncoin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Windows (MSVC or MinGW)

- Install Visual Studio 2019+ with C++ workload, or MinGW-w64.
- Install vcpkg, then:
  ```bash
  vcpkg install openssl boost-system boost-filesystem boost-thread
  ```
- Open CMake GUI or use command line:
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
  cmake --build . --config Release
  ```

## Options

- `-DBUILD_TESTS=ON` – build unit tests (requires Google Test)
- `-DBUILD_CLI=ON` – build `shawncoin-cli` (default ON)
- `-DUSE_NCURSES=ON` – use ncurses in CLI (default ON if found)
- `-DUSE_RESTINIO=ON` – use RESTinio for RPC (default ON if found)

## Output

- `build/shawncoind` – main daemon
- `build/shawncoin-cli` – command-line interface

## Run

```bash
./shawncoind --datadir=~/.shawncoin
./shawncoin-cli --help
```
