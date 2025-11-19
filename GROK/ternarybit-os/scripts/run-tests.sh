#!/bin/bash
# Test runner for TernaryBit OS
# Traceability: PRD-049 (Testing Framework)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
VERBOSE=0
COVERAGE=0
TEST_TYPE="all"
TEST_TARGETS=()
BUILD_DIR="${BUILD_DIR:-build/$(uname -m)}"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -c|--coverage)
            COVERAGE=1
            shift
            ;;
        -t|--type)
            TEST_TYPE="$2"
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        -*)
            echo -e "${RED}Error: Unknown option $1${NC}" >&2
            show_help
            exit 1
            ;;
        *)
            TEST_TARGETS+=("$1")
            shift
            ;;
    esac
done

show_help() {
    echo "Usage: $0 [options] [test_targets...]"
    echo "Options:"
    echo "  -v, --verbose    Show verbose output"
    echo "  -c, --coverage   Generate code coverage report"
    echo "  -t, --type TYPE  Run specific test type (unit, integration, all)"
    echo "  -h, --help       Show this help message"
    echo ""
    echo "Test targets can be specific test files or directories"
}

print_header() {
    echo -e "\n${YELLOW}=== $1 ===${NC}\n"
}

run_test() {
    local test_file="$1"
    local test_name=$(basename "$test_file")
    
    if [ $VERBOSE -eq 1 ]; then
        echo -e "🚀 Running ${GREEN}${test_name}${NC}"
        "$test_file"
        local status=$?
    else
        "$test_file" > /dev/null 2>&1
        local status=$?
    fi
    
    if [ $status -eq 0 ]; then
        echo -e "✅ ${GREEN}PASS${NC}: ${test_name}"
        return 0
    else
        echo -e "❌ ${RED}FAIL${NC}: ${test_name} (exit code: ${status})"
        if [ $VERBOSE -eq 0 ]; then
            echo "Use -v to see detailed output"
        fi
        return 1
    fi
}

run_unit_tests() {
    local test_files=("${TEST_TARGETS[@]}")
    
    if [ ${#test_files[@]} -eq 0 ]; then
        test_files=($(find tests -name "test_*.c" -type f | sed 's/\.c$//' | sort))
    fi
    
    local total=0
    local passed=0
    local failed=0
    
    print_header "Running Unit Tests"
    
    for test_file in "${test_files[@]}"; do
        # Skip if not a unit test when specific test type is requested
        if [[ "$test_file" != *"/unit/"* && "$TEST_TYPE" == "unit" ]]; then
            continue
        fi
        
        # Build the test if source file exists
        if [ -f "${test_file}.c" ]; then
            local test_name=$(basename "${test_file}.c")
            local test_bin="${BUILD_DIR}/tests/${test_name}"
            local deps_line
            local extra_sources=()

            if deps_line=$(grep -m1 "^// TEST_DEPS:" "${test_file}.c" 2>/dev/null); then
                deps_line=${deps_line#// TEST_DEPS:}
                # shellcheck disable=SC2206 # we want word splitting for dependency list
                extra_sources=($deps_line)
            fi
    
            mkdir -p "$(dirname "$test_bin")"
    
            echo -n "🔨 Building ${test_name}... "
            if gcc -o "$test_bin" "${test_file}.c" "${extra_sources[@]}" -Iinclude -Itests/unit/mocks \
                -Wall -Wextra -Werror -g -O0 \
                $([ $COVERAGE -eq 1 ] && echo "--coverage -fprofile-arcs -ftest-coverage"); then
                echo -e "${GREEN}OK${NC}"
                test_file="$test_bin"
            else
                echo -e "${RED}FAILED${NC}"
                failed=$((failed + 1))
                total=$((total + 1))
                continue
            fi
        fi
        
        if [ -x "$test_file" ]; then
            run_test "$test_file"
            if [ $? -eq 0 ]; then
                passed=$((passed + 1))
            else
                failed=$((failed + 1))
            fi
            total=$((total + 1))
        fi
    done
    
    echo -e "\n📊 Test Results:"
    echo -e "  Total:  ${total}"
    echo -e "  ${GREEN}Passed: ${passed}${NC}"
    
    if [ $failed -gt 0 ]; then
        echo -e "  ${RED}Failed: ${failed}${NC}"
        return 1
    else
        echo -e "  ${GREEN}All tests passed!${NC}"
        return 0
    fi
}

run_integration_tests() {
    print_header "Running Integration Tests"
    
    if [ ! -d "tests/integration" ]; then
        echo -e "${YELLOW}⚠️  No integration tests directory found${NC}"
        return 0
    fi
    
    local test_files=("${TEST_TARGETS[@]}")
    
    if [ ${#test_files[@]} -eq 0 ]; then
        test_files=($(find tests/integration -name "test_*" -type f -executable | sort))
    fi
    
    if [ ${#test_files[@]} -eq 0 ]; then
        echo -e "${YELLOW}⚠️  No integration tests found${NC}"
        return 0
    fi
    
    local total=0
    local passed=0
    local failed=0
    
    for test_file in "${test_files[@]}"; do
        if [ -x "$test_file" ]; then
            run_test "$test_file"
            if [ $? -eq 0 ]; then
                passed=$((passed + 1))
            else
                failed=$((failed + 1))
            fi
            total=$((total + 1))
        fi
    done
    
    echo -e "\n📊 Integration Test Results:"
    echo -e "  Total:  ${total}"
    echo -e "  ${GREEN}Passed: ${passed}${NC}"
    
    if [ $failed -gt 0 ]; then
        echo -e "  ${RED}Failed: ${failed}${NC}"
        return 1
    else
        return 0
    fi
}

# Generate coverage report if requested
generate_coverage_report() {
    if [ $COVERAGE -eq 1 ]; then
        print_header "Generating Coverage Report"
        
        if ! command -v gcovr &> /dev/null; then
            echo -e "${YELLOW}⚠️  gcovr not found. Install with 'pip install gcovr'${NC}"
            return 1
        fi
        
        mkdir -p "${BUILD_DIR}/coverage"
        gcovr -r . --html --html-details -o "${BUILD_DIR}/coverage/coverage.html"
        
        echo -e "\n📊 Coverage report generated: ${BUILD_DIR}/coverage/coverage.html"
    fi
}

# Main test execution
main() {
    local exit_code=0
    
    case $TEST_TYPE in
        unit)
            run_unit_tests || exit_code=1
            ;;
        integration)
            run_integration_tests || exit_code=1
            ;;
        all)
            run_unit_tests || exit_code=1
            run_integration_tests || exit_code=1
            ;;
        *)
            echo -e "${RED}Error: Unknown test type '${TEST_TYPE}'${NC}" >&2
            show_help
            exit 1
            ;;
    esac
    
    if [ $COVERAGE -eq 1 ]; then
        generate_coverage_report
    fi
    
    if [ $exit_code -eq 0 ]; then
        echo -e "\n✨ ${GREEN}All tests completed successfully!${NC}"
    else
        echo -e "\n❌ ${RED}Some tests failed!${NC}"
    fi
    
    exit $exit_code
}

main "$@"
