#!/bin/sh
set -e
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
TBOS_ROOT=$(cd "$SCRIPT_DIR/../../ternarybit-os" && pwd)

( cd "$TBOS_ROOT" && ./scripts/build.sh )
cp "$TBOS_ROOT/build/om.img" "$BINARIES_DIR/om.img"
