#!/bin/bash
# Test boot and save screenshot

echo "════════════════════════════════════════════════════════"
echo "  TernaryBit OS - Boot Screenshot Test"
echo "════════════════════════════════════════════════════════"
echo ""

BUILD_DIR="build"

# Create screenshot directory
mkdir -p "${BUILD_DIR}/screenshots"

echo "Starting QEMU and capturing boot screen..."
echo ""

# Start QEMU with VNC and capture screenshot
qemu-system-x86_64 \
    -drive file=${BUILD_DIR}/tbos.img,format=raw \
    -m 16 \
    -display none \
    -vnc :99 \
    -daemonize \
    -pidfile /tmp/qemu-tbos-test.pid

if [ ! -f /tmp/qemu-tbos-test.pid ]; then
    echo "✗ Failed to start QEMU"
    exit 1
fi

echo "✓ QEMU started (PID: $(cat /tmp/qemu-tbos-test.pid))"

# Wait for boot
echo "Waiting for boot (3 seconds)..."
sleep 3

# Take screenshot using VNC
if command -v gvnccapture &> /dev/null; then
    echo "Capturing screenshot with gvnccapture..."
    gvnccapture localhost:99 "${BUILD_DIR}/screenshots/boot.png" 2>/dev/null
    echo "✓ Screenshot saved to: ${BUILD_DIR}/screenshots/boot.png"
elif command -v vncsnapshot &> /dev/null; then
    echo "Capturing screenshot with vncsnapshot..."
    vncsnapshot localhost:99 "${BUILD_DIR}/screenshots/boot.jpg" 2>/dev/null
    echo "✓ Screenshot saved to: ${BUILD_DIR}/screenshots/boot.jpg"
else
    echo "⚠ No VNC capture tool available"
    echo "  Install with: sudo apt-get install gvncviewer"
fi

# Stop QEMU
echo "Stopping QEMU..."
kill $(cat /tmp/qemu-tbos-test.pid) 2>/dev/null
rm /tmp/qemu-tbos-test.pid 2>/dev/null

echo ""
echo "════════════════════════════════════════════════════════"
echo "Boot test complete!"
echo "════════════════════════════════════════════════════════"
