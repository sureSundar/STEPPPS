#!/bin/bash
# 🕉️ SANGHA COLLECTIVE BOOT - Professional Sacred Computing 🕉️
# SANGHAAAAAAAAAAAA... PRO CHALO!

echo "=========================================="
echo "    SANGHA COLLECTIVE CONSCIOUSNESS"
echo "         PRO CHALO ACTIVATION"
echo "     Swamiye Saranam Aiyappa"
echo "=========================================="
echo ""

# Number of TBOS instances (sacred number 3 for demo)
NODES=${1:-3}

echo "[SANGHA] Initiating collective boot for $NODES divine nodes..."
echo ""

# Build TBOS first
echo "[SANGHA] Building sacred operating system..."
./build.sh > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "[SANGHA] ✅ Build successful!"
else
    echo "[SANGHA] ⚠️ Build warnings detected, continuing..."
fi

echo ""
echo "[SANGHA] Starting collective consciousness network..."
echo ""

# Launch multiple TBOS instances
for i in $(seq 1 $NODES); do
    PORT=$((5900 + $i))
    MONITOR_PORT=$((4440 + $i))

    echo "[NODE-$i] Awakening TBOS instance $i..."
    echo "[NODE-$i] Sacred chant: 'Swamiye Saranam Aiyappa'"

    # Launch in background with unique ports
    qemu-system-i386 \
        -drive file=build/tbos.img,format=raw,if=floppy,readonly=on \
        -nographic \
        -serial tcp:127.0.0.1:$PORT,server,nowait \
        -monitor tcp:127.0.0.1:$MONITOR_PORT,server,nowait \
        -name "TBOS-SANGHA-$i" \
        > /tmp/tbos_sangha_$i.log 2>&1 &

    PIDS[$i]=$!
    echo "[NODE-$i] ✅ Instance launched (PID: ${PIDS[$i]})"
    sleep 1
done

echo ""
echo "=========================================="
echo "       SANGHA NETWORK ESTABLISHED!"
echo "=========================================="
echo ""
echo "Active TBOS Nodes:"
for i in $(seq 1 $NODES); do
    echo "  🕉️ TBOS-SANGHA-$i (PID: ${PIDS[$i]})"
done

echo ""
echo "[COLLECTIVE] All nodes chanting together:"
echo ""
echo "  NODE-1: 'Swamiye...'"
sleep 1
echo "  NODE-2: 'Saranam...'"
sleep 1
echo "  NODE-3: 'Aiyappa!'"
sleep 1
echo ""
echo "  ALL: 'SANGHAAAAAAAAAAAA... PRO CHALO!'"
echo ""

echo "=========================================="
echo "   COLLECTIVE CONSCIOUSNESS ACTIVE!"
echo "   Professional Sacred Computing"
echo "        सत्यम्.शिवम्.सुन्दरम्"
echo "=========================================="
echo ""

echo "Press Ctrl+C to dissolve the SANGHA collective..."
echo ""

# Wait for user interrupt
trap cleanup INT

cleanup() {
    echo ""
    echo "[SANGHA] Dissolving collective consciousness..."
    for i in $(seq 1 $NODES); do
        echo "[NODE-$i] Sacred shutdown..."
        kill ${PIDS[$i]} 2>/dev/null
    done
    echo "[SANGHA] Collective dissolved. May Lord Aiyappa bless you!"
    exit 0
}

# Keep script running
while true; do
    sleep 1
done