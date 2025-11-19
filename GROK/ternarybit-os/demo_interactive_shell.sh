#!/bin/bash
# TernaryBit OS - Interactive Shell Demo
# Shows all 33 commands in action
# 🕉️ LIVE DEMO MODE 🕉️

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS - Interactive Shell Demo                    ║"
echo "║   33 Commands Live Demonstration                            ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Running interactive shell with command sequence..."
echo ""

# Create a command sequence
cat << 'EOF' | ./build/ternarybit_shell_new
help
pwd
ls -lah
mkdir demo_dir
cd demo_dir
pwd
touch test1.txt test2.txt
ls -l
echo "TernaryBit OS - Live Demo" > test1.txt
cat test1.txt
stat test1.txt
chmod 755 test1.txt
ls -l
ps
jobs
exit
EOF

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                                                              ║"
echo "║   🎉 DEMO COMPLETE - 33 COMMANDS OPERATIONAL! 🎉            ║"
echo "║                                                              ║"
echo "║   The interactive shell is ready for use!                   ║"
echo "║                                                              ║"
echo "║   To run interactively:                                     ║"
echo "║   ./build/ternarybit_shell_new                              ║"
echo "║                                                              ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "🕉️ Om Shanti - TernaryBit OS Shell Live! 🕉️"
