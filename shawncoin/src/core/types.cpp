#include "core/types.hpp"
#include "crypto/hash.h"
#include <cstring>
#include <algorithm>

namespace shawncoin {

bool OutPoint::operator==(const OutPoint& o) const {
    return index == o.index && std::memcmp(hash.data(), o.hash.data(), 32) == 0;
}

bool OutPoint::operator<(const OutPoint& o) const {
    int c = std::memcmp(hash.data(), o.hash.data(), 32);
    return c < 0 || (c == 0 && index < o.index);
}

uint256 Transaction::getTxid() const {
    if (cached_txid) return *cached_txid;
    // Serialize for hashing: version, inputs, outputs, lock_time (no sigs)
    std::vector<uint8_t> buf;
    buf.push_back(version & 0xff);
    buf.push_back((version >> 8) & 0xff);
    buf.push_back((version >> 16) & 0xff);
    buf.push_back((version >> 24) & 0xff);
    buf.push_back((uint8_t)inputs.size());
    for (const auto& in : inputs) {
        buf.insert(buf.end(), in.prev_tx_hash.begin(), in.prev_tx_hash.end());
        buf.push_back(in.output_index & 0xff);
        buf.push_back((in.output_index >> 8) & 0xff);
        buf.push_back((in.output_index >> 16) & 0xff);
        buf.push_back((in.output_index >> 24) & 0xff);
    }
    buf.push_back((uint8_t)outputs.size());
    for (const auto& out : outputs) {
        buf.push_back(out.amount & 0xff);
        for (int i = 1; i <= 7; ++i) buf.push_back((out.amount >> (i * 8)) & 0xff);
        buf.push_back((uint8_t)out.script_pubkey.size());
        buf.insert(buf.end(), out.script_pubkey.begin(), out.script_pubkey.end());
    }
    buf.push_back(lock_time & 0xff);
    buf.push_back((lock_time >> 8) & 0xff);
    buf.push_back((lock_time >> 16) & 0xff);
    buf.push_back((lock_time >> 24) & 0xff);
    uint256 txid;
    shawncoin_sha256d(buf.data(), buf.size(), txid.data());
    cached_txid = txid;
    return txid;
}

bool Transaction::isCoinbase() const {
    return inputs.size() == 1 && inputs[0].prev_tx_hash == uint256{} && inputs[0].output_index == 0xffffffffu;
}

uint64_t Transaction::getTotalOutput() const {
    uint64_t sum = 0;
    for (const auto& o : outputs) sum += o.amount;
    return sum;
}

uint64_t Transaction::getTotalInput() const {
    (void)this;
    return 0; // Requires UTXO lookup; caller must sum from UTXO set
}

uint256 Block::getHeaderHash() const {
    std::vector<uint8_t> buf(80);  // Standard 80-byte block header for PoW
    size_t pos = 0;
    buf[pos++] = header.version & 0xff;
    buf[pos++] = (header.version >> 8) & 0xff;
    buf[pos++] = (header.version >> 16) & 0xff;
    buf[pos++] = (header.version >> 24) & 0xff;
    for (size_t i = 0; i < 32; ++i) buf[pos++] = header.previous_hash[i];
    for (size_t i = 0; i < 32; ++i) buf[pos++] = header.merkle_root[i];
    uint32_t ts = (uint32_t)header.timestamp;
    buf[pos++] = ts & 0xff;
    buf[pos++] = (ts >> 8) & 0xff;
    buf[pos++] = (ts >> 16) & 0xff;
    buf[pos++] = (ts >> 24) & 0xff;
    buf[pos++] = header.difficulty_target & 0xff;
    buf[pos++] = (header.difficulty_target >> 8) & 0xff;
    buf[pos++] = (header.difficulty_target >> 16) & 0xff;
    buf[pos++] = (header.difficulty_target >> 24) & 0xff;
    buf[pos++] = header.nonce & 0xff;
    buf[pos++] = (header.nonce >> 8) & 0xff;
    buf[pos++] = (header.nonce >> 16) & 0xff;
    buf[pos++] = (header.nonce >> 24) & 0xff;
    uint256 h;
    shawncoin_sha256d(buf.data(), 80, h.data());
    return h;
}

uint256 Block::getHash() const {
    return getHeaderHash();
}

} // namespace shawncoin
