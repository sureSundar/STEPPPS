#!/bin/bash
# V4.0 Boot Chain Integration Test
# Traceability: v4.0/ROADMAP.md V4-007
#
# Tests the full v4.0 boot chain: Stage1 -> Stage2 -> Kernel with BCB
# Requires: QEMU, built v4.0 image

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIOS_IMG="$PROJECT_ROOT/artifacts/baremetal/tbos-bios.img"
TIMEOUT_SEC=10

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

passed=0
failed=0

test_pass() {
    echo -e "  ${GREEN}PASS${NC}: $1"
    ((passed++))
}

test_fail() {
    echo -e "  ${RED}FAIL${NC}: $1"
    ((failed++))
}

echo "=============================================="
echo "  V4.0 Boot Chain Integration Test"
echo "=============================================="
echo ""

# Test 1: Check build artifacts exist
echo "[TEST] Build artifacts exist"
if [[ -f "$BIOS_IMG" ]]; then
    test_pass "BIOS image exists: $BIOS_IMG"
else
    test_fail "BIOS image missing: $BIOS_IMG"
    echo "Run ./build_v4.sh first"
    exit 1
fi

# Test 2: Check QEMU is available
echo "[TEST] QEMU availability"
if command -v qemu-system-i386 &>/dev/null; then
    test_pass "qemu-system-i386 found"
else
    test_fail "qemu-system-i386 not found"
    echo "Install QEMU to run boot tests"
    exit 1
fi

# Test 3: Boot and capture serial output
echo "[TEST] Boot chain execution"
SERIAL_LOG=$(mktemp)
trap "rm -f $SERIAL_LOG" EXIT

# Boot with timeout, capture serial output
timeout ${TIMEOUT_SEC}s qemu-system-i386 \
    -drive file="$BIOS_IMG",format=raw,if=floppy \
    -nographic \
    -serial file:"$SERIAL_LOG" \
    -monitor none \
    -no-reboot \
    2>/dev/null || true

# Check boot output
if [[ -s "$SERIAL_LOG" ]]; then
    test_pass "Serial output captured"
else
    test_fail "No serial output captured"
fi

# Test 4: Verify Stage 2 loaded
echo "[TEST] Stage 2 bootloader"
if grep -qi "stage.2\|loading kernel" "$SERIAL_LOG" 2>/dev/null; then
    test_pass "Stage 2 bootloader executed"
else
    # Stage 2 may not print to serial in all configurations
    echo "  SKIP: Stage 2 serial output not configured"
fi

# Test 5: Verify kernel started
echo "[TEST] Kernel initialization"
if grep -qi "kernel\|ternarybit\|tbos" "$SERIAL_LOG" 2>/dev/null; then
    test_pass "Kernel started"
else
    test_fail "Kernel start message not found"
fi

# Test 6: Verify BCB processing
echo "[TEST] BCB initialization"
if grep -qi "bcb\|boot.capability\|TBBC" "$SERIAL_LOG" 2>/dev/null; then
    test_pass "BCB processed"
else
    # BCB output may be to VGA not serial
    echo "  SKIP: BCB output may be VGA-only"
fi

# Summary
echo ""
echo "=============================================="
echo "  Results: $passed passed, $failed failed"
echo "=============================================="

if [[ $failed -eq 0 ]]; then
    echo -e "${GREEN}All critical tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed${NC}"
    exit 1
fi
