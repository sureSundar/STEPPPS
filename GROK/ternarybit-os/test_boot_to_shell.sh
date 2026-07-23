#!/bin/bash
# Proves the canonical image boots and accepts commands in the real shell.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="$ROOT/build/tbos_universal.img"

if [[ "${1:-}" == "--build" || ! -f "$IMAGE" ]]; then
    "$ROOT/build_universal.sh"
fi

command -v qemu-system-i386 >/dev/null 2>&1 || {
    echo "error: qemu-system-i386 is required" >&2
    exit 1
}
command -v expect >/dev/null 2>&1 || {
    echo "error: expect is required" >&2
    exit 1
}

export TBOS_IMAGE="$IMAGE"
expect <<'EXPECT_SCRIPT'
set timeout 30
spawn qemu-system-i386 -drive file=$env(TBOS_IMAGE),format=raw,if=ide -boot c -display none -serial stdio -monitor none -no-reboot
expect {
    "tbos:/> " {}
    timeout { puts stderr "FAIL: shell prompt was not reached"; exit 1 }
    eof { puts stderr "FAIL: QEMU exited before the shell prompt"; exit 1 }
}
send "echo TBOS_BOOT_TO_SHELL_OK\r"
expect {
    "\r\nTBOS_BOOT_TO_SHELL_OK\r\n" {}
    timeout { puts stderr "FAIL: shell did not execute the smoke-test command"; exit 1 }
    eof { puts stderr "FAIL: QEMU exited during the shell command"; exit 1 }
}
send "persona chemos\r"
expect "Persona switched to chemos (118-element ChemOS profile)"
send "filesystem pxfs\r"
expect "Filesystem switched to pxfs"
send "compress /etc/motd\r"
expect "PXFS lossless:"
send "tbvm run /etc/motd\r"
expect "TBVM validated"
send "steppps SPACE TIME\r"
expect "Requested dimensions: SPACE TIME"
send "\003"
expect eof
puts "PASS: bootloader -> kernel -> Enhanced Universal Shell"
EXPECT_SCRIPT
