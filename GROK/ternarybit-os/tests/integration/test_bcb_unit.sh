#!/bin/bash
# V4.0 BCB Unit Test Wrapper
# Traceability: v4.0/ROADMAP.md V4-007
#
# Wrapper to run BCB unit tests as part of integration suite

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "=============================================="
echo "  V4.0 BCB Unit Test Suite"
echo "=============================================="
echo ""

cd "$PROJECT_ROOT"

# Run BCB unit tests via make target
if make test-bcb-host; then
    echo ""
    echo "BCB unit tests: PASSED"
    exit 0
else
    echo ""
    echo "BCB unit tests: FAILED"
    exit 1
fi
