#!/usr/bin/env bash
# CI helper to build the universal image for multiple profiles.
# Ensures features like ch-sh remain linked across all variants.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_SCRIPT="$ROOT_DIR/build_universal.sh"

if [[ ! -x "$BUILD_SCRIPT" ]]; then
    echo "error: build_universal.sh not found or not executable" >&2
    exit 1
fi

if [[ $# -gt 0 ]]; then
    PROFILES=("$@")
else
    # Default to universal profile; others can be passed explicitly once ready.
    PROFILES=("universal")
fi

echo "🔁 Running CI universal build for profiles: ${PROFILES[*]}"

for profile in "${PROFILES[@]}"; do
    echo ""
    echo "==> Building profile '$profile'"

    if [[ "$profile" == "host" ]]; then
        echo "    [info] Host profile triggers hosted build and exits after completion."
        "$BUILD_SCRIPT" --profile "$profile"
        continue
    fi

    output_img="$ROOT_DIR/build/tbos_${profile}.img"
    "$BUILD_SCRIPT" --profile "$profile" --output "$output_img"
    echo "    ✔ Image ready: $output_img"
done

echo ""
echo "✅ CI universal build complete"
