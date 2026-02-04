#include "core/block.hpp"
#include "core/transaction.hpp"
#include "mining/merkle.hpp"
#include "crypto/hash.h"
#include <cstring>
#include <algorithm>
#include <string>

namespace shawncoin {

static void writeU32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(v & 0xff);
    out.push_back((v >> 8) & 0xff);
    out.push_back((v >> 16) & 0xff);
    out.push_back((v >> 24) & 0xff);
}

static void writeU64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back((v >> (i * 8)) & 0xff);
}

static void writeVarInt(std::vector<uint8_t>& out, size_t n) {
    if (n < 0xfd) {
        out.push_back((uint8_t)n);
    } else if (n <= 0xffff) {
        out.push_back(0xfd);
        out.push_back(n & 0xff);
        out.push_back((n >> 8) & 0xff);
    } else {
        out.push_back(0xfe);
        out.push_back(n & 0xff);
        out.push_back((n >> 8) & 0xff);
        out.push_back((n >> 16) & 0xff);
        out.push_back((n >> 24) & 0xff);
    }
}

std::vector<uint8_t> serializeBlock(const Block& block) {
    std::vector<uint8_t> out;
    writeU32(out, block.header.version);
    out.insert(out.end(), block.header.previous_hash.begin(), block.header.previous_hash.end());
    out.insert(out.end(), block.header.merkle_root.begin(), block.header.merkle_root.end());
    writeU64(out, block.header.timestamp);
    writeU32(out, block.header.difficulty_target);
    writeU32(out, block.header.nonce);
    writeVarInt(out, block.transactions.size());
    for (const auto& tx : block.transactions)
        serializeTransaction(tx, out);
    return out;
}

bool deserializeBlock(const uint8_t* data, size_t len, Block& block) {
    if (!data || len < 80) return false;
    size_t pos = 0;
    auto read = [&](void* dst, size_t n) {
        if (pos + n > len) return false;
        memcpy(dst, data + pos, n);
        pos += n;
        return true;
    };
    if (!read(&block.header.version, 4)) return false;
    if (!read(block.header.previous_hash.data(), 32)) return false;
    if (!read(block.header.merkle_root.data(), 32)) return false;
    if (!read(&block.header.timestamp, 8)) return false;
    if (!read(&block.header.difficulty_target, 4)) return false;
    if (!read(&block.header.nonce, 4)) return false;
    size_t nTx = 0;
    if (pos >= len) return false;
    if (data[pos] < 0xfd) { nTx = data[pos++]; }
    else if (data[pos] == 0xfd && pos + 3 <= len) {
        nTx = data[pos+1] | (data[pos+2] << 8);
        pos += 3;
    } else if (data[pos] == 0xfe && pos + 5 <= len) {
        nTx = (size_t)data[pos+1] | (data[pos+2] << 8) | (data[pos+3] << 16) | (data[pos+4] << 24);
        pos += 5;
    } else return false;
    block.transactions.clear();
    for (size_t i = 0; i < nTx; ++i) {
        Transaction tx;
        if (!deserializeTransaction(data, len, pos, tx)) return false;
        block.transactions.push_back(std::move(tx));
    }
    return true;
}

Block makeGenesisBlock() {
    Block block;
    block.header.version = 1;
    block.header.previous_hash = {};
    block.header.timestamp = 1704067200; // 2024-01-01 00:00:00 UTC
    block.header.difficulty_target = 0x1d00ffff; // Initial difficulty
    block.header.nonce = 0;
    const char* msg = "Shawn Coin - The Future of Digital Currency 2026";
    Transaction coinbase;
    coinbase.version = 1;
    coinbase.inputs.resize(1);
    coinbase.inputs[0].prev_tx_hash = {};
    coinbase.inputs[0].output_index = 0xffffffffu;
    coinbase.inputs[0].signature.assign((const uint8_t*)msg, (const uint8_t*)msg + strlen(msg));
    coinbase.outputs.resize(1);
    coinbase.outputs[0].amount = getBlockSubsidy(0);
    coinbase.outputs[0].script_pubkey = { 0x76, 0xa9, 0x14 }; // OP_DUP OP_HASH160 PUSH20
    coinbase.outputs[0].script_pubkey.insert(coinbase.outputs[0].script_pubkey.end(), 20, 0);
    coinbase.outputs[0].script_pubkey.push_back(0x88); // OP_EQUALVERIFY OP_CHECKSIG
    coinbase.outputs[0].script_pubkey.push_back(0xac);
    coinbase.lock_time = 0;
    block.transactions.push_back(coinbase);
    block.header.merkle_root = computeMerkleRoot(block.transactions);
    return block;
}

} // namespace shawncoin
