#include "core/transaction.hpp"
#include "core/types.hpp"
#include <cstring>

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

void serializeTransaction(const Transaction& tx, std::vector<uint8_t>& out) {
    writeU32(out, tx.version);
    writeVarInt(out, tx.inputs.size());
    for (const auto& in : tx.inputs) {
        out.insert(out.end(), in.prev_tx_hash.begin(), in.prev_tx_hash.end());
        writeU32(out, in.output_index);
        writeVarInt(out, in.signature.size());
        out.insert(out.end(), in.signature.begin(), in.signature.end());
        writeVarInt(out, in.pubkey.size());
        out.insert(out.end(), in.pubkey.begin(), in.pubkey.end());
    }
    writeVarInt(out, tx.outputs.size());
    for (const auto& o : tx.outputs) {
        writeU64(out, o.amount);
        writeVarInt(out, o.script_pubkey.size());
        out.insert(out.end(), o.script_pubkey.begin(), o.script_pubkey.end());
    }
    writeU32(out, tx.lock_time);
}

bool deserializeTransaction(const uint8_t* data, size_t len, size_t& pos, Transaction& tx) {
    auto read = [&](void* dst, size_t n) {
        if (pos + n > len) return false;
        memcpy(dst, data + pos, n);
        pos += n;
        return true;
    };
    auto readVarInt = [&](size_t& out) -> bool {
        if (pos >= len) return false;
        if (data[pos] < 0xfd) { out = data[pos++]; return true; }
        if (data[pos] == 0xfd && pos + 3 <= len) {
            out = data[pos+1] | (data[pos+2] << 8);
            pos += 3;
            return true;
        }
        if (data[pos] == 0xfe && pos + 5 <= len) {
            out = (size_t)data[pos+1] | (data[pos+2]<<8) | (data[pos+3]<<16) | (data[pos+4]<<24);
            pos += 5;
            return true;
        }
        return false;
    };
    if (!read(&tx.version, 4)) return false;
    size_t nIn = 0, nOut = 0;
    if (!readVarInt(nIn)) return false;
    tx.inputs.clear();
    for (size_t i = 0; i < nIn; ++i) {
        TxInput in;
        if (!read(in.prev_tx_hash.data(), 32)) return false;
        if (!read(&in.output_index, 4)) return false;
        size_t scriptLen = 0;
        if (!readVarInt(scriptLen) || scriptLen > 10000) return false;
        in.signature.resize(scriptLen);
        if (!read(in.signature.data(), scriptLen)) return false;
        size_t pkLen = 0;
        if (!readVarInt(pkLen) || pkLen > 10000) return false;
        in.pubkey.resize(pkLen);
        if (pkLen && !read(in.pubkey.data(), pkLen)) return false;
        tx.inputs.push_back(std::move(in));
    }
    if (!readVarInt(nOut)) return false;
    tx.outputs.clear();
    for (size_t i = 0; i < nOut; ++i) {
        TxOutput o;
        if (!read(&o.amount, 8)) return false;
        size_t scriptLen = 0;
        if (!readVarInt(scriptLen) || scriptLen > 10000) return false;
        o.script_pubkey.resize(scriptLen);
        if (!read(o.script_pubkey.data(), scriptLen)) return false;
        tx.outputs.push_back(std::move(o));
    }
    if (!read(&tx.lock_time, 4)) return false;
    tx.cached_txid.reset();
    return true;
}

} // namespace shawncoin
