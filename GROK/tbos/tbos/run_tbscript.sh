#!/bin/bash
# TernaryBit OS - TBScript Runner
# Automatically compiles and runs .tbs files

if [ $# -eq 0 ]; then
    echo "TernaryBit OS - TBScript Runner"
    echo "Usage: $0 <file.tbs>"
    echo ""
    echo "Examples:"
    echo "  $0 compiler/examples/hello.tbs"
    echo "  $0 compiler/examples/calculator.tbs"
    echo "  $0 compiler/examples/loop.tbs"
    echo ""
    echo "Available examples:"
    ls -1 compiler/examples/*.tbs 2>/dev/null | sed 's/^/  - /'
    exit 1
fi

SCRIPT_DIR="/home/vboxuser/ssr/tbos"
cd "$SCRIPT_DIR"

TBS_FILE="$1"

# Check if file exists
if [ ! -f "$TBS_FILE" ]; then
    echo "Error: File '$TBS_FILE' not found!"
    exit 1
fi

# Convert to absolute path
TBS_FILE=$(realpath "$TBS_FILE")

# Get filename without extension
BASENAME=$(basename "$TBS_FILE" .tbs)
TBC_FILE="${SCRIPT_DIR}/compiler/${BASENAME}.tbc"

echo "╔════════════════════════════════════════════════════════╗"
echo "║        TernaryBit OS - TBScript Runner                 ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Step 1: Compile
echo "Step 1: Compiling $TBS_FILE..."
echo "----------------------------------------"
cd "${SCRIPT_DIR}/compiler"
python3 tbsc.py "$TBS_FILE" -o "${BASENAME}.tbc"

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Compilation failed!"
    exit 1
fi

echo ""
echo "✅ Compilation successful!"
echo ""

# Step 2: Run
echo "Step 2: Running bytecode on Nano VM..."
echo "----------------------------------------"
cd "${SCRIPT_DIR}/vm"

# Check if VM exists
if [ ! -f tblvm ]; then
    echo "VM not found. Building..."
    make -f Makefile.cross tblvm > /dev/null 2>&1
fi

echo ""
echo "Output:"
echo "========================================"
./tblvm "${TBC_FILE}" 2>/dev/null || ./tblvm "${TBC_FILE}"
EXIT_CODE=$?
echo "========================================"
echo ""

if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ Program completed successfully!"
else
    echo "❌ Program exited with code: $EXIT_CODE"
fi

echo ""
echo "Bytecode saved to: ${TBC_FILE}"
echo ""
