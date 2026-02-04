# Shawn Coin (SHWN) – Technical Whitepaper

## Abstract

Shawn Coin is a decentralized, open-source cryptocurrency implementing a Proof-of-Work blockchain with a fixed supply of 12 million SHWN. It uses SHA-256 mining, a UTXO model, and a custom P2P protocol with magic bytes `0x53484157` (SHAW). This document describes the architecture, consensus, and core components.

## 1. Introduction

Shawn Coin aims to provide a simple, auditable, and performant digital currency. The design follows established patterns from Bitcoin-style chains while using a fixed supply schedule and a 10-minute block target with difficulty adjustment every 2016 blocks.

## 2. Coin Parameters

| Parameter | Value |
|-----------|--------|
| Name | Shawn Coin |
| Ticker | SHWN |
| Total supply | 12,000,000 SHWN |
| Block reward | 25 SHWN initially, halving every 210,000 blocks |
| Block time target | 10 minutes (600 s) |
| Difficulty adjustment | Every 2016 blocks |
| Consensus | Proof of Work (SHA-256) |
| Address prefix | 'S' (Base58Check) |
| P2P port | 7333 |
| RPC port | 7332 |

## 3. Architecture

### 3.1 Technology Stack

- **Core engine**: C++17 (blockchain, transactions, UTXO, consensus)
- **Cryptography**: C (OpenSSL + optional libsecp256k1), SHA-256, RIPEMD-160, ECDSA secp256k1
- **Network**: C++ with Boost.Asio (async TCP, custom binary protocol)
- **Storage**: RocksDB or in-memory chain state
- **RPC/API**: C++ (RESTinio optional), JSON-RPC 2.0
- **CLI**: C++ (Boost.Program_options, optional ncurses)
- **Build**: CMake

### 3.2 Block Structure

- **Header** (80 bytes for hashing): version (4), previous_hash (32), merkle_root (32), timestamp (8), difficulty_target (4), nonce (4).
- **Transactions**: variable-length list; first transaction must be coinbase.

### 3.3 Transaction Model (UTXO)

- **Inputs**: prev_tx_hash, output_index, signature, pubkey.
- **Outputs**: amount (satoshis), script_pubkey (e.g. P2PKH).
- **TxID**: double SHA-256 of serialized transaction.

### 3.4 Genesis Block

- Message: *"Shawn Coin - The Future of Digital Currency 2026"*
- Merkle root derived from this message; coinbase pays 25 SHWN to a placeholder output.

## 4. Consensus

- **Longest chain**: tip with greatest cumulative proof-of-work.
- **Proof-of-Work**: double SHA-256 of block header must be below target (compact `nBits`).
- **Difficulty**: every 2016 blocks, target is adjusted so that actual elapsed time approximates 20160 minutes (10 min/block).

## 5. Network Protocol

- **Magic**: `0x53484157` (SHAW).
- **Messages**: VERSION, VERACK, GETADDR, ADDR, INV, GETDATA, BLOCK, TX, PING, PONG, GETBLOCKS, GETHEADERS.
- **Format**: [magic 4][command 12][length 4][checksum 4][payload].

## 6. Security

- Constant-time comparison for sensitive data.
- Secure memory wiping for private keys.
- Compiler flags: stack protector, Fortify source.
- Wallet encryption (AES-256) and BIP39/BIP32 for HD wallets.

## 7. References

- Bitcoin Core (reference implementation).
- BIP32 (Hierarchical Deterministic Wallets).
- BIP39 (Mnemonic code for key generation).
