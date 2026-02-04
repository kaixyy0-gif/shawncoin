# Shawn Coin RPC / API Reference

## Endpoints (REST)

Base URL when running locally: `http://127.0.0.1:7332`

### Blockchain

- **GET /api/v1/blockchain/info**  
  Returns chain name, current block count, best block hash, mempool size.

- **GET /api/v1/block/:hash**  
  Returns block by hash (hex).

- **GET /api/v1/block/height/:height**  
  Returns block by height.

### Transactions

- **GET /api/v1/transaction/:txid**  
  Returns transaction by ID (hex).

- **POST /api/v1/transaction/broadcast**  
  Body: raw transaction hex. Broadcasts to the network.

### Address

- **GET /api/v1/address/:address/balance**  
  Returns balance for address (SHWN).

- **GET /api/v1/address/:address/transactions**  
  Returns list of transactions for address.

### Mining

- **GET /api/v1/mining/info**  
  Returns mining info (difficulty, hashrate, etc.).

### Wallet

- **POST /api/v1/wallet/create**  
  Creates a new wallet (optional params: passphrase).

- **POST /api/v1/wallet/send**  
  Body: `{ "address": "...", "amount": 123 }`. Sends SHWN.

### Network

- **GET /api/v1/network/peers**  
  Returns list of connected peers.

## JSON-RPC 2.0

When RESTinio is enabled, the server also accepts JSON-RPC 2.0 over HTTP.

Example:

```json
{
  "jsonrpc": "2.0",
  "method": "getblockchaininfo",
  "params": [],
  "id": 1
}
```

## Authentication

- RPC can be restricted by `rpcallowip` and protected with `rpcuser` / `rpcpassword` in `shawncoin.conf`.
- API keys and rate limiting can be enabled in configuration.
