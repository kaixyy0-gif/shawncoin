#include "core/consensus.hpp"
#include "core/block.hpp"
#include "core/blockchain.hpp"
#include "core/types.hpp"
#include "util/util.hpp"
#include "crypto/hash.h"
#include "mining/merkle.hpp"
#include "mining/difficulty.hpp"
#include <algorithm>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace shawncoin {

bool checkProofOfWork(const BlockHeader& header) {
    uint256 h;
    std::vector<uint8_t> buf(80);
    memcpy(buf.data(), &header.version, 4);
    memcpy(buf.data() + 4, header.previous_hash.data(), 32);
    memcpy(buf.data() + 36, header.merkle_root.data(), 32);
    for (int i = 0; i < 8; ++i) buf[68 + i] = (header.timestamp >> (i * 8)) & 0xff;
    memcpy(buf.data() + 72, &header.difficulty_target, 4);
    memcpy(buf.data() + 76, &header.nonce, 4);
    shawncoin_sha256d(buf.data(), 80, h.data());
    // Compact format: byte 0 = size (exponent), bytes 1-3 = mantissa. Target = mantissa * 2^(8*(size-3)).
    uint32_t compact = header.difficulty_target;
    int nSize = (int)(compact >> 24);
    uint32_t nWord = compact & 0x007fffff;
    uint256 target{};
    if (nSize <= 3) {
        nWord >>= 8 * (3 - nSize);
        target[31] = (nWord >> 0) & 0xff;
        target[30] = (nWord >> 8) & 0xff;
        target[29] = (nWord >> 16) & 0xff;
    } else {
        int shift = 8 * (nSize - 3);
        int bytePos = 32 - (shift / 8) - 1;
        if (bytePos >= 0) {
            target[bytePos] = (nWord >> 0) & 0xff;
            if (bytePos > 0) target[bytePos - 1] = (nWord >> 8) & 0xff;
            if (bytePos > 1) target[bytePos - 2] = (nWord >> 16) & 0xff;
        }
    }
    // Hash (LE) must be <= target (LE)
    for (int i = 31; i >= 0; --i) {
        if (h[i] < target[i]) return true;
        if (h[i] > target[i]) return false;
    }
    return true; // equal is valid
}

bool validateBlockStructure(const Block& block) {
    if (block.transactions.empty()) return false;
    if (!block.transactions[0].isCoinbase()) return false;
    uint256 merkle = computeMerkleRoot(block.transactions);
    if (merkle != block.header.merkle_root) return false;
    // Reject blocks with timestamps far in the future (> 2 hours)
    uint64_t now = (uint64_t)std::time(nullptr);
    if (block.header.timestamp > now + 2 * 60 * 60) return false;
    // Validate difficulty target sanity
    uint32_t compact = block.header.difficulty_target;
    uint32_t mant = compact & 0x007fffff;
    if (mant == 0) return false;
    return checkProofOfWork(block.header);
}

bool validateTransactionStructure(const Transaction& tx) {
    if (tx.outputs.empty()) return false;
    uint64_t outSum = 0;
    for (const auto& o : tx.outputs) {
        if (o.amount > MAX_MONEY) return false;
        if (outSum + o.amount < outSum) return false; // overflow
        outSum += o.amount;
    }
    if (outSum > MAX_MONEY) return false;
    if (!tx.isCoinbase() && tx.inputs.empty()) return false;
    return true;
}

bool connectBlockUTXO(const Block& block, UTXOSet& utxo) {
    for (size_t i = 1; i < block.transactions.size(); ++i) {
        const auto& tx = block.transactions[i];
        for (const auto& in : tx.inputs) {
            OutPoint op{ in.prev_tx_hash, in.output_index };
            if (!utxo.has(op)) return false;
            utxo.spend(op);
        }
    }
    for (size_t i = 0; i < block.transactions.size(); ++i) {
        const auto& tx = block.transactions[i];
        uint256 txid = tx.getTxid();
        for (size_t j = 0; j < tx.outputs.size(); ++j) {
            OutPoint op{ txid, (uint32_t)j };
            utxo.put(op, tx.outputs[j].amount, tx.outputs[j].script_pubkey);
        }
    }
    return true;
}

bool checkDifficulty(const Blockchain& chain, const BlockHeader& header) {
    // If chain is too short, accept header as-is
    uint64_t height = chain.getHeight();
    if (height == 0) return true;
    // previous block at 'height'
    auto prevOpt = chain.getBlockByHeight(height);
    if (!prevOpt) return true;
    const Block& prev = *prevOpt;
    uint32_t prevTarget = prev.header.difficulty_target;
    uint64_t nextHeight = height + 1;
    // If this is not a retarget point, difficulty must equal previous target
    if ((nextHeight % DIFFICULTY_INTERVAL) != 0) {
        return header.difficulty_target == prevTarget;
    }
    // Retarget: need first block timestamp from height - (DIFFICULTY_INTERVAL-1)
    if (height < DIFFICULTY_INTERVAL - 1) return header.difficulty_target == prevTarget;
    auto firstOpt = chain.getBlockByHeight(height - (DIFFICULTY_INTERVAL - 1));
    if (!firstOpt) return header.difficulty_target == prevTarget;
    uint64_t lastTime = prev.header.timestamp;
    uint64_t firstTime = (*firstOpt).header.timestamp;
    uint32_t expected = getAdaptiveNextDifficulty(prevTarget, lastTime, firstTime, DIFFICULTY_INTERVAL, nextHeight);
    return header.difficulty_target == expected;
}

void disconnectBlockUTXO(const Block& block, UTXOSet& utxo) {
    for (size_t i = 0; i < block.transactions.size(); ++i) {
        const auto& tx = block.transactions[i];
        uint256 txid = tx.getTxid();
        for (size_t j = 0; j < tx.outputs.size(); ++j) {
            OutPoint op{ txid, (uint32_t)j };
            utxo.spend(op);
        }
    }
    for (size_t i = 1; i < block.transactions.size(); ++i) {
        const auto& tx = block.transactions[i];
        for (const auto& in : tx.inputs) {
            // We need to restore the output - but we don't have script_pubkey/amount here.
            // In a full implementation we'd read from block undo or from previous block state.
            // For simplicity we skip restore (used only for reorg; full node would store undo).
            (void)in;
        }
    }
}


} // namespace shawncoin
