#!/bin/bash
# Build TernaryBit OS - Integrated System
# Boot + HAL + Shell + Sangha

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PERSONA_HELPER="$ROOT_DIR/scripts/persona_common.sh"

if [[ -f "$PERSONA_HELPER" ]]; then
    # shellcheck disable=SC1090
    source "$PERSONA_HELPER"
fi

usage() {
    cat <<EOF
Usage: $0 [--persona NAME] [--list-personas]

  --persona NAME     Build using config/personas/NAME.conf (default: desktop_512m)
  --list-personas    Show available persona configs and exit
  -h, --help         Show this help text
EOF
}

REQUESTED_PERSONA="desktop_512m"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --persona)
            shift
            REQUESTED_PERSONA="${1:-}"
            if [[ -z "$REQUESTED_PERSONA" ]]; then
                echo "Error: --persona requires a value" >&2
                exit 1
            fi
            shift
            ;;
        --list-personas)
            if declare -F persona::list_available >/dev/null; then
                persona::list_available
            else
                echo "Persona helpers not available." >&2
            fi
            exit 0
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

if declare -F persona::load >/dev/null; then
    persona::load "$REQUESTED_PERSONA"
else
    TBOS_PERSONA_NAME="default"
    TBOS_PERSONA_DESC="Legacy build without persona helpers"
fi

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║   Building TernaryBit OS - Integrated System                ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║   Persona: ${TBOS_PERSONA_NAME:-default}"
echo "║   Notes  : ${TBOS_PERSONA_DESC:-n/a}"
if [[ -n "${PACKAGES:-}" ]]; then
echo "║   Packages: ${PACKAGES}"
fi
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

CC=gcc
CFLAGS="-Wall -Wextra -std=c11 -I./src -I./include -I./kernel -D_POSIX_C_SOURCE=200809L -DTBOS_INTEGRATED"
LDFLAGS="-lrt -lpthread -lm"

if [[ -n "${TBOS_BUILD_DEFINES:-}" ]]; then
    CFLAGS+=" ${TBOS_BUILD_DEFINES}"
fi
if [[ -n "${TBOS_PACKAGE_DEFINES:-}" ]]; then
    CFLAGS+=" ${TBOS_PACKAGE_DEFINES}"
fi

BUILD_ROOT="./build_integrated"
BUILD_DIR="$BUILD_ROOT/${TBOS_PERSONA_NAME:-default}"

# Create build directory
mkdir -p "$BUILD_DIR"

OBJECTS=()

compile_module() {
    local src="$1"
    local label="$2"
    local obj_name
    obj_name="$(basename "${src}")"
    obj_name="${obj_name%.c}.o"
    local out="$BUILD_DIR/$obj_name"
    echo "  - ${label:-$src}..."
    $CC $CFLAGS -c "$src" -o "$out"
    OBJECTS+=("$out")
}

compile_optional_module() {
    local src="$1"
    local label="$2"
    local obj_name
    obj_name="$(basename "${src}")"
    obj_name="${obj_name%.c}.o"
    local out="$BUILD_DIR/$obj_name"
    echo "  - ${label:-$src}..."
    if $CC $CFLAGS -c "$src" -o "$out" >/dev/null 2>&1; then
        OBJECTS+=("$out")
    else
        echo "    (skipping ${label:-$src} - missing dependencies)"
        rm -f "$out"
    fi
}

echo "Step 1: Compiling Core Components..."

