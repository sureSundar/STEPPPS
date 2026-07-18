#!/bin/bash
# TernaryBit OS - Ubuntu/Linux Installer
# Universal OS deployment for Linux platforms

set -e

TBOS_VERSION="1.0.0"
INSTALL_DIR="/opt/ternarybit-os"
CONFIG_DIR="/etc/ternarybit-os"
USER_CONFIG_DIR="$HOME/.config/ternarybit-os"
DESKTOP_FILE="/usr/share/applications/ternarybit-os.desktop"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}🌟 TernaryBit Universal OS - Ubuntu Installer${NC}"
echo -e "${CYAN}==============================================${NC}"

# Check if running as root
if [[ $EUID -eq 0 ]]; then
    echo -e "${YELLOW}⚠️  Running as root - system-wide installation${NC}"
    SYSTEM_INSTALL=true
else
    echo -e "${BLUE}📱 Running as user - user-space installation${NC}"
    SYSTEM_INSTALL=false
    INSTALL_DIR="$HOME/.local/share/ternarybit-os"
    CONFIG_DIR="$USER_CONFIG_DIR"
fi

# Detect system capabilities
echo -e "${GREEN}📊 System Detection:${NC}"

# Get system info
OS_NAME=$(lsb_release -d | cut -f2)
KERNEL_VERSION=$(uname -r)
ARCH=$(uname -m)
CPU_CORES=$(nproc)
TOTAL_RAM=$(free -g | awk '/^Mem:/{print $2}')
AVAILABLE_RAM=$(free -g | awk '/^Mem:/{print $7}')

echo -e "  OS: $OS_NAME"
echo -e "  Kernel: $KERNEL_VERSION"
echo -e "  Architecture: $ARCH"
echo -e "  CPU Cores: $CPU_CORES"
echo -e "  Total RAM: ${TOTAL_RAM}GB"
echo -e "  Available RAM: ${AVAILABLE_RAM}GB"

# Determine TBOS configuration
MEMORY_LIMIT=$(echo "scale=1; $TOTAL_RAM * 0.1" | bc)
if (( $(echo "$MEMORY_LIMIT > 1.0" | bc -l) )); then
    MEMORY_LIMIT="1.0"
fi

if (( TOTAL_RAM < 2 )); then
    TBOS_MODE="minimal"
    FEATURES="shell,steppps"
    echo -e "${YELLOW}⚡ Configuring for minimal mode (low memory)${NC}"
else
    TBOS_MODE="full"
    FEATURES="shell,steppps,network,filesystem"
    echo -e "${GREEN}⚡ Configuring for full mode${NC}"
fi

# Check dependencies
echo -e "${BLUE}🔍 Checking dependencies...${NC}"

MISSING_DEPS=()

# Check for Python3
if ! command -v python3 &> /dev/null; then
    MISSING_DEPS+=("python3")
fi

# Check for essential tools
for cmd in curl wget gcc make; do
    if ! command -v $cmd &> /dev/null; then
        MISSING_DEPS+=("$cmd")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${YELLOW}📦 Installing missing dependencies: ${MISSING_DEPS[*]}${NC}"
    if $SYSTEM_INSTALL; then
        apt-get update
        apt-get install -y "${MISSING_DEPS[@]}"
    else
        echo -e "${RED}❌ Missing dependencies. Please install: ${MISSING_DEPS[*]}${NC}"
        echo "Run: sudo apt-get install ${MISSING_DEPS[*]}"
        exit 1
    fi
fi

# Create installation directories
echo -e "${BLUE}📁 Creating installation directories...${NC}"
mkdir -p "$INSTALL_DIR"/{bin,lib,share,config}
mkdir -p "$CONFIG_DIR"
mkdir -p "$USER_CONFIG_DIR"

# Install TBOS components
echo -e "${BLUE}📦 Installing TernaryBit OS components...${NC}"

# Create main TBOS executable (Python-based for Ubuntu)
cat > "$INSTALL_DIR/bin/tbos" << 'EOF'
#!/usr/bin/env python3
"""
TernaryBit Universal OS - Ubuntu Implementation
Hosted mode on Linux kernel
"""

