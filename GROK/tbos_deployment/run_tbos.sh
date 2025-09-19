#!/bin/bash
# TernaryBit OS Launcher Script
# Universal Operating System - Calculator to Supercomputer

echo "🚀 TernaryBit OS Launcher"
echo "=========================="
echo "Universal OS: Calculator to Supercomputer"
echo "Architecture Support: 8-bit to 64-bit"
echo "Zero Dependencies: Self-contained"
echo ""

# Check Java availability
if ! command -v java &> /dev/null; then
    echo "❌ Java not found. Please install Java 8 or higher."
    exit 1
fi

# Display Java version
echo "☕ Java Version:"
java -version 2>&1 | head -n 1

echo ""
echo "🎯 Starting TernaryBit OS..."
echo "Press Ctrl+C to exit"
echo ""

# Run TernaryBit OS
java -jar TernaryBitOS.jar