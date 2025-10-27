#!/bin/bash
# TBOS Interactive Shell Test
# Tests serial port input functionality
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "=========================================="
echo "  TBOS Shell Interactive Test"
echo "  Testing Serial Port Input"
echo "=========================================="
echo

# Create test commands
cat > test_commands.txt << 'EOF'
help
about
calc 5+3
echo Hello TBOS!
om
EOF

echo "[TEST] Sending commands to TBOS shell via serial port..."
echo

# Run QEMU with commands piped in, timeout after 15 seconds
timeout 15 qemu-system-i386 \
    -drive format=raw,file=build/tbos.img \
    -m 128M \
    -serial stdio \
    -display none \
    < test_commands.txt 2>&1 | tee shell_test_output.log

echo
echo "=========================================="
echo "  Test Output Analysis"
echo "=========================================="
echo

# Check for expected output
if grep -q "TBOS INTERACTIVE SHELL" shell_test_output.log; then
    echo "[PASS] Shell started successfully"
else
    echo "[FAIL] Shell did not start"
fi

if grep -q "TBOS Commands" shell_test_output.log; then
    echo "[PASS] Help command processed"
else
    echo "[FAIL] Help command not processed"
fi

if grep -q "Hello TBOS" shell_test_output.log; then
    echo "[PASS] Echo command processed (serial input working!)"
else
    echo "[FAIL] Echo command not processed (serial input may not be working)"
fi

if grep -q "Swamiye Saranam Aiyappa" shell_test_output.log; then
    echo "[PASS] Om command processed"
else
    echo "[FAIL] Om command not processed"
fi

echo
echo "Full output saved to: shell_test_output.log"
echo

# Clean up
rm -f test_commands.txt
