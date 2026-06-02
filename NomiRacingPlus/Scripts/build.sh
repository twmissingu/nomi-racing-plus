#!/bin/bash
# NIO Racing Plus - Build Script
# Builds the UE5 project for Windows and macOS

set -e  # Exit on error

# Configuration
PROJECT_NAME="NomiRacingPlus"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/Build"
LOG_DIR="${BUILD_DIR}/Logs"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${LOG_DIR}"

# Detect platform
detect_platform() {
    case "$(uname -s)" in
        Linux*)     PLATFORM="Linux";;
        Darwin*)    PLATFORM="Mac";;
        CYGWIN*|MINGW*|MSYS*) PLATFORM="Windows";;
        *)          PLATFORM="Unknown"
    esac
    log_info "Platform: ${PLATFORM}"
}

# Find UE5 installation
find_ue5() {
    if [ -n "${UE5_ROOT}" ]; then
        UE5_DIR="${UE5_ROOT}"
    elif [ "${PLATFORM}" = "Mac" ]; then
        # Default macOS UE5 location
        UE5_DIR="/Users/Shared/Epic Games/UE_5.7"
    elif [ "${PLATFORM}" = "Windows" ]; then
        # Default Windows UE5 location
        UE5_DIR="C:/Program Files/Epic Games/UE_5.7"
    else
        log_error "Cannot auto-detect UE5 installation. Set UE5_ROOT environment variable."
        exit 1
    fi

    if [ ! -d "${UE5_DIR}" ]; then
        log_error "UE5 not found at: ${UE5_DIR}"
        log_error "Please set UE5_ROOT to your UE5 installation directory"
        exit 1
    fi

    log_info "UE5 found at: ${UE5_DIR}"
}

# Build project
build_project() {
    local BUILD_TYPE=$1
    local PLATFORM_TARGET=$2

    log_info "Building ${PROJECT_NAME} for ${PLATFORM_TARGET} (${BUILD_TYPE})..."

    if [ "${PLATFORM}" = "Mac" ]; then
        # macOS build
        UAT_PATH="${UE5_DIR}/Engine/Build/BatchFiles/Mac/RunUAT.sh"
        BUILD_COMMAND="BuildCookRun"
    else
        # Windows/Linux build
        UAT_PATH="${UE5_DIR}/Engine/Build/BatchFiles/RunUAT.bat"
        BUILD_COMMAND="BuildCookRun"
    fi

    # Build arguments
    BUILD_ARGS=(
        "${BUILD_COMMAND}"
        -project="${PROJECT_DIR}/${PROJECT_NAME}.uproject"
        -noP4
        -platform="${PLATFORM_TARGET}"
        -clientconfig="${BUILD_TYPE}"
        -cook
        -build
        -stage
        -package
        -archive
        -archivedirectory="${BUILD_DIR}/Archive"
        -pak
        -prereqs
        -nodebuginfo
        -utf8output
    )

    # Add platform-specific args
    if [ "${PLATFORM_TARGET}" = "Mac" ]; then
        BUILD_ARGS+=(-clientconfig=Shipping)
    fi

    # Run build
    log_info "Running: ${UAT_PATH} ${BUILD_ARGS[*]}"

    if [ -f "${UAT_PATH}" ]; then
        "${UAT_PATH}" "${BUILD_ARGS[@]}" 2>&1 | tee "${LOG_DIR}/build_${PLATFORM_TARGET}_${BUILD_TYPE}.log"
        BUILD_RESULT=$?
    else
        log_error "UAT not found at: ${UAT_PATH}"
        exit 1
    fi

    if [ ${BUILD_RESULT} -eq 0 ]; then
        log_info "Build successful!"
    else
        log_error "Build failed with exit code: ${BUILD_RESULT}"
        exit 1
    fi
}

# Cook content only
cook_content() {
    log_info "Cooking content for ${PLATFORM}..."

    if [ "${PLATFORM}" = "Mac" ]; then
        UAT_PATH="${UE5_DIR}/Engine/Build/BatchFiles/Mac/RunUAT.sh"
    else
        UAT_PATH="${UE5_DIR}/Engine/Build/BatchFiles/RunUAT.bat"
    fi

    COOK_ARGS=(
        "Cook"
        -project="${PROJECT_DIR}/${PROJECT_NAME}.uproject"
        -platform="${PLATFORM}"
        -iterative
        -unversioned
    )

    "${UAT_PATH}" "${COOK_ARGS[@]}" 2>&1 | tee "${LOG_DIR}/cook.log"
}

