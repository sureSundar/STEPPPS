#!/bin/bash
#
# TBOS Bash Integration Profile
# Source this in your .bashrc or .bash_profile:
#   source /path/to/tbos_bash_profile.sh
#
# This adds TBOS features directly to your bash session.
#

# ═══════════════════════════════════════════════════════════════════════════
# TBOS State (shared across shell session)
# ═══════════════════════════════════════════════════════════════════════════

export TBOS_KARMA=${TBOS_KARMA:-100}
export TBOS_CONSCIOUSNESS="Aware"
export TBOS_VERSION="0.9.0"

# ═══════════════════════════════════════════════════════════════════════════
# Helper Functions
# ═══════════════════════════════════════════════════════════════════════════

_tbos_update_consciousness() {
    if [ "$TBOS_KARMA" -gt 150 ]; then
        TBOS_CONSCIOUSNESS="Enlightened"
    elif [ "$TBOS_KARMA" -gt 100 ]; then
        TBOS_CONSCIOUSNESS="Aware"
    elif [ "$TBOS_KARMA" -gt 50 ]; then
        TBOS_CONSCIOUSNESS="Awakening"
    else
        TBOS_CONSCIOUSNESS="Learning"
    fi
}

_tbos_add_karma() {
    local amount=$1
    local reason=$2
    TBOS_KARMA=$((TBOS_KARMA + amount))
    _tbos_update_consciousness
    [ -n "$reason" ] && echo "  +$amount karma: $reason"
}

# ═══════════════════════════════════════════════════════════════════════════
# TBOS Commands
# ═══════════════════════════════════════════════════════════════════════════

# Show TBOS banner
tbos_banner() {
    echo ""
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║                                                               ║"
    echo "║   ████████╗██████╗  ██████╗ ███████╗                          ║"
    echo "║   ╚══██╔══╝██╔══██╗██╔═══██╗██╔════╝                          ║"
    echo "║      ██║   ██████╔╝██║   ██║███████╗                          ║"
    echo "║      ██║   ██╔══██╗██║   ██║╚════██║                          ║"
    echo "║      ██║   ██████╔╝╚██████╔╝███████║                          ║"
    echo "║      ╚═╝   ╚═════╝  ╚═════╝ ╚══════╝                          ║"
    echo "║                                                               ║"
    echo "║            TernaryBit Operating System v$TBOS_VERSION             ║"
    echo "║         Conscious Computing with Compassion                   ║"
    echo "║                                                               ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo ""
}

# Karma status
karma() {
    _tbos_update_consciousness
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  TBOS Karma Status                         ║"
    echo "╠════════════════════════════════════════════╣"
    printf "║  Karma Balance:   %-24s ║\n" "$TBOS_KARMA"
    printf "║  Consciousness:   %-24s ║\n" "$TBOS_CONSCIOUSNESS"
    echo "╚════════════════════════════════════════════╝"
    echo ""
}

# TBOS help (shows both TBOS and hints about bash)
tbos_help() {
    echo ""
    echo "╔═══════════════════════════════════════════════════════════════╗"
    echo "║                 TBOS + Bash Integrated Shell                  ║"
    echo "╠═══════════════════════════════════════════════════════════════╣"
    echo "║  TBOS Commands:                                               ║"
    echo "║    tbos_banner  - Show TBOS banner                            ║"
    echo "║    karma        - Show karma status                           ║"
    echo "║    hostinfo     - Show host system info                       ║"
    echo "║    tbos_help    - This help message                           ║"
    echo "║                                                               ║"
    echo "║  Network Commands:                                            ║"
    echo "║    wow <url>    - Fetch URL via WOW protocol                  ║"
    echo "║    netstat_tbos - Show TBOS network status                    ║"
    echo "║                                                               ║"
    echo "║  Consciousness:                                               ║"
    echo "║    meditate     - Increase karma through mindfulness          ║"
    echo "║    reflect      - Review your computing journey               ║"
    echo "║                                                               ║"
    echo "║  All standard bash commands work normally!                    ║"
    echo "║  (ls, cd, grep, awk, pipes, redirects, scripting, etc.)       ║"
    echo "╚═══════════════════════════════════════════════════════════════╝"
    echo ""
}

