# Mining Shawn Coin (SHWN)

This project includes a simple CPU-friendly miner intended for testing and small private networks. If you want miners to be rewarded directly to your address (like Bitcoin), configure `mineaddr` in your `shawncoin.conf` or pass it via `--mineaddr=`.

Important security notes
- The wallet and mnemonic code in this repository are simplified/stub implementations for educational/testing purposes. They are not BIP39/BIP32/PBKDF2-compliant and should not be used with real funds.
- Always run miners on isolated test networks when experimenting.

Config
Place a `shawncoin.conf` in your data directory (default `~/.shawncoin`) and add under `[mining]`:

```
[mining]
# Enable mining
gen=1
# Number of local CPU threads used
genproclimit=2
# Optional: Base58Check Shawn address to receive coinbase rewards
mineaddr=S...yourAddressHere...
```

How payouts work
- If `mineaddr` is set and valid, the miner will place the 20-byte hash160 of that address into the coinbase output script (P2PKH-style script).
- If `mineaddr` is not set, the miner will use a placeholder (20 zero bytes) — change this before expecting payouts to your wallet.

Running the daemon
Build using CMake/Make and start mining:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./shawncoind --mine
```

Or set the datadir and config:

```bash
./shawncoind --datadir=/path/to/data --mine
```

Deriving an address
Use the provided `tools/wallet_tool` utility to create a wallet and print addresses. See `tools/wallet_tool.cpp` in the repository. Remember: generated mnemonics here are for testing only.

Next steps
- Replace the mnemonic PBKDF2/wordlist with a BIP39-compliant implementation to make wallets interoperable and secure.
- Add RPCs or getblocktemplate support for mining pools (Stratum) if you plan public mining.
- Audit consensus and storage code before mainnet launch.