compile_module "src/boot/tbos_minimal_boot.c" "Minimal Boot"
compile_module "src/hal/tbos_hal.c" "Hardware Abstraction Layer"
compile_module "src/core/filesystem/tbos_ramdisk.c" "Ramdisk Filesystem"
compile_module "src/core/filesystem/ucfs_codec.c" "UCFS Codec"
compile_module "src/core/filesystem/ucfs_overlay.c" "UCFS Overlay"
compile_module "src/core/filesystem/pxfs_codec.c" "PXFS Codec"
compile_module "src/core/filesystem/pxfs_overlay.c" "PXFS Overlay"
compile_module "src/core/lifecycle/tbos_lifecycle.c" "Lifecycle Manager"
compile_module "src/core/karma/tbos_karma_ledger.c" "Karma Ledger"
compile_module "kernel/fs/vfs.c" "VFS Core"
compile_module "src/core/shell/tbos_shell.c" "Conscious Shell"
compile_module "src/core/shell/tbos_supershell.c" "Supershell Manager"
compile_module "src/core/shell/tbos_supershell_plugins.c" "Supershell Plugins"
compile_module "src/core/shell/tbos_shell_commands.c" "Shell Commands"
compile_module "src/core/process/tbos_process.c" "Process Manager"
compile_module "src/core/tbos_metrics.c" "Metrics Core"
compile_module "src/core/steppps/tbos_steppps.c" "STEPPPS Engine"
compile_module "src/network/core/tbos_network.c" "Network Stack"
compile_module "src/network/transport/tcp/tbos_tcp.c" "TCP Transport"
compile_module "src/network/protocols/http/tbos_http.c" "HTTP Protocol"
compile_optional_module "src/network/sangha/tbos_sangha.c" "Digital Sangha"

echo ""
echo "Step 2: Creating Main Integration..."

cat > $BUILD_DIR/tbos_main.c <<'EOF'
/**
 * TernaryBit OS - Integrated System
 * Boot -> HAL Init -> Shell
 */

#include "../../src/boot/tbos_minimal_boot.h"
#include "../../src/hal/tbos_hal.h"
#include "../../src/core/shell/tbos_shell.h"
#include "../../src/core/shell/tbos_supershell.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int result;
    (void)argc;
    (void)argv;

    /* Stage 1: Boot */
    printf("\n=== TernaryBit OS Boot Sequence ===\n\n");
    result = tbos_minimal_boot();
    if (result != 0) {
        fprintf(stderr, "Boot failed!\n");
        return 1;
    }

    /* Stage 2: HAL Init */
    printf("\n=== Hardware Abstraction Layer Init ===\n\n");
    result = hal_init();
    if (result != 0) {
        fprintf(stderr, "HAL init failed!\n");
        return 1;
    }

    hal_profile_t profile = hal_get_profile();
    printf("Hardware Profile: %d\n", profile);
    printf("Free Memory: %u bytes\n", hal_get_free_memory());
    printf("\n");

    /* Stage 3: Shell Init & Run */
    printf("=== Conscious Shell Init ===\n\n");
    result = tbos_shell_init();
    if (result != 0) {
        fprintf(stderr, "Shell init failed!\n");
        return 1;
    }

    printf("TernaryBit OS Ready!\n");
    printf("Type 'help' for commands, 'exit' to quit.\n\n");

    tbos_supershell_init();
    const char* persona_id = getenv("TBOS_PERSONA_NAME");
    if (!persona_id || persona_id[0] == '\0') {
        persona_id = "desktop_512m";
    }
    result = tbos_supershell_launch(persona_id);

    printf("\n=== Graceful Shutdown ===\n\n");
    printf("॥ तत् सत् ॥\n\n");

    return result;
}
EOF

compile_module "$BUILD_DIR/tbos_main.c" "Compiling main integration"

echo ""
echo "Step 3: Linking..."

OUTPUT_BINARY="$BUILD_DIR/tbos"
TEMP_BINARY="${OUTPUT_BINARY}.tmp"

"${CC}" "${OBJECTS[@]}" \
   $LDFLAGS \
   -o "$TEMP_BINARY"

mv "$TEMP_BINARY" "$OUTPUT_BINARY"

if [ $? -eq 0 ]; then
    echo ""
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║   ✅ BUILD SUCCESSFUL                                        ║"
    echo "╠══════════════════════════════════════════════════════════════╣"
    echo "║   Executable: $BUILD_DIR/tbos                                ║"
    echo "║                                                              ║"
    echo "║   Run with: ./$BUILD_DIR/tbos                                ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo ""
else
    echo "❌ Build failed"
    exit 1
fi

if [[ -n "${TBOS_PACKAGE_NOTES:-}" ]]; then
    printf "\nPackages applied:%b\n\n" "${TBOS_PACKAGE_NOTES}"
fi
