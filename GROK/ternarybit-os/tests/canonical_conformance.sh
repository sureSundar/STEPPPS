#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
WORK=$(mktemp -d "${TMPDIR:-/tmp}/tbos-canonical.XXXXXX")
CC=${CC:-cc}

cleanup() {
    rm -rf "$WORK"
}
trap cleanup EXIT HUP INT TERM

pass_count=0

step() {
    printf '\n[%s] %s\n' "$1" "$2"
}

pass() {
    pass_count=$((pass_count + 1))
    printf '[PASS] %s\n' "$1"
}

cd "$ROOT"

step 1 "Freestanding public HAL and VFS headers"
if command -v x86_64-elf-gcc >/dev/null 2>&1; then
    FREESTANDING_CC=x86_64-elf-gcc
else
    FREESTANDING_CC=$CC
fi
printf '%s\n' \
    '#include "tbos/hal.h"' \
    '#include "tbos/vfs.h"' \
    'int main(void) { return 0; }' |
    "$FREESTANDING_CC" -std=c11 -ffreestanding -nostdlib \
        -Wall -Wextra -Werror -iquote include -x c -c -o "$WORK/headers.o" -
pass "public headers compile without hosted OS headers"

step 2 "Canonical shell command-provider boundary"
$CC -std=c11 -Wall -Wextra -Werror -I. \
    tests/unit/test_command_provider.c shell/command_provider.c \
    -o "$WORK/test_command_provider"
"$WORK/test_command_provider"
pass "shared shell command provider"

step 3 "STEPPPS v1, v2 and Earth JSON runtime"
$CC -std=c11 -Wall -Wextra -Werror -I. -iquote include \
    tests/unit/test_steppps_json.c steppps/v2/steppps_runtime.c -lm \
    -o "$WORK/test_steppps_json"
"$WORK/test_steppps_json"
pass "canonical STEPPPS entity forms"

step 4 "VFS, RAMFS and STEPPPS policy integration"
$CC -DHOST_BUILD -DTBOS_HOSTED -std=c11 -Wall -Wextra -Werror \
    -iquote include tests/integration/test_steppps_perms.c \
    kernel/fs/vfs.c kernel/fs/ramfs.c kernel/fs/steppps_vfs.c \
    -o "$WORK/test_steppps_perms"
"$WORK/test_steppps_perms"
pass "VFS create/read/chmod with STEPPPS gates"

step 5 "PXFS embedded STEPPPS metadata round-trip"
$CC -D_POSIX_C_SOURCE=200809L -std=c11 -Wall -Wextra -Werror \
    -Isrc/core/filesystem -Ifs tests/unit/test_pxfs_steppps_metadata.c \
    src/core/filesystem/steppps_xattr.c fs/pxfs_steppps.c \
    -o "$WORK/test_pxfs_steppps_metadata"
"$WORK/test_pxfs_steppps_metadata"
pass "PXFS preserves metadata and content losslessly"

step 6 "Offline STEPPPS PROMPT provider"
python3 -m unittest tests.unit.test_steppps_prompt_interaction
pass "offline PROMPT extraction and deterministic provider"

step 7 "Portable headless TBOS desktop"
make -C deploy/hosted BUILD_DIR="$WORK/gui" check
pass "desktop renders dark, light and saffron themes"

printf '\n============================================================\n'
printf 'TBOS canonical conformance: %s/7 gates passed\n' "$pass_count"
printf 'No source files were moved. Sign-off remains a human decision.\n'
printf '============================================================\n'
