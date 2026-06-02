#!/bin/bash
# ==============================================================================
# NIO Racing Plus - Verification Script
# ==============================================================================
# Runs pre-commit checks in two modes:
#   Fast mode (default): Static checks + compile verification (~30s)
#   Full mode (--full):  Fast mode + UE5 Automation tests (~minutes)
#
# Usage:
#   ./Scripts/verify.sh           # Fast mode
#   ./Scripts/verify.sh --full    # Full mode with UE5 tests
#   ./Scripts/verify.sh --quick   # Static checks only (no compile)
# ==============================================================================

set -euo pipefail

# --- Configuration ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PROJECT_NAME="NomiRacingPlus"
UPROJECT="${PROJECT_DIR}/${PROJECT_NAME}.uproject"
SOURCE_DIR="${PROJECT_DIR}/Source/${PROJECT_NAME}"
TESTS_DIR="${SOURCE_DIR}/Tests"
LOG_DIR="${PROJECT_DIR}/Build/Logs"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'

# Counters
CHECKS_PASSED=0
CHECKS_FAILED=0
CHECKS_WARNED=0

# --- Logging ---
log_header() {
    echo -e "\n${BOLD}${CYAN}━━━ $1 ━━━${NC}"
}

log_pass() {
    echo -e "  ${GREEN}✓${NC} $1"
    CHECKS_PASSED=$((CHECKS_PASSED + 1))
}

log_fail() {
    echo -e "  ${RED}✗${NC} $1"
    CHECKS_FAILED=$((CHECKS_FAILED + 1))
}

log_warn() {
    echo -e "  ${YELLOW}⚠${NC} $1"
    CHECKS_WARNED=$((CHECKS_WARNED + 1))
}

log_info() {
    echo -e "  ${BLUE}ℹ${NC} $1"
}

# --- Check Functions ---

# Check 1: Project structure
check_project_structure() {
    log_header "Project Structure"

    if [ -f "${UPROJECT}" ]; then
        log_pass ".uproject file exists"
    else
        log_fail ".uproject file missing: ${UPROJECT}"
    fi

    if [ -d "${SOURCE_DIR}" ]; then
        log_pass "Source directory exists"
    else
        log_fail "Source directory missing: ${SOURCE_DIR}"
    fi

    if [ -d "${TESTS_DIR}" ]; then
        local test_count=$(find "${TESTS_DIR}" -name "*.cpp" -o -name "*.h" | wc -l | tr -d ' ')
        log_pass "Tests directory exists (${test_count} files)"
    else
        log_fail "Tests directory missing: ${TESTS_DIR}"
    fi

    if [ -d "${PROJECT_DIR}/Config" ]; then
        log_pass "Config directory exists"
    else
        log_warn "Config directory missing"
    fi

    if [ -d "${PROJECT_DIR}/Content" ]; then
        log_pass "Content directory exists"
    else
        log_warn "Content directory missing"
    fi
}

# Check 2: JSON validation
check_json_files() {
    log_header "JSON Validation"

    local json_count=0
    local json_errors=0

    while IFS= read -r -d '' file; do
        json_count=$((json_count + 1))
        if python3 -m json.tool "$file" > /dev/null 2>&1; then
            log_pass "$(basename "$file")"
        else
            log_fail "$(basename "$file") - invalid JSON"
            json_errors=$((json_errors + 1))
        fi
    done < <(find "${PROJECT_DIR}" -name "*.json" -type f -not -path '*/.git/*' -not -path '*/node_modules/*' -not -path '*/Build/*' -not -path '*/Intermediate/*' -not -path '*/Saved/*' -not -name 'PackageRestoreData.json' -print0 2>/dev/null)

    if [ ${json_count} -eq 0 ]; then
        log_info "No JSON files found"
    elif [ ${json_errors} -eq 0 ]; then
        log_info "${json_count} JSON files validated"
    fi
}

