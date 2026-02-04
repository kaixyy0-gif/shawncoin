#!/bin/bash

# 🎯 Shawn Coin Launcher for Non-Technical Users
# Just double-click this file or run: ./start.sh

set -e  # Exit on any error

# Colors for better output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Function to print colored output
print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_header() { echo -e "${PURPLE}🚀 $1${NC}"; }

# Function to check if command exists
command_exists() { command -v "$1" >/dev/null 2>&1; }

print_header "Welcome to Shawn Coin (SHWN)!"
echo ""
print_info "Let me check what's available and help you get started..."
echo ""

# Check if we're in the right place
if [ ! -d "shawncoin" ]; then
    print_error "Can't find the shawncoin folder"
    echo "Please make sure this file is in the main project directory"
    exit 1
fi

print_success "Found project structure!"
echo ""

# Check dependencies
print_info "Checking dependencies..."
missing_deps=()

if ! command_exists gcc; then
    missing_deps+=("gcc")
fi
if ! command_exists g++; then
    missing_deps+=("g++")
fi
if ! command_exists cmake; then
    missing_deps+=("cmake")
fi
if ! command_exists make; then
    missing_deps+=("make")
fi

if [ ${#missing_deps[@]} -ne 0 ]; then
    print_warning "Missing dependencies: ${missing_deps[*]}"
    echo ""
    print_info "Install with:"
    echo "  Ubuntu/Debian: sudo apt install ${missing_deps[*]} libssl-dev pkg-config"
    echo "  CentOS/RHEL:   sudo yum install ${missing_deps[*]} openssl-devel pkgconfig"
    echo "  macOS:        brew install ${missing_deps[*]} openssl pkg-config"
    echo ""
    read -p "Continue anyway? (y/N): " continue_anyway
    if [[ ! "$continue_anyway" =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    print_success "All dependencies found!"
fi

echo ""
print_info "Available options:"
echo "1) 📂 Explore the project files"
echo "2) 🏗️  Build Shawn Coin"
echo "3) ▶️  Run existing build"
echo "4) 📚 View documentation"
echo "5) ⚙️  Configuration help"
echo ""
read -p "Choose an option (1-5): " choice

case $choice in
    1)
        echo ""
        print_info "Exploring project files..."
        cd shawncoin
        echo ""
        print_success "Main project files:"
        ls -la
        echo ""
        print_info "Looking for documentation..."
        if [ -f "README.md" ]; then
            echo "Found README.md! Here's what it says:"
            echo "----------------------------------------"
            cat README.md
            echo "----------------------------------------"
        fi
        if [ -f "EASY_CLI_GUIDE.md" ]; then
            echo ""
            print_success "Found EASY_CLI_GUIDE.md - great for beginners!"
        fi
        echo ""
        print_info "Key directories:"
        echo "  src/        - Source code"
        echo "  docs/       - Documentation"
        echo "  tests/      - Test files"
        echo "  cli/        - Command-line interface"
        ;;
    2)
        echo ""
        print_info "Building Shawn Coin..."
        cd shawncoin
        echo ""
        
        # Check if build directory exists
        if [ -d "build" ]; then
            print_warning "Build directory exists. Cleaning first..."
            rm -rf build
        fi
        
        print_info "Creating build directory..."
        mkdir -p build && cd build
        
        print_info "Configuring with CMake..."
        if cmake .. -DCMAKE_BUILD_TYPE=Release; then
            print_success "CMake configuration successful!"
        else
            print_error "CMake configuration failed!"
            exit 1
        fi
        
        echo ""
        print_info "Compiling (this may take a few minutes)..."
        if make -j$(nproc); then
            print_success "Build completed successfully!"
            echo ""
            print_info "Built executables:"
            ls -la shawncoind shawncoin-cli 2>/dev/null || print_warning "Some executables may not have been built"
            echo ""
            print_success "You can now run Shawn Coin!"
            print_info "Try: ./shawncoind --daemon"
        else
            print_error "Build failed!"
            print_info "Try running: cd .. && make -j$(nproc)"
            exit 1
        fi
        ;;
    3)
        echo ""
        print_info "Looking for existing build..."
        cd shawncoin
        
        if [ -f "build/shawncoind" ] && [ -x "build/shawncoind" ]; then
            print_success "Found existing build in build/ directory"
            echo ""
            print_info "Available executables:"
            ls -la build/shawncoind build/shawncoin-cli 2>/dev/null || print_warning "CLI not built"
            
            echo ""
            print_info "To run Shawn Coin:"
            echo "  cd build"
            echo "  ./shawncoind --daemon"
            echo "  ./shawncoin-cli --help"
        elif [ -f "shawncoind" ] && [ -x "shawncoind" ]; then
            print_success "Found executables in main directory"
            echo ""
            print_info "To run Shawn Coin:"
            echo "  ./shawncoind --daemon"
            echo "  ./shawncoin-cli --help"
        else
            print_warning "No existing build found."
            print_info "Please choose option 2 to build Shawn Coin first."
        fi
        ;;
    4)
        echo ""
        print_info "Available documentation..."
        cd shawncoin
        
        echo ""
        print_success "Documentation files:"
        find docs/ -name "*.md" 2>/dev/null | head -10
        echo ""
        
        if [ -f "README.md" ]; then
            echo "📖 README.md - Project overview"
        fi
        if [ -f "EASY_CLI_GUIDE.md" ]; then
            echo "🎯 EASY_CLI_GUIDE.md - User-friendly guide"
        fi
        if [ -f "WHITEPAPER.md" ]; then
            echo "📄 WHITEPAPER.md - Technical details"
        fi
        
        echo ""
        read -p "View a specific file? (name without extension): " doc_file
        if [ -f "${doc_file}.md" ] || [ -f "docs/${doc_file}.md" ]; then
            if [ -f "docs/${doc_file}.md" ]; then
                doc_file="docs/${doc_file}"
            fi
            echo ""
            print_info "Displaying ${doc_file}.md:"
            echo "----------------------------------------"
            cat "${doc_file}.md"
            echo "----------------------------------------"
        fi
        ;;
    5)
        echo ""
        print_info "Configuration help..."
        cd shawncoin
        
        if [ -f "shawncoin.conf.example" ]; then
            print_success "Found example configuration file!"
            echo ""
            print_info "Configuration steps:"
            echo "1) Copy example: cp shawncoin.conf.example ~/.shawncoin/shawncoin.conf"
            echo "2) Edit with: nano ~/.shawncoin/shawncoin.conf"
            echo ""
            print_warning "Security recommendations:"
            echo "• Change default rpcuser/rpcpassword"
            echo "• Use strong, unique passwords"
            echo "• Restrict rpcallowip to trusted IPs"
            echo ""
            read -p "View example configuration? (y/N): " view_config
            if [[ "$view_config" =~ ^[Yy]$ ]]; then
                echo ""
                cat shawncoin.conf.example
            fi
        else
            print_warning "Configuration example not found"
        fi
        ;;
    *)
        print_error "Invalid choice. Please run this script again and choose 1-5."
        exit 1
        ;;
esac

echo ""
print_success "Done! If you need more help:"
echo "  • Read GETTING_STARTED.md in the main directory"
echo "  • Check shawncoin/docs/ for detailed documentation"
echo "  • Run this script again for other options"
echo ""
print_info "Happy Shawn Coin mining! 🚀💰"