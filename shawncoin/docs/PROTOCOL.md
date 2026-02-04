# Shawn Coin P2P Protocol

## Overview

- **Magic bytes**: `0x53 0x48 0x41 0x57` (SHAW, 0x53484157 little-endian).
- **Default port**: 7333 (TCP).
- **Version**: 1.

## Message Format

All messages (except version negotiation) follow:

| Field    | Size  | Description        |
|----------|-------|--------------------|
| magic    | 4     | 0x53484157         |
| command  | 12    | ASCII, null-padded |
| length   | 4     | Payload length     |
| checksum | 4     | First 4 bytes of SHA256(SHA256(payload)) |
| payload  | variable | Message body    |

## Commands

- **version** – Protocol version, block height, node address.
- **verack** – Acknowledgment of version.
- **getaddr** – Request peer addresses.
- **addr** – List of peer addresses.
- **inv** – Inventory (block/tx hashes).
- **getdata** – Request blocks or transactions by hash.
- **block** – Full block data.
- **tx** – Transaction data.
- **ping** / **pong** – Keepalive.
- **getblocks** / **getheaders** – Sync requests.

## Connection Limits

- Maximum 125 outbound and 125 inbound connections (configurable).
- Misbehaving peers can be banned or throttled.
