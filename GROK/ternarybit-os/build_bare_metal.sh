#!/bin/bash
# Compatibility wrapper for legacy usage

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ -t 1 ]]; then
    cat <<'EOF'
[info] build_bare_metal.sh is deprecated.
[info] Use ./build_universal.sh for the canonical build.
EOF
fi

"$ROOT_DIR/build_universal.sh" "$@"
