#!/bin/bash
# V4.0 Hosted Linux Integration Test
# Traceability: v4.0/ROADMAP.md V4-004, V4-006
#
# Tests the hosted Linux environment with BCB and shell

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
HOSTED_BIN="$PROJECT_ROOT/artifacts/hosted/tbos-linux"

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
echo "  V4.0 Hosted Linux Integration Test"
echo "=============================================="
echo ""

# Test 1: Check binary exists
echo "[TEST] Hosted binary exists"
if [[ -f "$HOSTED_BIN" ]]; then
    test_pass "Binary exists: $HOSTED_BIN"
else
    test_fail "Binary missing: $HOSTED_BIN"
    echo "Run ./build_v4.sh or make build-hosted-linux first"
    exit 1
fi

# Test 2: Check binary is executable
echo "[TEST] Binary is executable"
if [[ -x "$HOSTED_BIN" ]]; then
    test_pass "Binary is executable"
else
    test_fail "Binary is not executable"
    exit 1
fi

# Test 3: Run with commands and capture output
echo "[TEST] Shell execution"
OUTPUT=$(echo -e "bcb\nmemory\nexit" | "$HOSTED_BIN" 2>&1) || true

if [[ -n "$OUTPUT" ]]; then
    test_pass "Shell produced output"
else
    test_fail "No output from shell"
fi

# Test 4: Verify BCB info command works
echo "[TEST] BCB info command"
if echo "$OUTPUT" | grep -qi "bcb\|hosted\|linux\|TBBC"; then
    test_pass "BCB info displayed"
else
    test_fail "BCB info not found in output"
fi

# Test 5: Verify memory command works
echo "[TEST] Memory command"
if echo "$OUTPUT" | grep -qi "memory\|heap\|bytes\|MB"; then
    test_pass "Memory info displayed"
else
    test_fail "Memory info not found in output"
fi

# Test 6: Verify clean exit
echo "[TEST] Clean exit"
EXIT_CODE=0
echo "exit" | "$HOSTED_BIN" >/dev/null 2>&1 || EXIT_CODE=$?
if [[ $EXIT_CODE -eq 0 ]]; then
    test_pass "Clean exit (code 0)"
else
    test_fail "Non-zero exit code: $EXIT_CODE"
fi

# Summary
echo ""
echo "=============================================="
echo "  Results: $passed passed, $failed failed"
echo "=============================================="

if [[ $failed -eq 0 ]]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed${NC}"
    exit 1
fi