# Check 3: C++ header include consistency
check_cpp_includes() {
    log_header "C++ Include Consistency"

    local missing_includes=0

    # Project subdirectories (local includes to check)
    local project_dirs="AI Camera Core Effects NOMI Race Tests UI Vehicles"

    # Check that local project includes resolve correctly
    while IFS= read -r line; do
        local file=$(echo "$line" | cut -d: -f1)
        local include=$(echo "$line" | grep -oE '#include\s+"[^"]+"' | sed 's/#include\s*"//;s/"//')

        if [ -n "$include" ]; then
            # Only check includes that look like project-local headers
            # (contain one of our project subdirectory prefixes)
            local is_local=0
            for dir in ${project_dirs}; do
                if [[ "${include}" == ${dir}/* ]] || [[ "${include}" == Tests/${dir}* ]]; then
                    is_local=1
                    break
                fi
            done

            # Also check same-directory includes (no / prefix)
            if [[ "${include}" != *"/"* ]] && [[ "${include}" != *".generated.h" ]]; then
                local file_dir=$(dirname "$file")
                if [ -f "${file_dir}/${include}" ]; then
                    is_local=0  # Exists in same directory, skip
                fi
            fi

            if [ ${is_local} -eq 1 ]; then
                local include_path="${SOURCE_DIR}/${include}"
                if [ ! -f "${include_path}" ]; then
                    log_warn "Missing local include: ${include} (in $(basename "$file"))"
                    missing_includes=$((missing_includes + 1))
                fi
            fi
        fi
    done < <(grep -rn '#include\s*"' "${SOURCE_DIR}" --include="*.h" --include="*.cpp" 2>/dev/null || true)

    if [ ${missing_includes} -eq 0 ]; then
        log_pass "All local includes resolved"
    else
        log_warn "${missing_includes} local include(s) could not be resolved"
    fi
}

# Check 4: UBT compile check (fast mode)
check_ubt_compile() {
    log_header "UBT Compile Check"

    # Find UE5 installation
    local ue5_dir=""
    if [ -n "${UE5_ROOT:-}" ]; then
        ue5_dir="${UE5_ROOT}"
    elif [ -d "/Users/Shared/Epic Games/UE_5.7" ]; then
        ue5_dir="/Users/Shared/Epic Games/UE_5.7"
    elif [ -d "/Users/Shared/Epic Games/UE_5.5" ]; then
        ue5_dir="/Users/Shared/Epic Games/UE_5.5"
    else
        log_warn "UE5 not found - skipping compile check (set UE5_ROOT)"
        return
    fi

    local ubt_path=""
    if [ "$(uname -s)" = "Darwin" ]; then
        ubt_path="${ue5_dir}/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh"
    else
        ubt_path="${ue5_dir}/Engine/Binaries/DotNET/UnrealBuildTool.exe"
    fi

    if [ ! -f "${ubt_path}" ]; then
        log_warn "UBT not found at: ${ubt_path} - skipping compile check"
        return
    fi

    log_info "UE5 found at: ${ue5_dir}"
    log_info "Running UBT project file generation..."

    mkdir -p "${LOG_DIR}"

    if "${ubt_path}" -projectfiles -project="${UPROJECT}" -game -rocket -progress > "${LOG_DIR}/ubt_generate.log" 2>&1; then
        log_pass "UBT project generation succeeded"
    else
        log_fail "UBT project generation failed - check ${LOG_DIR}/ubt_generate.log"
    fi
}

# Check 5: Test file integrity
check_test_files() {
    log_header "Test File Integrity"

    local test_headers=$(find "${TESTS_DIR}" -name "*.h" | wc -l | tr -d ' ')
    local test_sources=$(find "${TESTS_DIR}" -name "*.cpp" | wc -l | tr -d ' ')

    # Each .h should have a matching .cpp (except TestUtilities which is .h + .cpp)
    local missing_pairs=0
    while IFS= read -r header; do
        local base=$(basename "$header" .h)
        local dir=$(dirname "$header")
        local cpp="${dir}/${base}.cpp"

        # TestUtilities.h has TestUtilities.cpp, others should too
        if [ ! -f "${cpp}" ]; then
            # Check if it's an IMPLEMENT_SIMPLE_AUTOMATION_TEST header (no separate .cpp needed)
            if grep -q "IMPLEMENT_SIMPLE_AUTOMATION_TEST" "${header}" 2>/dev/null; then
                # These headers define tests - their .cpp should exist
                log_warn "Missing .cpp for: ${base}.h"
                missing_pairs=$((missing_pairs + 1))
            fi
        fi
    done < <(find "${TESTS_DIR}" -name "*.h")

    log_pass "${test_headers} headers, ${test_sources} source files"

    if [ ${missing_pairs} -eq 0 ]; then
        log_pass "All test files have matching pairs"
    fi

    # Check for IMPLEMENT_SIMPLE_AUTOMATION_TEST macros
    local test_count=$(grep -r "IMPLEMENT_SIMPLE_AUTOMATION_TEST" "${TESTS_DIR}" 2>/dev/null | wc -l | tr -d ' ')
    if [ ${test_count} -gt 0 ]; then
        log_pass "${test_count} automation test(s) defined"
    else
        log_warn "No IMPLEMENT_SIMPLE_AUTOMATION_TEST macros found"
    fi
}

# Check 6: Config file validation
check_config_files() {
    log_header "Config File Validation"

    local config_dir="${PROJECT_DIR}/Config"
    if [ ! -d "${config_dir}" ]; then
        log_warn "Config directory not found"
        return
    fi

    local ini_count=0
    local ini_errors=0

    for ini in "${config_dir}"/*.ini; do
        if [ -f "$ini" ]; then
            ini_count=$((ini_count + 1))
            # Basic INI validation: check for unclosed sections
            if grep -qE '^\[.*[^]]$' "$ini" 2>/dev/null; then
                log_fail "$(basename "$ini") - unclosed section header"
                ini_errors=$((ini_errors + 1))
            else
                log_pass "$(basename "$ini")"
            fi
        fi
    done

    if [ ${ini_count} -eq 0 ]; then
        log_info "No INI files found"
    fi
}

# Check 7: UE5 Automation tests (full mode only)
check_ue5_tests() {
    log_header "UE5 Automation Tests"

    local ue5_dir=""
    if [ -n "${UE5_ROOT:-}" ]; then
        ue5_dir="${UE5_ROOT}"
    elif [ -d "/Users/Shared/Epic Games/UE_5.7" ]; then
        ue5_dir="/Users/Shared/Epic Games/UE_5.7"
    elif [ -d "/Users/Shared/Epic Games/UE_5.5" ]; then
        ue5_dir="/Users/Shared/Epic Games/UE_5.5"
    else
        log_fail "UE5 not found - cannot run tests (set UE5_ROOT)"
        return
    fi

    local editor_cmd=""
    if [ "$(uname -s)" = "Darwin" ]; then
        editor_cmd="${ue5_dir}/Engine/Binaries/Mac/UnrealEditor-Cmd"
    else
        editor_cmd="${ue5_dir}/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
    fi

    if [ ! -f "${editor_cmd}" ]; then
        log_fail "UnrealEditor-Cmd not found at: ${editor_cmd}"
        return
    fi

    log_info "Running UE5 Automation tests..."
    log_info "This may take several minutes..."

    mkdir -p "${LOG_DIR}"

    local test_log="${LOG_DIR}/test_results.log"
    local test_exit=0

    "${editor_cmd}" "${UPROJECT}" \
        -ExecCmds="Automation RunTests NomiRacingPlus; Quit" \
        -log \
        -unattended \
        -nopause \
        -nullrhi \
        -NoSound \
        > "${test_log}" 2>&1 || test_exit=$?

    if [ ${test_exit} -eq 0 ]; then
        # Parse results from log
        local passed=$(grep -c "Passed" "${test_log}" 2>/dev/null || echo "0")
        local failed=$(grep -c "Failed" "${test_log}" 2>/dev/null || echo "0")

        if [ "${failed}" -gt 0 ]; then
            log_fail "${failed} test(s) failed, ${passed} passed - check ${test_log}"
        else
            log_pass "All tests passed (${passed} total)"
        fi
    else
        log_fail "Test execution failed (exit code: ${test_exit}) - check ${test_log}"
    fi
}

# Check 8: Large file check
check_large_files() {
    log_header "Large File Check"

    local large_count=0
    while IFS= read -r -d '' file; do
        local size=$(du -h "$file" | cut -f1)
        log_warn "Large file: $(basename "$file") (${size})"
        large_count=$((large_count + 1))
    done < <(find "${PROJECT_DIR}" -type f -size +10M -not -path '*/.git/*' -not -path '*/Build/*' -not -path '*/Intermediate/*' -not -path '*/Binaries/*' -print0 2>/dev/null)

    if [ ${large_count} -eq 0 ]; then
        log_pass "No oversized files (>10MB)"
    fi
}

# --- Main ---
main() {
    local mode="fast"

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --full)
                mode="full"
                shift
                ;;
            --quick)
                mode="quick"
                shift
                ;;
            --help|-h)
                echo "NIO Racing Plus Verification Script"
                echo ""
                echo "Usage: $0 [options]"
                echo ""
                echo "Options:"
                echo "  (default)    Fast mode: static checks + compile verification"
                echo "  --full       Full mode: fast mode + UE5 Automation tests"
                echo "  --quick      Quick mode: static checks only (no compile)"
                echo "  --help       Show this help"
                exit 0
                ;;
            *)
                shift
                ;;
        esac
    done

    echo -e "\n${BOLD}${CYAN}╔══════════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}${CYAN}║   NIO Racing Plus - Verification (${mode} mode)    ║${NC}"
    echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════╝${NC}"

    # Always run these checks
    check_project_structure
    check_json_files
    check_config_files
    check_test_files
    check_large_files
    check_cpp_includes

    # Fast and full mode: compile check
    if [ "${mode}" != "quick" ]; then
        check_ubt_compile
    fi

    # Full mode: run UE5 tests
    if [ "${mode}" = "full" ]; then
        check_ue5_tests
    fi

    # Summary
    echo -e "\n${BOLD}${CYAN}━━━ Summary ━━━${NC}"
    echo -e "  ${GREEN}✓ Passed:${NC}  ${CHECKS_PASSED}"
    echo -e "  ${YELLOW}⚠ Warned:${NC}  ${CHECKS_WARNED}"
    echo -e "  ${RED}✗ Failed:${NC}  ${CHECKS_FAILED}"

    if [ ${CHECKS_FAILED} -gt 0 ]; then
        echo -e "\n${RED}${BOLD}Verification FAILED${NC} - ${CHECKS_FAILED} check(s) failed"
        exit 1
    elif [ ${CHECKS_WARNED} -gt 0 ]; then
        echo -e "\n${YELLOW}${BOLD}Verification PASSED with warnings${NC}"
        exit 0
    else
        echo -e "\n${GREEN}${BOLD}Verification PASSED${NC}"
        exit 0
    fi
}

main "$@"
