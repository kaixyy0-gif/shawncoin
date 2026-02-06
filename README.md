# ShawnCoin

ShawnCoin is an open-source cryptocurrency project implementing a custom blockchain protocol and related components.

## Overview

This repository contains the core implementation of ShawnCoin, including:

- Node software
- Consensus and chain state management
- Networking and peer-to-peer communication
- Wallet and transaction handling
- Build system and Docker support

The project is written primarily in C++ with CMake as the build system.

## Current Status

- Early development phase
- Active work on core protocol, memory chain state, security improvements, and build stability
- Not yet production-ready

## Repository Structure

- `shawncoin/`          – Main source code directory
- `.github/workflows/`  – GitHub Actions CI/CD pipelines
- `start.sh`            – Helper script to build and/or start the node
- `GETTING_STARTED.md`  – Basic setup and build instructions (recommended first read)
- `.gitignore`          – Standard ignore patterns

## Getting Started

Please read [`GETTING_STARTED.md`](GETTING_STARTED.md) for detailed instructions on:

- Prerequisites (compiler, dependencies, etc.)
- Building the project
- Running a node
- Basic configuration

Quick start (after cloning):

```bash
git clone https://github.com/kaixyy0-gif/shawncoin.git
cd shawncoin
./start.sh
