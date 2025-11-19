#!/bin/bash
# TernaryBit OS - Build Interactive Shell
# Compiles all command weeks into one executable
# 🕉️ FULL INTEGRATION MODE 🕉️

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS - Interactive Shell Builder                 ║"
echo "║   Integrating All Command Weeks                             ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Setup
BUILD_DIR="build"
SRC_DIR="src/shell/universal"
STUB_DIR="src/core/stubs"

mkdir -p "$BUILD_DIR"

echo "📦 [1/6] Compiling core shell..."
gcc -c "$SRC_DIR/tbos_universal_shell.c" -o "$BUILD_DIR/tbos_universal_shell.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

echo "📦 [2/6] Compiling main entry point..."
gcc -c "$SRC_DIR/main_shell.c" -o "$BUILD_DIR/main_shell.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

echo "📦 [3/6] Compiling command weeks..."

# Core commands
gcc -c "$SRC_DIR/commands_core.c" -o "$BUILD_DIR/commands_core.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

# Week 1
gcc -c "$SRC_DIR/commands_week1.c" -o "$BUILD_DIR/commands_week1.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

# Week 2
gcc -c "$SRC_DIR/commands_week2.c" -o "$BUILD_DIR/commands_week2.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

# Week 3
gcc -c "$SRC_DIR/commands_week3.c" -o "$BUILD_DIR/commands_week3.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

# Week 4
gcc -c "$SRC_DIR/commands_week4.c" -o "$BUILD_DIR/commands_week4.o" \
    -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
    -Wall -Wextra 2>&1 | grep -v "warning:" || true

# Week 5 (CX completed)
if [ -f "$SRC_DIR/commands_week5.c" ]; then
    echo "  Compiling Week 5..."
    gcc -c "$SRC_DIR/commands_week5.c" -o "$BUILD_DIR/commands_week5.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

# Week 6 (CX completed)
if [ -f "$SRC_DIR/commands_week6.c" ]; then
    echo "  Compiling Week 6..."
    gcc -c "$SRC_DIR/commands_week6.c" -o "$BUILD_DIR/commands_week6.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

# Week 7 (CX completed)
if [ -f "$SRC_DIR/commands_week7.c" ]; then
    echo "  Compiling Week 7..."
    gcc -c "$SRC_DIR/commands_week7.c" -o "$BUILD_DIR/commands_week7.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

# Week 8 (CX completed)
if [ -f "$SRC_DIR/commands_week8.c" ]; then
    echo "  Compiling Week 8..."
    gcc -c "$SRC_DIR/commands_week8.c" -o "$BUILD_DIR/commands_week8.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

# Week 9 (CX completed)
if [ -f "$SRC_DIR/commands_week9.c" ]; then
    echo "  Compiling Week 9..."
    gcc -c "$SRC_DIR/commands_week9.c" -o "$BUILD_DIR/commands_week9.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

# Week 10 (CX completed)
if [ -f "$SRC_DIR/commands_week10.c" ]; then
    echo "  Compiling Week 10..."
    gcc -c "$SRC_DIR/commands_week10.c" -o "$BUILD_DIR/commands_week10.o" \
        -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
        -Wall -Wextra 2>&1 | grep -v "warning:" || true
fi

echo "📦 [4/6] Compiling stubs..."

# Compile stubs
for stub in tbos_ramdisk tbos_hal tbos_consciousness tbos_karma tbos_sangha tbos_boot_stub; do
    if [ -f "$STUB_DIR/${stub}.c" ]; then
        gcc -c "$STUB_DIR/${stub}.c" -o "$BUILD_DIR/${stub}.o" \
            -I src -I src/shell/universal -I src/boot -I src/hal -I src/network/sangha -I src/core/compression \
            -Wall -Wextra 2>&1 | grep -v "warning:" || true
    fi
done

echo "🔗 [5/6] Linking interactive shell..."

# Build link command
LINK_OBJS="$BUILD_DIR/main_shell.o \
    $BUILD_DIR/tbos_universal_shell.o \
    $BUILD_DIR/commands_core.o \
    $BUILD_DIR/commands_week1.o \
    $BUILD_DIR/commands_week2.o \
    $BUILD_DIR/commands_week3.o \
    $BUILD_DIR/commands_week4.o"

# Add additional weeks if compiled
for week in 5 6 7 8 9 10; do
    if [ -f "$BUILD_DIR/commands_week${week}.o" ]; then
        LINK_OBJS="$LINK_OBJS $BUILD_DIR/commands_week${week}.o"
    fi
