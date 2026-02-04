# 🚀 Welcome to Shawn Coin (SHWN)!

This guide helps you get started with Shawn Coin, even if you're not technical.

## 📁 What's in this folder?

- **build/** - Contains built/compiled files
- **shawncoin/** - Your main Shawn Coin project files
- **start.sh** - Easy launcher script for beginners

## 🎯 Quick Start (Choose one option below)

### Option 1: 🎬 Easy Start (Recommended for Beginners)
```bash
./start.sh
```
This interactive script will guide you through everything!

### Option 2: 🏗️ Build from Source
```bash
cd shawncoin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./shawncoind --daemon
./shawncoin-cli --help
```

### Option 3: ⚡ Quick Makefile Build
```bash
cd shawncoin
make -j$(nproc)
./shawncoind --datadir=.shawncoin_data
```

### Option 4: 🔧 Install Dependencies First
```bash
# Ubuntu/Debian:
sudo apt update && sudo apt install build-essential cmake libssl-dev pkg-config

# CentOS/RHEL:
sudo yum install gcc-c++ cmake openssl-devel pkgconfig

# macOS (with Homebrew):
brew install cmake openssl pkg-config

# Then proceed with Option 2 or 3
```

## 🔧 Shawn Coin Common Commands

| What you want to do | Command to run |
|---------------------|----------------|
| Start the daemon | `./shawncoind --daemon` |
| Get help | `./shawncoin-cli --help` |
| Create wallet | `./shawncoin-cli getnewaddress` |
| Check balance | `./shawncoin-cli getbalance` |
| Send coins | `./shawncoin-cli sendtoaddress ADDRESS AMOUNT` |
| Start mining | `./shawncoind --mine=1` |
| Stop mining | `./shawncoind --mine=0` |
| Check blockchain info | `./shawncoin-cli getblockchaininfo` |

## ❓ Need Help?

1. **Build Issues?** Try these steps:
   - Install dependencies first (see Option 4)
   - Make sure you're in the shawncoin directory
   - Check CMake version: `cmake --version` (needs 3.16+)

2. **Runtime Issues?** Try these steps:
   - Check if daemon is running: `pgrep shawncoind`
   - Look at logs: `tail -f ~/.shawncoin/debug.log`
   - Verify config: `cat ~/.shawncoin/shawncoin.conf`

3. **Still stuck?** Check for:
   - System requirements: Linux/macOS/Windows with C++17 compiler
   - Disk space: At least 1GB free
   - Memory: Minimum 2GB RAM recommended

## 💡 Tips for Beginners

- 🖱️ **Copy & Paste**: You can copy these commands directly
- 📝 **Case Matters**: `shawncoin` is different from `ShawnCoin`
- 🔄 **Try Again**: If something fails, read the error message carefully
- 📚 **Learn More**: Check the documentation in `shawncoin/docs/`
- 🏠 **Data Directory**: Default is `~/.shawncoin/`
- 🔒 **Security**: Never share your wallet files or private keys

## 🌟 Advanced Topics

Want to dive deeper? Check out:
- `shawncoin/docs/WHITEPAPER.md` - Technical details
- `shawncoin/docs/BUILD.md` - Advanced build options
- `shawncoin/EASY_CLI_GUIDE.md` - User-friendly CLI guide
- `shawncoin/shawncoin.conf.example` - Configuration options

---

**Ready to start?** Begin with Option 1 (./start.sh) for the easiest experience! ⚡