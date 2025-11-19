#!/usr/bin/env bash
# Persona-aware QEMU runner. Uses TBOS_QEMU_FLAGS from config/personas/*.conf

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# shellcheck disable=SC1091
source "$REPO_ROOT/scripts/persona_common.sh"

usage() {
    cat <<EOF
Usage: $0 [--persona NAME] [--disk IMAGE] [--serial-log FILE] [--qemu-bin CMD] [--dry-run] [-- extra qemu args]

Examples:
  $0 --persona calc_4k
  $0 --persona super_1g --serial-log /tmp/super.log -- -monitor stdio
EOF
}

PERSONA="desktop_512m"
DISK_IMAGE="${TBOS_PERSONA_DISK_IMAGE:-}"
SERIAL_LOG=""
QEMU_BIN_OVERRIDE=""
DRY_RUN=0

EXTRA_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --persona)
            shift
            PERSONA="${1:-}"
            if [[ -z "$PERSONA" ]]; then
                echo "Error: --persona requires a value" >&2
                exit 1
            fi
            shift
            ;;
        --disk)
            shift
            DISK_IMAGE="${1:-}"
            if [[ -z "$DISK_IMAGE" ]]; then
                echo "Error: --disk requires a value" >&2
                exit 1
            fi
            shift
            ;;
        --serial-log)
            shift
            SERIAL_LOG="${1:-}"
            if [[ -z "$SERIAL_LOG" ]]; then
                echo "Error: --serial-log requires a value" >&2
                exit 1
            fi
            shift
            ;;
        --qemu-bin)
            shift
            QEMU_BIN_OVERRIDE="${1:-}"
            if [[ -z "$QEMU_BIN_OVERRIDE" ]]; then
                echo "Error: --qemu-bin requires a value" >&2
                exit 1
            fi
            shift
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            EXTRA_ARGS=("$@")
            break
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

persona::load "$PERSONA"

persona::summary_line
echo ""

if [[ -z "$DISK_IMAGE" ]]; then
    for candidate in \
        "$REPO_ROOT/build/tbos_universal.img" \
        "$REPO_ROOT/build/tbos_bare_metal.img" \
        "$REPO_ROOT/build/tbos.img" \
        "$REPO_ROOT/build/tbos_day1.img"
    do
        if [[ -f "$candidate" ]]; then
            DISK_IMAGE="$candidate"
            break
        fi
    done
fi

if [[ -z "$DISK_IMAGE" ]]; then
    echo "No disk image found. Pass --disk <path> or build one (e.g., ./build_universal.sh)." >&2
    exit 1
fi

QEMU_BIN="${QEMU_BIN_OVERRIDE:-${TBOS_QEMU_CMD:-qemu-system-x86_64}}"

if ! command -v "$QEMU_BIN" >/dev/null 2>&1; then
    echo "QEMU binary not found: $QEMU_BIN" >&2
    exit 1
fi

declare -a PERSONA_QEMU_ARGS=()
if [[ -n "${TBOS_QEMU_FLAGS:-}" ]]; then
    # shellcheck disable=SC2206
    PERSONA_QEMU_ARGS=(${TBOS_QEMU_FLAGS})
fi

declare -a SERIAL_ARGS=()
if [[ -n "$SERIAL_LOG" ]]; then
    mkdir -p "$(dirname "$SERIAL_LOG")"
    SERIAL_ARGS=(-serial "file:${SERIAL_LOG}")
fi

CMD=(
    "$QEMU_BIN"
    -name "TBOS-${TBOS_PERSONA_NAME}"
    -drive "file=${DISK_IMAGE},format=raw,if=floppy"
    "${PERSONA_QEMU_ARGS[@]}"
    "${SERIAL_ARGS[@]}"
    "${EXTRA_ARGS[@]}"
)

echo "Disk Image : $DISK_IMAGE"
echo "QEMU Binary: $QEMU_BIN"
echo "Persona    : $TBOS_PERSONA_NAME"
echo ""

if (( DRY_RUN )); then
    echo "[dry-run] ${CMD[*]}"
    exit 0
fi

exec "${CMD[@]}"
