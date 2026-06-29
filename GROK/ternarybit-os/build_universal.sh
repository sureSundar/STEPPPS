#!/bin/bash
# TernaryBit OS - Universal Build Script
# Produces the canonical bootloader → kernel → shell image
# 🕉️ Swamiye Saranam Aiyappa 🕉️
#
# Cross-platform: Works on macOS and Linux

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TBOS_ROOT="$SCRIPT_DIR"
BUILD_DIR="$TBOS_ROOT/build"
BOOT_DIR="$TBOS_ROOT/boot"
TOOLS_DIR="$TBOS_ROOT/tools"

# Detect platform
PLATFORM="$(uname -s)"

# Cross-platform file size function
get_file_size() {
    local file="$1"
    if [[ "$PLATFORM" == "Darwin" ]]; then
        stat -f%z "$file"
    else
        stat -c%s "$file"
    fi
}

# Cross-platform truncate function
create_file_with_size() {
    local file="$1"
    local size="$2"
    if command -v truncate &> /dev/null; then
        truncate -s "$size" "$file"
    else
        # macOS fallback using dd
        dd if=/dev/zero of="$file" bs=1 count=0 seek="$size" 2>/dev/null
    fi
}

# Check for required tools
check_dependencies() {
    local missing=()

    if ! command -v nasm &> /dev/null; then
        missing+=("nasm")
    fi

    if ! command -v gcc &> /dev/null; then
        missing+=("gcc")
    fi

    if [[ "$PLATFORM" == "Darwin" ]]; then
        # macOS needs cross-compiler tools
        if ! command -v x86_64-elf-gcc &> /dev/null && ! command -v x86_64-unknown-linux-gnu-gcc &> /dev/null; then
            # Check if system gcc can do -m64
            if ! gcc -m64 -E -x c /dev/null &> /dev/null 2>&1; then
                missing+=("x86_64-elf-gcc (cross-compiler)")
            fi
        fi
        if ! command -v x86_64-elf-ld &> /dev/null && ! command -v x86_64-unknown-linux-gnu-ld &> /dev/null; then
            if ! ld -m elf_x86_64 --version &> /dev/null 2>&1; then
                missing+=("x86_64-elf-binutils (cross-linker)")
            fi
        fi
    fi

    if [[ ${#missing[@]} -gt 0 ]]; then
        echo "ERROR: Missing required tools: ${missing[*]}" >&2
        echo "" >&2
        if [[ "$PLATFORM" == "Darwin" ]]; then
            echo "Install on macOS:" >&2
            echo "  brew install nasm" >&2
            echo "  brew install x86_64-elf-gcc" >&2
            echo "  brew install x86_64-elf-binutils" >&2
        else
            echo "Install on Linux:" >&2
            echo "  sudo apt install nasm gcc binutils" >&2
        fi
        exit 1
    fi
}

# Detect cross-compiler prefix for macOS
detect_cross_compiler() {
    if [[ "$PLATFORM" == "Darwin" ]]; then
        if command -v x86_64-elf-gcc &> /dev/null; then
            CC_PREFIX="x86_64-elf-"
        elif command -v x86_64-unknown-linux-gnu-gcc &> /dev/null; then
            CC_PREFIX="x86_64-unknown-linux-gnu-"
        else
            CC_PREFIX=""
        fi
    else
        CC_PREFIX=""
    fi

    # Set compiler and linker
    TBOS_CC="${CC_PREFIX}gcc"
    TBOS_LD="${CC_PREFIX}ld"
    TBOS_OBJCOPY="${CC_PREFIX}objcopy"
}

IMAGE_NAME_DEFAULT="tbos_universal.img"
OUTPUT_IMG="$BUILD_DIR/$IMAGE_NAME_DEFAULT"
KERNEL_SOURCE_DEFAULT="kernel/kernel_main.c"
KERNEL_SOURCE="$KERNEL_SOURCE_DEFAULT"
PROFILE="universal"
PROFILE_DIR="$TBOS_ROOT/config"
GENERATED_INCLUDE_DIR="$BUILD_DIR/generated"
LEGACY_IMAGE="$BUILD_DIR/tbos_bare_metal.img"
HOSTED_DIR="$TBOS_ROOT/hosted"
HOSTED_BIN="$TBOS_ROOT/src/shell/universal/tbos_shell"

# Configuration variables (bash 3.x compatible - no associative arrays)
CFG_SHELL="y"
CFG_FS="y"
CFG_VFS="y"
CFG_KEYBOARD="y"
CFG_INTERRUPTS="y"

normalize_value() {
    local raw="$1"
    local fallback="$2"
    # Cross-platform lowercase (works on bash 3.x too)
    local lower
    lower=$(echo "$raw" | tr '[:upper:]' '[:lower:]')
    case "$lower" in
        y|yes|true|1) echo "y" ;;
        n|no|false|0) echo "n" ;;
        *) echo "$fallback" ;;
    esac
}