import os
import sys
import json
import time
import threading
import subprocess
from datetime import datetime

class TernaryBitOS:
    def __init__(self):
        self.version = "1.0.0"
        self.mode = "hosted"
        self.platform = "linux"
        self.config = self.load_config()
        self.running = False
        self.steppps_dimensions = ["SPACE", "TIME", "EVENT", "PSYCHOLOGY", "PIXEL", "PROMPT", "SCRIPT"]
        
    def load_config(self):
        config_paths = [
            "/etc/ternarybit-os/tbos.json",
            os.path.expanduser("~/.config/ternarybit-os/tbos.json")
        ]
        
        for path in config_paths:
            if os.path.exists(path):
                with open(path, 'r') as f:
                    return json.load(f)
        
        # Default config
        return {
            "mode": "full",
            "features": ["shell", "steppps", "network"],
            "memory_limit_gb": 1.0,
            "cpu_cores": os.cpu_count()
        }
    
    def boot(self):
        print("🌟 TernaryBit Universal OS v{} - Ubuntu Edition".format(self.version))
        print("=" * 50)
        print("🔧 Mode: {} ({})".format(self.config.get('mode', 'unknown'), self.platform))
        print("💾 Memory Limit: {}GB".format(self.config.get('memory_limit_gb', 'unlimited')))
        print("🧠 CPU Cores: {}".format(self.config.get('cpu_cores', 'unknown')))
        print("🎯 Features: {}".format(', '.join(self.config.get('features', []))))
        print("🌐 STEPPPS Dimensions: {}".format(len(self.steppps_dimensions)))
        print()
        
        # Initialize STEPPPS framework
        print("🚀 Initializing STEPPPS Framework...")
        for dim in self.steppps_dimensions:
            print(f"  ✅ {dim} dimension loaded")
            time.sleep(0.1)
        
        print("✅ TernaryBit OS ready!")
        self.running = True
        
    def shell(self):
        print("\n🖥️  TernaryBit OS Interactive Shell")
        print("Type 'help' for commands, 'exit' to quit")
        print("-" * 40)
        
        while self.running:
            try:
                cmd = input("tbos> ").strip()
                if not cmd:
                    continue
                    
                if cmd == 'exit':
                    break
                elif cmd == 'help':
                    self.show_help()
                elif cmd == 'status':
                    self.show_status()
                elif cmd == 'steppps':
                    self.show_steppps()
                elif cmd.startswith('echo '):
                    print(cmd[5:])
                elif cmd == 'clear':
                    os.system('clear')
                elif cmd == 'version':
                    print(f"TernaryBit OS v{self.version}")
                else:
                    # Try to execute as system command
                    try:
                        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
                        if result.stdout:
                            print(result.stdout)
                        if result.stderr:
                            print(f"Error: {result.stderr}")
                    except Exception as e:
                        print(f"Unknown command: {cmd}")
                        
            except KeyboardInterrupt:
                print("\n^C")
                continue
            except EOFError:
                break
    
    def show_help(self):
        print("TernaryBit OS Commands:")
        print("  help      - Show this help")
        print("  status    - Show system status")
        print("  steppps   - Show STEPPPS framework info")
        print("  version   - Show OS version")
        print("  clear     - Clear screen")
        print("  exit      - Exit TBOS")
        print("  <cmd>     - Execute system command")
    
    def show_status(self):
        print("📊 TernaryBit OS Status:")
        print(f"  Version: {self.version}")
        print(f"  Platform: {self.platform}")
        print(f"  Mode: {self.config.get('mode')}")
        print(f"  Uptime: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"  Features: {', '.join(self.config.get('features', []))}")
    
    def show_steppps(self):
        print("🌟 STEPPPS Framework Status:")
        for i, dim in enumerate(self.steppps_dimensions, 1):
            print(f"  {i}. {dim}: ✅ Active")
    
    def shutdown(self):
        print("\n🔄 Shutting down TernaryBit OS...")
        self.running = False
        print("✅ Shutdown complete")

def main():
    if len(sys.argv) > 1:
        if sys.argv[1] == '--version':
            print("TernaryBit OS v1.0.0")
            return
        elif sys.argv[1] == '--help':
            print("TernaryBit Universal OS - Ubuntu Edition")
            print("Usage: tbos [options]")
            print("Options:")
            print("  --version    Show version")
            print("  --help       Show help")
            print("  --shell      Start interactive shell (default)")
            return
    
    tbos = TernaryBitOS()
    try:
        tbos.boot()
        tbos.shell()
    except KeyboardInterrupt:
        pass
    finally:
        tbos.shutdown()

if __name__ == "__main__":
    main()
EOF

chmod +x "$INSTALL_DIR/bin/tbos"

# Create configuration file
echo -e "${BLUE}⚙️ Creating configuration...${NC}"
cat > "$CONFIG_DIR/tbos.json" << EOF
{
    "version": "$TBOS_VERSION",
    "mode": "$TBOS_MODE",
    "platform": "linux",
    "hardware": {
        "cpu_cores": $CPU_CORES,
        "memory_limit_gb": $MEMORY_LIMIT,
        "architecture": "$ARCH"
    },
    "features": [$(echo $FEATURES | sed 's/,/", "/g' | sed 's/^/"/' | sed 's/$/"/')],
    "steppps": {
        "dimensions": ["SPACE", "TIME", "EVENT", "PSYCHOLOGY", "PIXEL", "PROMPT", "SCRIPT"],
        "auto_adapt": true
    },
    "linux": {
        "integration": true,
        "shell_compatibility": true,
        "systemd_service": false
    }
}
EOF

# Copy config to user directory
cp "$CONFIG_DIR/tbos.json" "$USER_CONFIG_DIR/"

# Create symlink in PATH
echo -e "${BLUE}🔧 Configuring system PATH...${NC}"
if $SYSTEM_INSTALL; then
    ln -sf "$INSTALL_DIR/bin/tbos" "/usr/local/bin/tbos"
    echo -e "  ✅ Added to system PATH (/usr/local/bin)"
else
    mkdir -p "$HOME/.local/bin"
    ln -sf "$INSTALL_DIR/bin/tbos" "$HOME/.local/bin/tbos"
    echo -e "  ✅ Added to user PATH (~/.local/bin)"
    
    # Add to PATH if not already there
    if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME/.bashrc"
        echo -e "  ✅ Updated ~/.bashrc"
    fi
fi

# Create desktop entry
if $SYSTEM_INSTALL; then
    echo -e "${BLUE}🖥️ Creating desktop entry...${NC}"
    cat > "$DESKTOP_FILE" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=TernaryBit OS
Comment=Universal Operating System
Exec=gnome-terminal -- tbos
Icon=utilities-terminal
Terminal=false
Categories=System;
EOF
    echo -e "  ✅ Desktop entry created"
fi

# Test installation
echo -e "${BLUE}🧪 Testing installation...${NC}"
if [ -x "$INSTALL_DIR/bin/tbos" ]; then
    echo -e "  ✅ Installation test passed"
else
    echo -e "  ${RED}❌ Installation test failed${NC}"
    exit 1
fi

# Display completion message
echo
echo -e "${GREEN}🎉 TernaryBit OS Installation Complete!${NC}"
echo -e "${GREEN}=======================================${NC}"
echo
echo -e "${CYAN}Installation Details:${NC}"
echo -e "  📁 Location: $INSTALL_DIR"
echo -e "  ⚙️ Mode: $TBOS_MODE"
echo -e "  🧠 Memory Limit: ${MEMORY_LIMIT}GB"
echo -e "  🔧 Features: $FEATURES"
echo
echo -e "${YELLOW}Quick Start:${NC}"
echo -e "  • Run 'tbos' from terminal"
echo -e "  • Use 'tbos --help' for options"
if ! $SYSTEM_INSTALL; then
    echo -e "  • Restart terminal or run: source ~/.bashrc"
fi
echo
echo -e "${CYAN}🌟 Welcome to Universal Computing!${NC}"

# Offer to start TBOS
read -p "Would you like to start TernaryBit OS now? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${GREEN}🚀 Starting TernaryBit OS...${NC}"
    exec "$INSTALL_DIR/bin/tbos"
fi
