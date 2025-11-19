#!/usr/bin/env bash
# Shared helpers for TBOS persona-aware builds/tests

TBOS_PERSONA_DIR="${TBOS_PERSONA_DIR:-config/personas}"

persona::config_path() {
    local persona="$1"
    echo "${TBOS_PERSONA_DIR}/${persona}.conf"
}

persona::ensure_available() {
    if [[ ! -d "$TBOS_PERSONA_DIR" ]]; then
        echo "Persona directory not found: $TBOS_PERSONA_DIR" >&2
        return 1
    fi
}

persona::load() {
    local persona="$1"
    persona::ensure_available || return 1

    local config_file
    config_file="$(persona::config_path "$persona")"

    if [[ ! -f "$config_file" ]]; then
        echo "Unknown persona '$persona'. Expected file: $config_file" >&2
        return 1
    fi

    # shellcheck disable=SC1090
    source "$config_file"

    export TBOS_PERSONA_NAME
    export TBOS_PERSONA_DESC
    export TBOS_MEM_KB
    export TBOS_PAGE_BYTES
    export TBOS_SCHED_SLICE_MS
    export TBOS_DEVICE_SET
    export TBOS_QEMU_FLAGS
    export TBOS_QEMU_CMD
    export TBOS_BUILD_DEFINES
}

persona::list_available() {
    persona::ensure_available || return 1
    shopt -s nullglob
    local file
    for file in "${TBOS_PERSONA_DIR}"/*.conf; do
        (
            # shellcheck disable=SC1090
            source "$file"
            printf " - %-14s %s\n" "${TBOS_PERSONA_NAME:-$(basename "$file" .conf)}" "${TBOS_PERSONA_DESC:-"(no description)"}"
        )
    done
    shopt -u nullglob
}

persona::artifact_root() {
    local base="${1:-artifacts/personas}"
    local persona="${TBOS_PERSONA_NAME:-default}"
    echo "${base}/${persona}"
}

persona::build_dir() {
    local root="${TBOS_PERSONA_BUILD_ROOT:-build_integrated}"
    local persona="${TBOS_PERSONA_NAME:-default}"
    echo "${root}/${persona}"
}

persona::summary_line() {
    [[ -z "${TBOS_PERSONA_NAME:-}" ]] && return
    printf "%s — %s | mem=%sKB page=%sB slice=%sms devices=%s\n" \
        "$TBOS_PERSONA_NAME" \
        "${TBOS_PERSONA_DESC:-n/a}" \
        "${TBOS_MEM_KB:-?}" \
        "${TBOS_PAGE_BYTES:-?}" \
        "${TBOS_SCHED_SLICE_MS:-?}" \
        "${TBOS_DEVICE_SET:-n/a}"
}
