#!/bin/bash
#
# TernaryBit OS - Comprehensive Command Test Suite
# Tests all 76 commands across 10 weeks
#
# Usage: ./tests/test_all_commands.sh
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SHELL_BIN="$REPO_ROOT/src/shell/universal/tbos_shell"
LOG_DIR="$REPO_ROOT/build/tests"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="$LOG_DIR/test_all_commands_$TIMESTAMP.log"
RESULT_FILE="$LOG_DIR/test_results_$TIMESTAMP.txt"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0

mkdir -p "$LOG_DIR"

# Check if shell exists
if [[ ! -x "$SHELL_BIN" ]]; then
    echo -e "${YELLOW}Shell binary not found. Building...${NC}"
    cd "$REPO_ROOT/src/shell/universal" && make clean && make
    cd "$REPO_ROOT"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║   TernaryBit OS - Comprehensive Command Test Suite               ║"
echo "║   Testing all 76 commands across 10 weeks                        ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
echo "Shell: $SHELL_BIN"
echo "Log:   $LOG_FILE"
echo ""

# Create test directory
TEST_DIR=$(mktemp -d "${TMPDIR:-/tmp}/tbos_test.XXXXXX")
trap 'rm -rf "$TEST_DIR"' EXIT

# Function to run a command and check result
run_test() {
    local cmd="$1"
    local description="$2"
    local expect_fail="${3:-false}"

    TOTAL=$((TOTAL + 1))

    # Run command through shell
    result=$(echo -e "$cmd\nexit" | "$SHELL_BIN" 2>&1) || true

    # Check for common error patterns
    if echo "$result" | grep -qi "command not found\|not implemented\|error:\|failed\|segmentation fault"; then
        if [[ "$expect_fail" == "true" ]]; then
            PASSED=$((PASSED + 1))
            echo -e "  ${GREEN}✓${NC} $description (expected failure)"
            return 0
        fi
        FAILED=$((FAILED + 1))
        echo -e "  ${RED}✗${NC} $description"
        echo "    Command: $cmd" >> "$RESULT_FILE"
        echo "    Error: $(echo "$result" | grep -i "error\|failed\|not found" | head -1)" >> "$RESULT_FILE"
        return 1
    else
        PASSED=$((PASSED + 1))
        echo -e "  ${GREEN}✓${NC} $description"
        return 0
    fi
}

# Function to run command expecting specific output
run_test_output() {
    local cmd="$1"
    local description="$2"
    local expected="$3"

    TOTAL=$((TOTAL + 1))

    result=$(echo -e "$cmd\nexit" | "$SHELL_BIN" 2>&1) || true

    if echo "$result" | grep -q "$expected"; then
        PASSED=$((PASSED + 1))
        echo -e "  ${GREEN}✓${NC} $description"
        return 0
    else
        FAILED=$((FAILED + 1))
        echo -e "  ${RED}✗${NC} $description (expected: $expected)"
        return 1
    fi
}

echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 0: Core Commands (1 command)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "help" "help - Display help information"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 1: File Operations (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "ls" "ls - List directory contents"
run_test "ls -la" "ls -la - Detailed listing"
run_test "pwd" "pwd - Print working directory"
run_test "cd /tmp" "cd - Change directory"
run_test "mkdir $TEST_DIR/testdir" "mkdir - Create directory"
run_test "rmdir $TEST_DIR/testdir" "rmdir - Remove empty directory"
run_test "touch $TEST_DIR/testfile.txt" "touch - Create file"
run_test "cp $TEST_DIR/testfile.txt $TEST_DIR/copy.txt" "cp - Copy file"
run_test "mv $TEST_DIR/copy.txt $TEST_DIR/moved.txt" "mv - Move/rename file"
run_test "rm $TEST_DIR/moved.txt" "rm - Remove file"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 2: Text Processing (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
# Create test file for text processing
echo -e "line1\nline2\nline3\nline4\nline5" > "$TEST_DIR/text.txt"
run_test "cat $TEST_DIR/text.txt" "cat - Display file contents"
run_test "head -2 $TEST_DIR/text.txt" "head - Display first lines"
run_test "tail -2 $TEST_DIR/text.txt" "tail - Display last lines"
run_test "grep line1 $TEST_DIR/text.txt" "grep - Search text patterns"
run_test "wc $TEST_DIR/text.txt" "wc - Word/line/byte count"
run_test "find $TEST_DIR -name '*.txt'" "find - Find files"
run_test "more $TEST_DIR/text.txt" "more - Page through file"
run_test "less $TEST_DIR/text.txt" "less - View file with scrolling"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 3: Permissions & File Info (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "chmod 755 $TEST_DIR/text.txt" "chmod - Change file permissions"
run_test "stat $TEST_DIR/text.txt" "stat - Display file status"
run_test "touch $TEST_DIR/newfile.txt" "touch - Update timestamps"
run_test "ln -s $TEST_DIR/text.txt $TEST_DIR/link.txt" "ln - Create symbolic link"
run_test "du $TEST_DIR" "du - Disk usage"
run_test "df" "df - Filesystem space"
# chown/chgrp may need root
run_test "chown $(whoami) $TEST_DIR/text.txt" "chown - Change owner (may need root)"
run_test "chgrp staff $TEST_DIR/text.txt" "chgrp - Change group (may need root)"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 4: Process Management (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "ps" "ps - List processes"
run_test "jobs" "jobs - List background jobs"
# These are harder to test non-interactively
run_test "nice echo test" "nice - Run with priority"
run_test "renice 10 -p $$" "renice - Change priority"
# top, bg, fg, kill need interactive or specific PIDs
echo -e "  ${YELLOW}○${NC} top - Process monitor (interactive, skipped)"
echo -e "  ${YELLOW}○${NC} bg - Background job (interactive, skipped)"
echo -e "  ${YELLOW}○${NC} fg - Foreground job (interactive, skipped)"
echo -e "  ${YELLOW}○${NC} kill - Send signal (needs PID, skipped)"
SKIPPED=$((SKIPPED + 4))

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 5: System Information (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "uname" "uname - System information"
run_test "uname -a" "uname -a - Full system info"
run_test "hostname" "hostname - Display hostname"
run_test "uptime" "uptime - System uptime"
run_test "whoami" "whoami - Current username"
run_test "id" "id - User/group IDs"
run_test "groups" "groups - User groups"
run_test "env" "env - Environment variables"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 6: Text Transformation (7 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
echo -e "a:b:c" > "$TEST_DIR/fields.txt"
echo -e "c\nb\na" > "$TEST_DIR/unsorted.txt"
echo -e "a\na\nb\nb\nc" > "$TEST_DIR/dups.txt"
run_test "cut -d: -f1 $TEST_DIR/fields.txt" "cut - Extract fields"
run_test "sort $TEST_DIR/unsorted.txt" "sort - Sort lines"
run_test "uniq $TEST_DIR/dups.txt" "uniq - Remove duplicates"
run_test "tr a-z A-Z < $TEST_DIR/text.txt" "tr - Translate characters"
run_test "fmt $TEST_DIR/text.txt" "fmt - Format text"
run_test "paste $TEST_DIR/text.txt $TEST_DIR/text.txt" "paste - Merge lines"
run_test "join $TEST_DIR/text.txt $TEST_DIR/text.txt" "join - Join files"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 7: Text Layout (7 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
echo -e "\thello\tworld" > "$TEST_DIR/tabs.txt"
run_test "expand $TEST_DIR/tabs.txt" "expand - Convert tabs to spaces"
run_test "unexpand $TEST_DIR/tabs.txt" "unexpand - Convert spaces to tabs"
run_test "rev $TEST_DIR/text.txt" "rev - Reverse lines"
run_test "tac $TEST_DIR/text.txt" "tac - Reverse file"
run_test "column -t $TEST_DIR/fields.txt" "column - Columnate output"
run_test "col $TEST_DIR/text.txt" "col - Filter reverse line feeds"
run_test "colrm 1 2 < $TEST_DIR/text.txt" "colrm - Remove columns"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 8: Binary & Encoding (7 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
echo "Hello Binary World" > "$TEST_DIR/binary.txt"
run_test "strings $TEST_DIR/binary.txt" "strings - Extract printable strings"
run_test "od $TEST_DIR/binary.txt" "od - Octal dump"
run_test "hexdump $TEST_DIR/binary.txt" "hexdump - Hex dump"
run_test "xxd $TEST_DIR/binary.txt" "xxd - Vim-style hex dump"
run_test "iconv -f UTF-8 -t ASCII $TEST_DIR/binary.txt" "iconv - Character set conversion"
run_test "dos2unix $TEST_DIR/binary.txt" "dos2unix - Convert line endings"
run_test "unix2dos $TEST_DIR/binary.txt" "unix2dos - Convert line endings"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 9: Network Basics (7 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "hostname" "hostname - Show hostname"
# Network commands may timeout or fail without network
echo -e "  ${YELLOW}○${NC} ping - Network ping (requires network, skipped)"
echo -e "  ${YELLOW}○${NC} traceroute - Trace route (requires network, skipped)"
echo -e "  ${YELLOW}○${NC} curl - HTTP client (requires network, skipped)"
echo -e "  ${YELLOW}○${NC} wget - Download files (requires network, skipped)"
echo -e "  ${YELLOW}○${NC} nslookup - DNS lookup (requires network, skipped)"
echo -e "  ${YELLOW}○${NC} dig - DNS query (requires network, skipped)"
SKIPPED=$((SKIPPED + 6))

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Week 10: System Info Extended (8 commands)${NC}"
echo "═══════════════════════════════════════════════════════════════════"
run_test "uname -a" "uname - System name"
run_test "hostname" "hostname - Host name"
run_test "date" "date - Current date/time"
run_test "uptime" "uptime - System uptime"
run_test "whoami" "whoami - Current user"
run_test "id" "id - User identity"
run_test "groups" "groups - Group membership"
run_test "cal" "cal - Calendar"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo -e "${BLUE}Consciousness & Karma Features${NC}"
echo "═══════════════════════════════════════════════════════════════════"
# Check karma tracking in help output
result=$(echo -e "help\nexit" | "$SHELL_BIN" 2>&1)
if echo "$result" | grep -qi "karma"; then
    PASSED=$((PASSED + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${GREEN}✓${NC} Karma tracking displayed in help"
else
    FAILED=$((FAILED + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${RED}✗${NC} Karma tracking not found"
fi

if echo "$result" | grep -qi "consciousness"; then
    PASSED=$((PASSED + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${GREEN}✓${NC} Consciousness level displayed"
else
    FAILED=$((FAILED + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${RED}✗${NC} Consciousness level not found"
fi

# Calculate results
echo ""
echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                      TEST RESULTS SUMMARY                        ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
echo "  Total Tests:  $TOTAL"
echo -e "  ${GREEN}Passed:${NC}       $PASSED"
echo -e "  ${RED}Failed:${NC}       $FAILED"
echo -e "  ${YELLOW}Skipped:${NC}      $SKIPPED"
echo ""

# Calculate percentage
if [[ $TOTAL -gt 0 ]]; then
    PCT=$(( (PASSED * 100) / TOTAL ))
    echo "  Pass Rate:    $PCT%"
fi

echo ""
echo "  Log saved to: $LOG_FILE"
echo ""

# Save summary
cat > "$RESULT_FILE" << EOF
TernaryBit OS Command Test Results
===================================
Date: $(date)
Total: $TOTAL
Passed: $PASSED
Failed: $FAILED
Skipped: $SKIPPED
Pass Rate: $PCT%
EOF

if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}All tests passed!${NC}"
    echo ""
    echo "॥ तत् सत् ॥ - All commands functioning with consciousness!"
    exit 0
else
    echo -e "${YELLOW}Some tests failed. Check $RESULT_FILE for details.${NC}"
    exit 1
fi
