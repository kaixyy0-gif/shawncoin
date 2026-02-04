# Dependencies

Optional bundled dependencies can be placed here:

- **libsecp256k1**: Build from https://github.com/bitcoin-core/secp256k1 and set `SECP256K1_INCLUDE_DIR` and `SECP256K1_LIBRARY` in CMake.
- **RocksDB**: Install via system package or build from https://github.com/facebook/rocksdb and set `ROCKSDB_ROOT` or install to system.

Otherwise the build uses system libraries (OpenSSL, Boost) and optional stubs when RocksDB or libsecp256k1 are not found.