done

# Add stubs
LINK_OBJS="$LINK_OBJS \
    $BUILD_DIR/tbos_ramdisk.o \
    $BUILD_DIR/tbos_hal.o \
    $BUILD_DIR/tbos_consciousness.o \
    $BUILD_DIR/tbos_karma.o \
    $BUILD_DIR/tbos_sangha.o \
    $BUILD_DIR/tbos_boot_stub.o"

gcc -o "$BUILD_DIR/ternarybit_shell_new" $LINK_OBJS -lm 2>&1 | grep -v "warning:" || true

if [ ! -f "$BUILD_DIR/ternarybit_shell_new" ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ [6/6] Build complete!"
echo ""

# Count commands
WEEK0_COUNT=1   # help
WEEK1_COUNT=8   # ls, cd, pwd, mkdir, rmdir, rm, cp, mv
WEEK2_COUNT=8   # cat, head, tail, more, less, wc, grep, find
WEEK3_COUNT=8   # touch, chmod, chown, chgrp, ln, stat, du, df
WEEK4_COUNT=8   # ps, top, kill, bg, fg, jobs, nice, renice
WEEK5_COUNT=0
WEEK6_COUNT=0
WEEK7_COUNT=0
WEEK8_COUNT=0
WEEK9_COUNT=0
WEEK10_COUNT=0

# Count based on what was compiled
[ -f "$BUILD_DIR/commands_week5.o" ] && WEEK5_COUNT=7
[ -f "$BUILD_DIR/commands_week6.o" ] && WEEK6_COUNT=7
[ -f "$BUILD_DIR/commands_week7.o" ] && WEEK7_COUNT=7
[ -f "$BUILD_DIR/commands_week8.o" ] && WEEK8_COUNT=7
[ -f "$BUILD_DIR/commands_week9.o" ] && WEEK9_COUNT=7
[ -f "$BUILD_DIR/commands_week10.o" ] && WEEK10_COUNT=8

TOTAL_COUNT=$((WEEK0_COUNT + WEEK1_COUNT + WEEK2_COUNT + WEEK3_COUNT + WEEK4_COUNT + WEEK5_COUNT + WEEK6_COUNT + WEEK7_COUNT + WEEK8_COUNT + WEEK9_COUNT + WEEK10_COUNT))

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                                                              ║"
echo "║        🎉 INTERACTIVE SHELL READY! 🎉                       ║"
echo "║                                                              ║"
echo "║   Total Commands: $TOTAL_COUNT/400                                    ║"
echo "║                                                              ║"
echo "║   Week 0 (Core):  1 command  (help)                         ║"
echo "║   Week 1 (Files): 8 commands (ls, cd, pwd...)               ║"
echo "║   Week 2 (Text):  8 commands (cat, grep, find...)           ║"
echo "║   Week 3 (Perms): 8 commands (chmod, touch, stat...)        ║"
echo "║   Week 4 (Proc):  8 commands (ps, top, kill...)             ║"
if [ $WEEK5_COUNT -gt 0 ]; then
echo "║   Week 5 (Archive): 7 commands (diff, cmp, tar...)          ║"
fi
if [ $WEEK6_COUNT -gt 0 ]; then
echo "║   Week 6 (Transform): 7 commands (cut, paste, sort...)      ║"
fi
if [ $WEEK7_COUNT -gt 0 ]; then
echo "║   Week 7 (Layout): 7 commands (expand, column, tac...)      ║"
fi
if [ $WEEK8_COUNT -gt 0 ]; then
echo "║   Week 8 (Binary): 7 commands (strings, hexdump, iconv...)  ║"
fi
if [ $WEEK9_COUNT -gt 0 ]; then
echo "║   Week 9 (Network): 7 commands (ping, curl, dig...)         ║"
fi
if [ $WEEK10_COUNT -gt 0 ]; then
echo "║   Week 10 (SysInfo): 8 commands (uname, date, cal...)       ║"
fi
echo "║                                                              ║"
echo "║   Executable: build/ternarybit_shell_new                    ║"
echo "║                                                              ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "🚀 To run the interactive shell:"
echo "   ./build/ternarybit_shell_new"
echo ""
echo "📖 Try these commands:"
echo "   help              - Show all commands"
echo "   ls -lah          - List files"
echo "   ps               - Show processes"
echo "   stat README.md   - File information"
echo "   exit             - Quit shell"
echo ""
echo "🕉️ Om Shanti - Ready for Interactive Mode! 🕉️"
