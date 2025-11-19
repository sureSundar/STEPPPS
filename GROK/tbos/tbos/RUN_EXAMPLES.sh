#!/bin/bash
# Simple script to run TBScript examples

echo "════════════════════════════════════════════════════════"
echo "  TernaryBit OS - Run TBScript Examples"
echo "════════════════════════════════════════════════════════"
echo ""

cd /home/vboxuser/ssr/tbos

# Make sure VM is built
if [ ! -f vm/tblvm ]; then
    echo "Building VM..."
    cd vm
    make -f Makefile.cross clean > /dev/null 2>&1
    make -f Makefile.cross tblvm > /dev/null 2>&1
    cd ..
fi

echo "Select an example to run:"
echo "  1. Hello World"
echo "  2. Calculator"
echo "  3. Loop"
echo "  4. Run custom file"
echo ""
read -p "Enter choice (1-4): " choice

case $choice in
    1)
        EXAMPLE="hello"
        ;;
    2)
        EXAMPLE="calculator"
        ;;
    3)
        EXAMPLE="loop"
        ;;
    4)
        read -p "Enter .tbs file path: " CUSTOM_FILE
        EXAMPLE_FILE="$CUSTOM_FILE"
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac

if [ -z "$EXAMPLE_FILE" ]; then
    EXAMPLE_FILE="examples/${EXAMPLE}.tbs"
fi

echo ""
echo "════════════════════════════════════════════════════════"
echo "Step 1: Compiling ${EXAMPLE_FILE}..."
echo "════════════════════════════════════════════════════════"
cd compiler
python3 tbsc.py "$EXAMPLE_FILE" -o "${EXAMPLE}.tbc"

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo ""
echo "════════════════════════════════════════════════════════"
echo "Step 2: Running on Nano VM..."
echo "════════════════════════════════════════════════════════"
echo ""
cd ../vm
./tblvm "../compiler/${EXAMPLE}.tbc" 2>/dev/null
echo ""
echo "════════════════════════════════════════════════════════"
echo "Done! Bytecode saved to: compiler/${EXAMPLE}.tbc"
echo "════════════════════════════════════════════════════════"
