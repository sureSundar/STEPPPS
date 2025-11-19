#!/bin/bash
# Legacy wrapper retained for compatibility with older docs/tools.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ -t 1 ]]; then
    cat <<'EOF'
[info] scripts/build.sh is deprecated.
[info] Delegating to ./build_universal.sh for the canonical build pipeline.
EOF
fi

"$ROOT_DIR/build_universal.sh" "$@"