# Generate project files (for IDE)
generate_project_files() {
    log_info "Generating project files..."

    if [ "${PLATFORM}" = "Mac" ]; then
        UBT_PATH="${UE5_DIR}/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh"
    else
        UBT_PATH="${UE5_DIR}/Engine/Binaries/DotNET/UnrealBuildTool.exe"
    fi

    if [ -f "${UBT_PATH}" ]; then
        "${UBT_PATH}" -projectfiles -project="${PROJECT_DIR}/${PROJECT_NAME}.uproject" -game -rocket -progress
    else
        log_error "UBT not found at: ${UBT_PATH}"
        exit 1
    fi
}

# Clean build
clean_build() {
    log_info "Cleaning build artifacts..."

    rm -rf "${BUILD_DIR}"
    rm -rf "${PROJECT_DIR}/Intermediate"
    rm -rf "${PROJECT_DIR}/Binaries"
    rm -rf "${PROJECT_DIR}/Saved"

    log_info "Clean complete"
}

# Run automated tests
run_tests() {
    local FILTER=${1:-""}

    log_info "Running automated tests..."

    if [ "${PLATFORM}" = "Mac" ]; then
        EDITOR_CMD="${UE5_DIR}/Engine/Binaries/Mac/UnrealEditor-Cmd"
    else
        EDITOR_CMD="${UE5_DIR}/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
    fi

    if [ ! -f "${EDITOR_CMD}" ]; then
        log_error "UnrealEditor-Cmd not found at: ${EDITOR_CMD}"
        exit 1
    fi

    # Build test command
    local TEST_CMD="Automation RunTests NomiRacingPlus"
    if [ -n "${FILTER}" ]; then
        TEST_CMD="Automation RunTests ${FILTER}"
    fi

    log_info "Running: ${EDITOR_CMD} ${PROJECT_DIR}/${PROJECT_NAME}.uproject -ExecCmds=\"${TEST_CMD}; Quit\""

    "${EDITOR_CMD}" "${PROJECT_DIR}/${PROJECT_NAME}.uproject" \
        -ExecCmds="${TEST_CMD}; Quit" \
        -log \
        -unattended \
        -nopause \
        -nullrhi \
        -NoSound \
        -ReportOutputPath="${LOG_DIR}/TestResults" \
        2>&1 | tee "${LOG_DIR}/test.log"

    TEST_RESULT=$?

    if [ ${TEST_RESULT} -eq 0 ]; then
        log_info "Tests completed successfully!"
    else
        log_error "Tests failed with exit code: ${TEST_RESULT}"
        exit 1
    fi
}

# Show help
show_help() {
    echo "NIO Racing Plus Build Script"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build [Development|Shipping|Debug]  Build the project"
    echo "  cook                                 Cook content only"
    echo "  generate                             Generate project files"
    echo "  test [filter]                        Run automated tests"
    echo "  clean                                Clean build artifacts"
    echo "  help                                 Show this help"
    echo ""
    echo "Options:"
    echo "  --platform [Win64|Mac|Linux]         Target platform"
    echo "  --ue5 PATH                           Path to UE5 installation"
    echo ""
    echo "Examples:"
    echo "  $0 build Development                 Build for development"
    echo "  $0 build Shipping --platform Win64   Build for Windows shipping"
    echo "  $0 cook                              Cook content only"
    echo "  $0 test                              Run all tests"
    echo "  $0 test NomiRacingPlus.Vehicle       Run vehicle tests only"
    echo "  $0 clean                             Clean build artifacts"
}

# Main
main() {
    detect_platform
    find_ue5

    COMMAND=${1:-help}
    BUILD_TYPE=${2:-Development}
    PLATFORM_TARGET=${PLATFORM}

    # Parse options
    shift 2 2>/dev/null || true
    while [[ $# -gt 0 ]]; do
        case $1 in
            --platform)
                PLATFORM_TARGET="$2"
                shift 2
                ;;
            --ue5)
                UE5_DIR="$2"
                shift 2
                ;;
            *)
                shift
                ;;
        esac
    done

    case ${COMMAND} in
        build)
            build_project "${BUILD_TYPE}" "${PLATFORM_TARGET}"
            ;;
        cook)
            cook_content
            ;;
        generate)
            generate_project_files
            ;;
        test)
            # For test, BUILD_TYPE is actually the filter
            run_tests "${BUILD_TYPE}"
            ;;
        clean)
            clean_build
            ;;
        help|*)
            show_help
            ;;
    esac
}

main "$@"
