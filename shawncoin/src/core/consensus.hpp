#ifndef SHAWNCOIN_CORE_CONSENSUS_HPP
#define SHAWNCOIN_CORE_CONSENSUS_HPP

#include "core/types.hpp"
#include "core/block.hpp"
#include "core/transaction.hpp"
#include "core/utxo.hpp"
#include <cstdint>
#include <optional>

namespace shawncoin {

/** Check block header proof-of-work (hash below target). */
bool checkProofOfWork(const BlockHeader& header);

/** Check that the header's difficulty target matches expected chain difficulty. */
bool checkDifficulty(const class Blockchain& chain, const BlockHeader& header);

/** Validate block (header, merkle, tx count, coinbase, sizes). Does not validate tx inputs against UTXO. */
bool validateBlockStructure(const Block& block);

/** Validate transaction (basic: inputs/outputs, amounts, scripts). Double-spend checked separately. */
bool validateTransactionStructure(const Transaction& tx);

/** Apply block to UTXO set (spend inputs, add outputs). Returns false if any input missing. */
bool connectBlockUTXO(const Block& block, UTXOSet& utxo);

/** Disconnect block from UTXO set (remove outputs, restore inputs). */
void disconnectBlockUTXO(const Block& block, UTXOSet& utxo);

} // namespace shawncoin

#endif // SHAWNCOIN_CORE_CONSENSUS_HPP
