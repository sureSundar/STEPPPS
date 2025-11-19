#!/usr/bin/env bash
# Run the persona build/test matrix and capture logs on an ext4 scratch volume.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# shellcheck disable=SC1091
source "$REPO_ROOT/scripts/persona_common.sh"

DEFAULT_PERSONAS=("calc_4k" "mobile_64k" "desktop_512m" "super_1g")
PERSONAS=("$@")
if [[ ${#PERSONAS[@]} -eq 0 ]]; then
    PERSONAS=("${DEFAULT_PERSONAS[@]}")
fi

ARTIFACT_ROOT="${TBOS_PERSONA_ARTIFACT_ROOT:-/tmp/tbos_persona_ci}"
RUN_ID="${TBOS_PERSONA_RUN_ID:-$(date +%Y%m%d-%H%M%S)}"
RUN_ROOT="$ARTIFACT_ROOT/$RUN_ID"
mkdir -p "$RUN_ROOT"

SUMMARY_FILE="$RUN_ROOT/summary.log"
{
    echo "TBOS Persona CI"
    echo "Run ID     : $RUN_ID"
    echo "Artifact Dir: $RUN_ROOT"
    echo "Personas   : ${PERSONAS[*]}"
    echo ""
} | tee "$SUMMARY_FILE"

pushd "$REPO_ROOT" >/dev/null

for persona in "${PERSONAS[@]}"; do
    echo "==> Persona: $persona" | tee -a "$SUMMARY_FILE"

    if ! persona::load "$persona"; then
        echo "    ✗ persona config missing, skipping" | tee -a "$SUMMARY_FILE"
        continue
    fi

    persona_dir="$RUN_ROOT/$TBOS_PERSONA_NAME"
    mkdir -p "$persona_dir"
    build_dir="$(persona::build_dir)"
    mkdir -p "$build_dir"

    persona::summary_line | tee "$persona_dir/meta.txt" >/dev/null
    {
        printf "PERSONA=%s\n" "$TBOS_PERSONA_NAME"
        printf "DESC=%s\n" "$TBOS_PERSONA_DESC"
        printf "MEM_KB=%s\n" "$TBOS_MEM_KB"
        printf "PAGE_BYTES=%s\n" "$TBOS_PAGE_BYTES"
        printf "SCHED_SLICE_MS=%s\n" "$TBOS_SCHED_SLICE_MS"
        printf "DEVICE_SET=%s\n" "$TBOS_DEVICE_SET"
        printf "QEMU_FLAGS=%s\n" "$TBOS_QEMU_FLAGS"
    } > "$persona_dir/persona.env"

    build_log="$persona_dir/build.log"
    test_log="$persona_dir/test.log"

    if ./build_tbos_system.sh --persona "$TBOS_PERSONA_NAME" >"$build_log" 2>&1; then
        echo "    ✓ build succeeded" | tee -a "$SUMMARY_FILE"
        if [[ -f "$build_dir/tbos" ]]; then
            cp "$build_dir/tbos" "$persona_dir/tbos"
        fi
    else
        echo "    ✗ build failed (see $build_log)" | tee -a "$SUMMARY_FILE"
        continue
    fi

    if TBOS_PERSONA_NAME="$TBOS_PERSONA_NAME" ./test_tbos.sh --persona "$TBOS_PERSONA_NAME" >"$test_log" 2>&1; then
        echo "    ✓ tests passed" | tee -a "$SUMMARY_FILE"
    else
        echo "    ⚠ tests had issues (see $test_log)" | tee -a "$SUMMARY_FILE"
    fi

    if [[ "$TBOS_PERSONA_NAME" == "calc_4k" ]]; then
        smoke_log="$persona_dir/week1_smoke.log"
        if ./tests/smoke/shell_week1.sh >"$smoke_log" 2>&1; then
            echo "    ✓ Week-1 shell smoke passed" | tee -a "$SUMMARY_FILE"
        else
            echo "    ⚠ Week-1 shell smoke failed (see $smoke_log)" | tee -a "$SUMMARY_FILE"
        fi
    fi
done

popd >/dev/null

echo ""
echo "Persona CI artifacts stored under: $RUN_ROOT"
echo "Summary: $SUMMARY_FILE"
