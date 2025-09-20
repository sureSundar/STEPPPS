#!/bin/bash
# TernaryBit OS Universal Launcher
echo "🕉️  TernaryBit OS - Sanatana Dharma Computing"
echo "Platform: $(uname -s) $(uname -m)"
echo ""

# Try Java first
if command -v java >/dev/null 2>&1; then
    echo "☕ Launching Java Universal TBOS..."
    java TernaryBitOS
elif [ -x "./tbos_universal" ]; then
    echo "⚡ Launching Native Universal TBOS..."
    ./tbos_universal
else
    echo "❌ No compatible TBOS implementation found"
    echo "Please install Java or use native binary"
fi
