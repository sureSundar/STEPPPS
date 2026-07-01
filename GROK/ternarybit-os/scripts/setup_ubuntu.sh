#!/bin/bash
#
# TBOS Ubuntu Integration Setup
# Sets up Ubuntu environment for TBOS shell escape
#

set -e

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║           TBOS Ubuntu Integration Setup                    ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ -f /etc/os-release ]]; then
        if grep -qi ubuntu /etc/os-release; then
            echo "ubuntu"
        else
            echo "linux"
        fi
    elif grep -qi microsoft /proc/version 2>/dev/null; then
        echo "wsl"
    else
        echo "unknown"
    fi
}

OS=$(detect_os)
echo "Detected OS: $OS"
echo ""

# ═══════════════════════════════════════════════════════════════════════════
# macOS Setup - Use Docker
# ═══════════════════════════════════════════════════════════════════════════
setup_macos() {
    echo "Setting up Ubuntu via Docker on macOS..."
    echo ""

    # Check Docker
    if ! command -v docker &> /dev/null; then
        echo "❌ Docker not found!"
        echo ""
        echo "Please install Docker Desktop:"
        echo "  https://docs.docker.com/desktop/install/mac-install/"
        echo ""
        echo "Or via Homebrew:"
        echo "  brew install --cask docker"
        exit 1
    fi

    # Check Docker running
    if ! docker info &> /dev/null; then
        echo "❌ Docker is not running!"
        echo "   Please start Docker Desktop and try again."
        exit 1
    fi

    echo "✅ Docker is available"
    echo ""

    # Pull Ubuntu image
    echo "Pulling Ubuntu image..."
    docker pull ubuntu:latest

    echo ""
    echo "✅ Ubuntu image ready"
    echo ""

    # Create persistent Ubuntu container (optional)
    read -p "Create persistent Ubuntu container? [y/N] " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Creating persistent container 'tbos-ubuntu-persistent'..."

        # Create volume for persistence
        docker volume create tbos-ubuntu-home 2>/dev/null || true

        # Create container
        docker create -it \
            --name tbos-ubuntu-persistent \
            -v tbos-ubuntu-home:/root \
            -v /:/host:ro \
            ubuntu:latest \
            /bin/bash

        echo "✅ Persistent container created"
        echo ""
        echo "Use: docker start -ai tbos-ubuntu-persistent"
    fi

    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo "Setup complete! In TBOS shell, use:"
    echo "  ubuntu        - Launch Ubuntu shell (Docker)"
    echo "  docker-ubuntu - Explicit Docker Ubuntu"
    echo "═══════════════════════════════════════════════════════════════"
}

# ═══════════════════════════════════════════════════════════════════════════
# Native Ubuntu/Linux Setup
# ═══════════════════════════════════════════════════════════════════════════
setup_ubuntu() {
    echo "Native Ubuntu detected - no additional setup needed!"
    echo ""

    # Check for essential packages
    echo "Checking essential packages..."

    PACKAGES=""

    if ! command -v gcc &> /dev/null; then
        PACKAGES="$PACKAGES build-essential"
    fi

    if ! command -v git &> /dev/null; then
        PACKAGES="$PACKAGES git"
    fi

    if [[ -n "$PACKAGES" ]]; then
        echo "Installing: $PACKAGES"
        sudo apt-get update
        sudo apt-get install -y $PACKAGES
    fi

    echo "✅ System ready"
    echo ""

    # Optional: Install desktop
    read -p "Install Ubuntu Desktop (for 'startx' command)? [y/N] " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Installing Ubuntu Desktop..."
        sudo apt-get install -y ubuntu-desktop
        echo "✅ Desktop installed"
    fi

    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo "Setup complete! In TBOS shell, use:"
    echo "  ubuntu  - Launch bash shell"
    echo "  startx  - Start graphical desktop"
    echo "═══════════════════════════════════════════════════════════════"
}

# ═══════════════════════════════════════════════════════════════════════════
# WSL Setup
# ═══════════════════════════════════════════════════════════════════════════
setup_wsl() {
    echo "WSL detected!"
    echo ""

    # Check WSL version
    if command -v wsl.exe &> /dev/null; then
        echo "WSL Version:"
        wsl.exe --version 2>/dev/null || echo "  (version check unavailable)"
    fi

    echo ""
    echo "For GUI support (WSLg), ensure you have:"
    echo "  - Windows 11 or Windows 10 (build 19041+)"
    echo "  - WSL 2 with latest updates"
    echo ""

    # Optional: Install desktop packages
    read -p "Install GUI packages for WSLg? [y/N] " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Installing GUI packages..."
        sudo apt-get update
        sudo apt-get install -y \
            x11-apps \
            gedit \
            nautilus \
            gnome-terminal
        echo "✅ GUI packages installed"
    fi

    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo "Setup complete! In TBOS shell, use:"
    echo "  ubuntu  - Launch bash shell"
    echo "  startx  - Start GUI apps (with WSLg)"
    echo "═══════════════════════════════════════════════════════════════"
}

# ═══════════════════════════════════════════════════════════════════════════
# Main
# ═══════════════════════════════════════════════════════════════════════════

case $OS in
    macos)
        setup_macos
        ;;
    ubuntu)
        setup_ubuntu
        ;;
    wsl)
        setup_wsl
        ;;
    linux)
        echo "Generic Linux detected."
        echo "Install Docker for Ubuntu container support:"
        echo "  curl -fsSL https://get.docker.com | sh"
        setup_macos  # Use Docker approach
        ;;
    *)
        echo "Unknown OS. Please install Docker manually."
        exit 1
        ;;
esac

echo ""
