#!/bin/bash
# TBOS Bootloader - Interactive Test Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️

clear

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Bootloader - Interactive Test           ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

cd /media/sf_vboxshare/lab/STEPPPS/GROK/ternarybit-os

# Check if bootloader image exists
if [ ! -f "build/tbos_hardware_test.img" ]; then
    echo "[INFO] Bootloader not built yet. Building now..."
    echo ""
    ./build_hardware_test.sh
    echo ""
fi

echo "═══════════════════════════════════════════════════════════"
echo " Choose how to run TBOS bootloader:"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "  1) Graphical Window (see the boot process visually)"
echo "  2) Text Mode (serial console, like a server)"
echo "  3) With more RAM (256 MB - see different classification)"
echo "  4) Cancel"
echo ""
read -p "Your choice (1-4): " choice

case $choice in
    1)
        echo ""
        echo "[RUNNING] Opening QEMU graphical window..."
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
        echo "[RUNNING] Text mode (serial console)..."
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
echo " QEMU exited. Test complete!"
echo "═══════════════════════════════════════════════════════════"