reset_config() {
    CFG_SHELL="y"
    CFG_FS="y"
    CFG_VFS="y"
    CFG_KEYBOARD="y"
    CFG_INTERRUPTS="y"
}

load_profile() {
    reset_config
    local profile_file="$PROFILE_DIR/profile_${PROFILE}.conf"
    if [[ ! -f "$profile_file" ]]; then
        echo "error: profile '$PROFILE' not found ($profile_file)" >&2
        exit 1
    fi

    # shellcheck disable=SC1090
    source "$profile_file"

    CFG_SHELL=$(normalize_value "${CONFIG_SHELL:-}" "y")
    CFG_FS=$(normalize_value "${CONFIG_FS:-}" "y")
    CFG_VFS=$(normalize_value "${CONFIG_VFS:-}" "$CFG_FS")
    CFG_KEYBOARD=$(normalize_value "${CONFIG_KEYBOARD:-}" "y")
    CFG_INTERRUPTS=$(normalize_value "${CONFIG_INTERRUPTS:-}" "y")
}

config_enabled() {
    local key="$1"
    local val=""
    case "$key" in
        SHELL) val="$CFG_SHELL" ;;
        FS) val="$CFG_FS" ;;
        VFS) val="$CFG_VFS" ;;
        KEYBOARD) val="$CFG_KEYBOARD" ;;
        INTERRUPTS) val="$CFG_INTERRUPTS" ;;
        *) val="y" ;;
    esac
    [[ "$val" == "y" ]]
}

ensure_generated_headers() {
    mkdir -p "$GENERATED_INCLUDE_DIR/tbos"
}

generate_config_header() {
    ensure_generated_headers
    local header="$GENERATED_INCLUDE_DIR/tbos/config.h"
    {
        echo "#pragma once"
        echo "/* Auto-generated by build_universal.sh */"
        echo "#define CONFIG_PROFILE \"$PROFILE\""
        # Generate config defines for each feature
        if config_enabled "SHELL"; then echo "#define CONFIG_SHELL 1"; else echo "#define CONFIG_SHELL 0"; fi
        if config_enabled "FS"; then echo "#define CONFIG_FS 1"; else echo "#define CONFIG_FS 0"; fi
        if config_enabled "VFS"; then echo "#define CONFIG_VFS 1"; else echo "#define CONFIG_VFS 0"; fi
        if config_enabled "KEYBOARD"; then echo "#define CONFIG_KEYBOARD 1"; else echo "#define CONFIG_KEYBOARD 0"; fi
        if config_enabled "INTERRUPTS"; then echo "#define CONFIG_INTERRUPTS 1"; else echo "#define CONFIG_INTERRUPTS 0"; fi
    } > "$header"
}

log_step() {
    echo ""
    echo "==> $1"
}

show_usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --output <path>   Set output disk image (default: build/$IMAGE_NAME_DEFAULT)
  --kernel <path>   Override kernel C source (default: $KERNEL_SOURCE_DEFAULT)
  --profile <name>  Select build profile (config/profile_<name>.conf) (default: $PROFILE)
  -h, --help        Show this help