# Host system info
hostinfo() {
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  Host System Information                   ║"
    echo "╠════════════════════════════════════════════╣"

    # Detect OS
    local os_name="Unknown"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        os_name="macOS $(sw_vers -productVersion 2>/dev/null || echo '')"
    elif [[ -f /etc/os-release ]]; then
        os_name=$(grep PRETTY_NAME /etc/os-release | cut -d'"' -f2)
    fi

    printf "║  OS:         %-29s ║\n" "$os_name"
    printf "║  Hostname:   %-29s ║\n" "$(hostname)"
    printf "║  User:       %-29s ║\n" "$USER"
    printf "║  Shell:      %-29s ║\n" "$SHELL"
    printf "║  Home:       %-29s ║\n" "$HOME"
    printf "║  TBOS Karma: %-29s ║\n" "$TBOS_KARMA"
    echo "╚════════════════════════════════════════════╝"
    echo ""
}

# WOW protocol fetch
wow() {
    local url="$1"

    if [ -z "$url" ]; then
        echo "Usage: wow <url>"
        echo "Examples:"
        echo "  wow http://example.com"
        echo "  wow https://api.github.com"
        echo "  wow wow://http://example.com"
        return 1
    fi

    # Strip wow:// prefix if present
    url="${url#wow://}"

    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  WOW Protocol Fetch                        ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    echo "Fetching: $url"
    echo ""

    # Use curl with TBOS-style output
    if command -v curl &> /dev/null; then
        local response
        local http_code

        http_code=$(curl -s -o /dev/null -w "%{http_code}" "$url")

        echo "Status: $http_code"
        echo "─────────────────────────────────────────────"

        curl -s "$url" | head -20

        echo ""
        echo "─────────────────────────────────────────────"

        _tbos_add_karma 2 "Network exploration"
    else
        echo "Error: curl not found"
        return 1
    fi
}

# TBOS network status
netstat_tbos() {
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  TBOS Network Status                       ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""

    # Show active connections (simplified)
    if command -v ss &> /dev/null; then
        ss -tuln 2>/dev/null | head -10
    elif command -v netstat &> /dev/null; then
        netstat -an 2>/dev/null | grep -E "^(tcp|udp)" | head -10
    else
        echo "No network tools available"
    fi

    echo ""
    echo "TBOS Network Layer: Active"
    echo "Karma: $TBOS_KARMA | Consciousness: $TBOS_CONSCIOUSNESS"
}

# Meditate command - mindfulness break
meditate() {
    local duration=${1:-5}

    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  TBOS Meditation                           ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    echo "Taking a mindful moment..."
    echo ""

    for i in $(seq 1 $duration); do
        echo -n "  ॐ "
        sleep 1
    done

    echo ""
    echo ""
    echo "Meditation complete."
    _tbos_add_karma 10 "Mindful computing practice"
    echo ""
}

# Reflect on computing journey
reflect() {
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║  TBOS Reflection                           ║"
    echo "╠════════════════════════════════════════════╣"
    echo "║                                            ║"
    echo "║  \"Code with compassion,                    ║"
    echo "║   Debug with patience,                     ║"
    echo "║   Deploy with wisdom.\"                     ║"
    echo "║                                            ║"
    echo "╠════════════════════════════════════════════╣"
    printf "║  Session Karma: %-26s ║\n" "$TBOS_KARMA"
    printf "║  Consciousness: %-26s ║\n" "$TBOS_CONSCIOUSNESS"
    printf "║  Commands Run:  %-26s ║\n" "$(history | wc -l | tr -d ' ')"
    echo "╚════════════════════════════════════════════╝"
    echo ""
}

# ═══════════════════════════════════════════════════════════════════════════
# Enhanced Prompt (optional)
# ═══════════════════════════════════════════════════════════════════════════

# Uncomment to enable TBOS-style prompt
# _tbos_prompt() {
#     local karma_color
#     if [ "$TBOS_KARMA" -gt 100 ]; then
#         karma_color="\033[1;32m"  # Green
#     elif [ "$TBOS_KARMA" -gt 50 ]; then
#         karma_color="\033[1;33m"  # Yellow
#     else
#         karma_color="\033[1;31m"  # Red
#     fi
#
#     PS1="\[\033[1;36m\]tbos\[\033[0m\]:\[\033[1;34m\]\w\[\033[0m\] ${karma_color}[$TBOS_KARMA]\[\033[0m\]\$ "
# }
# PROMPT_COMMAND=_tbos_prompt

# ═══════════════════════════════════════════════════════════════════════════
# Startup Message
# ═══════════════════════════════════════════════════════════════════════════

echo ""
echo "TBOS Integration Loaded - Type 'tbos_help' for commands"
echo "Karma: $TBOS_KARMA | Consciousness: $TBOS_CONSCIOUSNESS"
echo ""
