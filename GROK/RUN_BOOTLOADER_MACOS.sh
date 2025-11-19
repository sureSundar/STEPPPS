#!/bin/bash
# TBOS Bootloader - macOS Interactive Test Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️

clear

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Bootloader - macOS Test                ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/ternarybit-os"

# Check if QEMU is installed
if ! command -v qemu-system-i386 &> /dev/null; then
    echo "[ERROR] QEMU not found!"
    echo ""
    echo "Please install QEMU for macOS:"
    echo "  brew install qemu"
    echo ""
    echo "Or download from: https://www.qemu.org/download/#macos"
    echo ""
    exit 1
fi

# Check if bootloader image exists
if [ ! -f "build/tbos_hardware_test.img" ]; then
    echo "[INFO] Bootloader not built yet. Building now..."
    echo ""
    ./build_hardware_test.sh
    echo ""
fi

echo "═══════════════════════════════════════════════════════════"
echo " Choose how to run TBOS bootloader on macOS:"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "  1) Graphical Window (see the boot process visually)"
echo "  2) Text Mode (serial console, like a server)"
echo "  3) With more RAM (256 MB - see different classification)"
echo "  4) Test on Apple Silicon (if applicable)"
echo "  5) Cancel"
echo ""
read -p "Your choice (1-5): " choice

case $choice in
    1)
        echo ""
        echo "[RUNNING] Opening QEMU graphical window on macOS..."
        echo ""
        echo "You will see:"
        echo "  - BIOS boot sequence"
        echo "  - TBOS banner"
        echo "  - Hardware detection"
        echo "  - Classification results"
        echo ""
        echo "Press Ctrl+Alt+2 to switch to QEMU monitor"
        echo "Type 'quit' in monitor to exit"
        echo ""
        read -p "Press Enter to start..."

        qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
        ;;

    2)
        echo ""
        echo "[RUNNING] Text mode (serial console) on macOS..."
        echo ""
        echo "Press Ctrl+A then X to exit QEMU"
        echo ""
        read -p "Press Enter to start..."

        qemu-system-i386 -fda build/tbos_hardware_test.img -nographic -m 128M
        ;;

    3)
        echo ""
        echo "[RUNNING] With 256 MB RAM (different classification)..."
        echo ""
        echo "This should classify as SERVER or WORKSTATION instead of CLUSTER"
        echo ""
        read -p "Press Enter to start..."

        qemu-system-i386 -fda build/tbos_hardware_test.img -m 256M
        ;;

    4)
        echo ""
        echo "[INFO] Testing on Apple Silicon..."
        echo ""

        # Detect if running on Apple Silicon
        if [[ $(uname -m) == "arm64" ]]; then
            echo "✓ Apple Silicon detected (M1/M2/M3)"
            echo ""
            echo "QEMU will emulate x86 on ARM (slower but works)"
            echo ""
            read -p "Press Enter to start..."

            # On Apple Silicon, QEMU might need hvf acceleration
            qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
        else
            echo "✗ Intel Mac detected"
            echo ""
            echo "Running regular QEMU..."
            read -p "Press Enter to start..."

            qemu-system-i386 -fda build/tbos_hardware_test.img -m 128M
        fi
        ;;

    5)
        echo ""
        echo "Cancelled."
        exit 0
        ;;

    *)
        echo ""
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac

echo ""
echo "═══════════════════════════════════════════════════════════"
echo " QEMU exited. Test complete on macOS!"
echo "═══════════════════════════════════════════════════════════"