EOF
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --output)
                shift
                [[ $# -gt 0 ]] || { echo "error: --output requires a value" >&2; exit 1; }
                OUTPUT_IMG="$1"
                shift
                ;;
            --kernel)
                shift
                [[ $# -gt 0 ]] || { echo "error: --kernel requires a value" >&2; exit 1; }
                KERNEL_SOURCE="$1"
                shift
                ;;
            --profile)
                shift
                [[ $# -gt 0 ]] || { echo "error: --profile requires a value" >&2; exit 1; }
                PROFILE="$1"
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                echo "error: unknown option: $1" >&2
                show_usage >&2
                exit 1
                ;;
        esac
    done
}

handle_host_profile() {
    log_step "Hosted profile detected"
    if [[ ! -d "$HOSTED_DIR" ]]; then
        echo "error: hosted tooling not found" >&2
        exit 1
    fi
    "$HOSTED_DIR/build_hosted.sh"
    if [[ -x "$HOSTED_DIR/test_hosted.sh" ]]; then
        "$HOSTED_DIR/test_hosted.sh"
    fi
    echo ""
    echo "Hosted binary available at: $HOSTED_BIN"
    echo "Use 'make hosted' or run the binary directly for interactive sessions."
    exit 0
}

ensure_build_dir() {
    mkdir -p "$BUILD_DIR"
}

build_objects=()

compile_asm() {
    local src="$1"
    local out="$2"
    nasm -f elf32 "$src" -o "$out"
    build_objects+=("$out")
}

compile_c() {
    local src="$1"
    local absolute="$src"
    if [[ "$absolute" != /* ]]; then
        absolute="$TBOS_ROOT/$absolute"
    fi

    local rel="${absolute#$TBOS_ROOT/}"
    local obj_name="${rel//\//_}"
    obj_name="${obj_name%.c}.o"
    local out="$BUILD_DIR/$obj_name"
    local include_flags=(-I"$GENERATED_INCLUDE_DIR" -I"$TBOS_ROOT/include" -I"$TBOS_ROOT/src")

    # Compile as 32-bit (matches stage2_protected.asm bootloader)
    # Use -O0 for kernel_main.c to prevent loop optimization issues with VGA writes
    local opt_level="-O2"
    if [[ "$rel" == "kernel/kernel_main.c" ]]; then
        opt_level="-O0"
    fi
    echo "    [$rel] Compiling 32-bit with $opt_level"

    # Build compiler flags for 32-bit protected mode kernel
    local cflags="-m32 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie -nostdlib -Wall -Wextra"
    if $TBOS_CC -fcf-protection=none -E -x c /dev/null &> /dev/null 2>&1; then
        cflags="$cflags -fcf-protection=none"
    fi

    $TBOS_CC $cflags $opt_level \
        "${include_flags[@]}" \
        -c "$absolute" -o "$out"

    build_objects+=("$out")
}

compile_directory_sources() {
    local rel_dir="$1"
    local label="${2:-$rel_dir}"
    local abs_dir="$TBOS_ROOT/$rel_dir"

    if [[ ! -d "$abs_dir" ]]; then
        echo "    [skip] Directory not found: $rel_dir"
        return
    fi

    local count=0
    while IFS= read -r -d '' file; do
        local rel="${file#$TBOS_ROOT/}"
        compile_c "$rel"
        count=$((count + 1))
    done < <(find "$abs_dir" -type f -name '*.c' -print0 | LC_ALL=C sort -z)

    if [[ "$count" -eq 0 ]]; then
        echo "    [warn] No C sources discovered under $rel_dir"
    else
        echo "    [+] Compiled $count $label source files from $rel_dir/"
    fi
}

assemble_bootloader_stage1() {
    local stage2_sectors="$1"
    nasm -I "$BOOT_DIR" -f bin \
        -D STAGE2_SECTOR_COUNT="$stage2_sectors" \
        "$BOOT_DIR/tbos_sacred.asm" -o "$BUILD_DIR/boot.bin"

    local size
    size=$(get_file_size "$BUILD_DIR/boot.bin")
    if [[ "$size" -ne 512 ]]; then
        echo "error: boot sector must be exactly 512 bytes (got $size)" >&2
        exit 1
    fi
}

assemble_bootloader_stage2() {
    local kernel_start="$1"
    local kernel_sectors="$2"

    # Use 32-bit protected mode bootloader (matches 32-bit kernel)
    nasm -I "$BOOT_DIR" -f bin \
        -D KERNEL_START_SECTOR="$kernel_start" \
        -D KERNEL_SECTORS="$kernel_sectors" \
        "$BOOT_DIR/stage2_protected.asm" -o "$BUILD_DIR/stage2.bin"
}

link_kernel() {
    # Link as 32-bit ELF (matches stage2_protected.asm bootloader)
    $TBOS_LD -m elf_i386 \
        -T "$TBOS_ROOT/kernel/linker.ld" \
        -o "$BUILD_DIR/kernel.elf" \
        "${build_objects[@]}"
}

emit_kernel_binary() {
    $TBOS_OBJCOPY -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel.bin"
    # Cross-platform dd (status=none may not work on all systems)
    dd if="$BUILD_DIR/kernel.bin" of="$BUILD_DIR/kernel_padded.bin" \
        bs=512 conv=sync 2>/dev/null
}

tbpx_roundtrip() {
    local tool="$TOOLS_DIR/tbpx_codec.py"
    [[ -f "$tool" ]] || { echo "    Skipping TBPX step (tools/tbpx_codec.py not found)"; return; }

    if python3 -c "import PIL" >/dev/null 2>&1; then
        format="png"
    else
        format="ppm"
    fi

    local encoded="$BUILD_DIR/kernel_tbpx.$format"
    python3 "$tool" encode "$BUILD_DIR/kernel.bin" "$encoded" \
        --width 256 --repeat-header --format "$format" >/dev/null
    python3 "$tool" decode "$encoded" "$BUILD_DIR/kernel_from_tbpx.bin" >/dev/null

    if cmp -s "$BUILD_DIR/kernel.bin" "$BUILD_DIR/kernel_from_tbpx.bin"; then
        echo "    TBPX roundtrip: OK (kernel.bin -> $format -> bin)"
    else
        echo "error: TBPX roundtrip failed (kernel mismatch after decode)" >&2
        exit 1
    fi
}

create_disk_image() {
    local stage2_sectors="$1"
    local kernel_start="$2"

    local img_size=$((10 * 1024 * 1024))  # 10 MiB
    create_file_with_size "$OUTPUT_IMG" "$img_size"

    dd if="$BUILD_DIR/boot.bin" of="$OUTPUT_IMG" conv=notrunc 2>/dev/null
    dd if="$BUILD_DIR/stage2.bin" of="$OUTPUT_IMG" bs=512 seek=1 conv=notrunc 2>/dev/null
    dd if="$BUILD_DIR/kernel_padded.bin" of="$OUTPUT_IMG" bs=512 seek="$kernel_start" \
        conv=notrunc 2>/dev/null

    if [[ "$OUTPUT_IMG" != "$LEGACY_IMAGE" ]]; then
        cp "$OUTPUT_IMG" "$LEGACY_IMAGE"
    fi
}

report_summary() {
    local stage2_size kernel_size img_bytes
    stage2_size=$(get_file_size "$BUILD_DIR/stage2.bin")
    kernel_size=$(get_file_size "$BUILD_DIR/kernel.bin")
    img_bytes=$(get_file_size "$OUTPUT_IMG")

    echo ""
    echo "Build complete!"
    echo "  Profile     : $PROFILE"
    echo "  Features     : shell=${CFG_SHELL} fs=${CFG_FS} vfs=${CFG_VFS} keyboard=${CFG_KEYBOARD} interrupts=${CFG_INTERRUPTS}"
    echo "  Stage 1      : 512 bytes"
    echo "  Stage 2      : $stage2_size bytes"
    echo "  Kernel       : $kernel_size bytes"
    echo "  Disk image   : $img_bytes bytes ($OUTPUT_IMG)"
    if [[ "$OUTPUT_IMG" != "$LEGACY_IMAGE" ]]; then
        echo "  Legacy alias : $LEGACY_IMAGE"
    fi
    echo ""
    echo "Run with:"
    echo "  qemu-system-x86_64 -drive file=$OUTPUT_IMG,format=raw -m 512 -serial stdio"
    echo ""
}

main() {
    parse_args "$@"

    # Platform checks
    check_dependencies
    detect_cross_compiler

    ensure_build_dir
    load_profile

    if [[ "$PROFILE" == "host" ]]; then
        handle_host_profile
    fi
    generate_config_header

    echo "======================================="
    echo " Building TernaryBit OS (Universal) "
    echo "======================================="
    echo "Platform      : $PLATFORM"
    echo "Compiler      : $TBOS_CC"
    echo "Profile       : $PROFILE"

    log_step "Compiling kernel and drivers"

    build_objects=()
    # Use 32-bit kernel entry (matches stage2_protected.asm)
    compile_asm "$TBOS_ROOT/kernel/kernel_entry.asm" "$BUILD_DIR/kernel_entry.o"
    if config_enabled "INTERRUPTS"; then
        compile_asm "$TBOS_ROOT/kernel/isr.asm" "$BUILD_DIR/isr.o"
    else
        echo "    [skip] Interrupt stubs disabled by profile"
    fi

    compile_c "kernel/hal_baremetal.c"
    compile_c "kernel/tbds.c"
    compile_c "$KERNEL_SOURCE"
    if config_enabled "INTERRUPTS"; then
        compile_c "kernel/interrupt.c"
    else
        echo "    [skip] Interrupt controller disabled by profile"
    fi
    compile_c "kernel/libc.c"
    compile_c "kernel/tbos_baremetal_stubs.c"
    if config_enabled "FS"; then
        compile_c "kernel/fs/ramfs.c"
        compile_c "kernel/fs/vfs.c"
        compile_c "kernel/fs/ucfs_driver.c"
        compile_c "src/core/filesystem/blockdev.c"
        compile_c "src/core/filesystem/ucfs_codec.c"
        compile_c "src/core/filesystem/ucfs_overlay.c"
        compile_c "src/core/filesystem/ucfs_config.c"
    else
        echo "    [skip] VFS/RAMFS disabled by profile"
        compile_c "src/core/filesystem/fs_disabled.c"
    fi
    compile_directory_sources "drivers/vga" "vga driver"
    compile_directory_sources "drivers/timer" "timer driver"
    if config_enabled "KEYBOARD" && config_enabled "INTERRUPTS"; then
        compile_directory_sources "drivers/keyboard" "keyboard driver"
    else
        echo "    [skip] Hardware keyboard driver disabled by profile"
        compile_c "drivers/keyboard/keyboard_stub.c"
    fi
    if config_enabled "SHELL"; then
        compile_directory_sources "shell" "shell"
    else
        echo "    [skip] Interactive shell disabled by profile"
    fi

    log_step "Linking kernel"
    link_kernel
    emit_kernel_binary

    kernel_size=$(get_file_size "$BUILD_DIR/kernel_padded.bin")
    kernel_sectors=$((kernel_size / 512))
    echo "    Kernel size    : $kernel_size bytes"
    echo "    Kernel sectors : $kernel_sectors"

    log_step "Assembling bootloader stages"
    local placeholder_stage2=8
    local placeholder_start=$((1 + placeholder_stage2))
    assemble_bootloader_stage2 "$placeholder_start" "$kernel_sectors"

    stage2_size=$(get_file_size "$BUILD_DIR/stage2.bin")
    stage2_sectors=$(((stage2_size + 511) / 512))
    # Kernel starts after stage2, with some padding for safety
    # Stage2 occupies LBA 1 through (1 + stage2_sectors - 1)
    # Kernel starts at next sector with padding
    kernel_start=$((1 + stage2_sectors + 2))  # +2 sectors padding

    assemble_bootloader_stage2 "$kernel_start" "$kernel_sectors"
    stage2_size=$(get_file_size "$BUILD_DIR/stage2.bin")
    local stage2_sectors_verified=$(((stage2_size + 511) / 512))
    if [[ "$stage2_sectors_verified" -ne "$stage2_sectors" ]]; then
        echo "error: stage2 size mismatch after reassembly (expected $stage2_sectors sectors, got $stage2_sectors_verified)" >&2
        exit 1
    fi
    stage2_sectors="$stage2_sectors_verified"
    assemble_bootloader_stage1 "$stage2_sectors"

    log_step "Creating bootable disk image"
    create_disk_image "$stage2_sectors" "$kernel_start"

    log_step "Pixelizing kernel via TBPX (optional)"
    tbpx_roundtrip

    report_summary
}

main "$@"
