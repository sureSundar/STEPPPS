#!/bin/bash
# TernaryBit OS - Uninstall Script for Linux/Unix
# Removes TernaryBit OS and all associated files

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default installation paths
INSTALL_PATHS=(
    "/usr/local/bin/tbos"
    "/usr/bin/tbos"
    "$HOME/.local/bin/tbos"
    "$HOME/.tbos"
    "/opt/ternarybit-os"
    "$HOME/ternarybit-os"
)

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo -e "${YELLOW}Running as root. Will attempt to remove system-wide installation.${NC}"
    INSTALL_PATHS+=("/etc/tbos" "/var/lib/tbos")
fi

# Function to check and remove files/directories
remove_if_exists() {
    if [ -e "$1" ] || [ -L "$1" ]; then
        echo -e "${YELLOW}Removing: $1${NC}"
        rm -rf "$1"
        if [ $? -ne 0 ]; then
            echo -e "${RED}Failed to remove: $1${NC}"
            return 1
        fi
    fi
    return 0
}

# Function to remove from PATH in shell config
remove_from_path() {
    local shell_rc=""
    
    case $SHELL in
        */zsh)
            shell_rc="$HOME/.zshrc"
            ;;
        */bash)
            shell_rc="$HOME/.bashrc"
            ;;
        */fish)
            shell_rc="$HOME/.config/fish/config.fish"
            ;;
        *)
            shell_rc="$HOME/.profile"
            ;;
    esac
    
    if [ -f "$shell_rc" ]; then
        echo -e "${YELLOW}Updating $shell_rc to remove TernaryBit OS from PATH${NC}"
        sed -i.bak '\|# TernaryBit OS PATH|d' "$shell_rc"
        sed -i.bak '/export PATH.*\/tbos\/bin/d' "$shell_rc"
    fi
}

# Main uninstallation process
echo -e "${GREEN}=== TernaryBit OS Uninstaller ===${NC}"

echo -e "${YELLOW}Stopping any running TernaryBit OS processes...${NC}"
pkill -f tbos || true

# Remove installation files
echo -e "${YELLOW}Removing installed files...${NC}"

for path in "${INSTALL_PATHS[@]}"; do
    remove_if_exists "$path"
done

# Remove from PATH
remove_from_path

# Remove desktop entries and icons
DESKTOP_ENTRIES=(
    "$HOME/.local/share/applications/ternarybit-os.desktop"
    "/usr/share/applications/ternarybit-os.desktop"
    "$HOME/.local/share/icons/ternarybit-os.png"
    "/usr/share/icons/ternarybit-os.png"
    "$HOME/Desktop/ternarybit-os.desktop"
)

for entry in "${DESKTOP_ENTRIES[@]}"; do
    remove_if_exists "$entry"
done

# Clean up temporary files
echo -e "${YELLOW}Cleaning up temporary files...${NC}"
rm -rf "/tmp/tbos-*"
rm -rf "/tmp/ternarybit-os-*"

# Update package manager cache if running as root
if [ "$EUID" -eq 0 ]; then
    if command -v apt-get &> /dev/null; then
        apt-get update
    elif command -v dnf &> /dev/null; then
        dnf makecache
    elif command -v yum &> /dev/null; then
        yum makecache
    elif command -v pacman &> /dev/null; then
        pacman -Syu
    fi
fi

echo -e "${GREEN}Uninstallation complete!${NC}"
echo -e "${YELLOW}Please restart your terminal or run 'source ~/.bashrc' (or equivalent) to update your PATH.${NC}"

exit 0
