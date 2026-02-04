# Shawn Coin (SHWN)

**Shawn Coin - The Future of Digital Currency 2026**

A high-performance, production-ready cryptocurrency implementation built with C/C++. Proof of Work consensus, UTXO model, 12 million total supply.

## Features

- **Consensus**: SHA-256 Proof of Work, 10-minute block target
- **Supply**: 12,000,000 SHWN fixed supply, halving every 210,000 blocks
- **Addresses**: Base58Check with prefix 'S'
- **Network**: Custom P2P protocol (magic 0x53484157), ports 7333 (P2P) / 7332 (RPC)
- **Storage**: RocksDB for blocks, UTXO set, and chain state
- **Wallet**: HD wallet with BIP39/BIP32, AES-256 encryption

## Quick Start

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./shawncoind --daemon
./shawncoin-cli blockchain info
```

See [docs/BUILD.md](docs/BUILD.md) for full build instructions.

## Project Structure

- `src/core/` - Blockchain, blocks, transactions, UTXO, consensus
- `src/crypto/` - Hashing, ECDSA, keys, addresses (C + C++)
- `src/wallet/` - HD wallet, mnemonic, key management
- `src/mining/` - Miner, difficulty, merkle tree
- `src/network/` - P2P node, peers, protocol
- `src/storage/` - RocksDB database, chainstate
- `src/rpc/` - REST/JSON-RPC API server
- `src/util/` - Config, logger, serialization
- `cli/` - Command-line interface

## Documentation

- [WHITEPAPER](docs/WHITEPAPER.md) - Architecture and design
- [BUILD](docs/BUILD.md) - Build instructions
- [API](docs/API.md) - RPC/API reference
- [PROTOCOL](docs/PROTOCOL.md) - Network protocol

## License

MIT License. See [LICENSE](LICENSE).
