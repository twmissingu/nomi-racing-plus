#!/bin/bash
# ==============================================================================
# NIO Racing Plus - macOS Build, Sign, Package, and Distribute
# ==============================================================================
# Usage:
#   ./build_macos.sh [command] [options]
#
# Commands:
#   build       - Build the UE5 project for macOS
#   sign        - Code sign an existing build
#   notarize    - Notarize and staple a signed build
#   package     - Create DMG installer
#   release     - Full pipeline: build -> sign -> notarize -> package
#   clean       - Remove build artifacts
#   verify      - Verify code signature of a built app
#
# Options:
#   --build-type [Development|Shipping|Debug]  (default: Shipping)
#   --ue5 PATH                                 UE5 installation path
#   --skip-sign                                Skip code signing
#   --skip-notarize                            Skip notarization
#   --skip-package                             Skip DMG creation
#   --verbose                                  Enable verbose output
#
# Environment Variables:
#   MAC_SIGNING_IDENTITY   - Code signing identity
#   MAC_TEAM_ID            - Apple Team ID
#   MAC_NOTARIZE_APPLE_ID  - Apple ID for notarization
#   MAC_NOTARIZE_PASSWORD  - App-specific password for notarization
#   MAC_NOTARIZE_TEAM_ID   - Team ID for notarization
# ==============================================================================

set -euo pipefail

# --- Script Directory & Source Config ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# shellcheck source=build_config.sh
source "${SCRIPT_DIR}/build_config.sh"

# --- Derived Paths ---
BUILD_DIR="${PROJECT_DIR}/${BUILD_DIR_NAME}"
ARCHIVE_DIR="${BUILD_DIR}/${ARCHIVE_DIR_NAME}"
PACKAGES_DIR="${BUILD_DIR}/${PACKAGES_DIR_NAME}"
LOGS_DIR="${BUILD_DIR}/${LOGS_DIR_NAME}"

# --- Command Defaults ---
COMMAND="${1:-help}"
BUILD_TYPE="${DEFAULT_BUILD_TYPE}"
UE5_DIR=""
SKIP_SIGN=false
SKIP_NOTARIZE=false
SKIP_PACKAGE=false
VERBOSE=false

# --- Color Output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info()  { echo -e "${GREEN}[INFO]${NC}  $(date '+%H:%M:%S') $1"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC}  $(date '+%H:%M:%S') $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $(date '+%H:%M:%S') $1"; }
log_step()  { echo -e "${CYAN}[STEP]${NC}  $(date '+%H:%M:%S') $1"; }
log_debug() { if [ "${VERBOSE}" = true ]; then echo -e "${BLUE}[DEBUG]${NC} $(date '+%H:%M:%S') $1"; fi; }

# --- Parse Arguments ---
parse_args() {
    shift  # Skip command
    while [[ $# -gt 0 ]]; do
        case $1 in
            --build-type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            --ue5)
                UE5_DIR="$2"
                shift 2
                ;;
            --skip-sign)
                SKIP_SIGN=true
                shift
                ;;
            --skip-notarize)
                SKIP_NOTARIZE=true
                shift
                ;;
            --skip-package)
                SKIP_PACKAGE=true
                shift
                ;;
            --verbose)
                VERBOSE=true
                shift
                ;;
            *)
                log_warn "Unknown option: $1"
                shift
                ;;
        esac
    done
}

# --- Find UE5 Installation ---
find_ue5() {
    if [ -n "${UE5_DIR}" ] && [ -d "${UE5_DIR}" ]; then
        log_info "Using UE5 from: ${UE5_DIR}"
        return 0
    fi

    # Check environment variable
    if [ -n "${UE5_ROOT:-}" ] && [ -d "${UE5_ROOT}" ]; then
        UE5_DIR="${UE5_ROOT}"
        log_info "Using UE5 from UE5_ROOT: ${UE5_DIR}"
        return 0
    fi

    # Common macOS locations
    local candidates=(
        "/Users/Shared/Epic Games/UE_5.5"
        "/Users/Shared/Epic Games/UE_5.4"
        "${HOME}/Epic Games/UE_5.5"
        "${HOME}/Epic Games/UE_5.4"
    )

    for candidate in "${candidates[@]}"; do
        if [ -d "${candidate}" ]; then
            UE5_DIR="${candidate}"
            log_info "Found UE5 at: ${UE5_DIR}"
            return 0
        fi
    done

    log_error "UE5 not found. Set UE5_ROOT or use --ue5 PATH"
    exit 1
}

# --- Initialize Directories ---
init_dirs() {
    log_debug "Creating output directories..."
    mkdir -p "${BUILD_DIR}"
    mkdir -p "${ARCHIVE_DIR}"
    mkdir -p "${PACKAGES_DIR}"
    mkdir -p "${LOGS_DIR}"
}

# ==============================================================================
# BUILD
# ==============================================================================
cmd_build() {
    log_step "Building ${PROJECT_DISPLAY_NAME} for macOS (${BUILD_TYPE})..."

    find_ue5
    init_dirs

    local uat_path="${UE5_DIR}/Engine/Build/BatchFiles/Mac/RunUAT.sh"
    if [ ! -f "${uat_path}" ]; then
        log_error "RunUAT.sh not found at: ${uat_path}"
        exit 1
    fi

    local build_args=(
        "BuildCookRun"
        -project="${PROJECT_DIR}/${PROJECT_NAME}.uproject"
        -noP4
        -platform="${DEFAULT_PLATFORM_MAC}"
        -clientconfig="${BUILD_TYPE}"
        -cook
        -build
        -stage
        -package
        -archive
        -archivedirectory="${ARCHIVE_DIR}"
        -pak
        -prereqs
        -utf8output
    )

    if [ "${VERBOSE}" = true ]; then
        build_args+=(-verbose)
    fi

    log_info "Running UAT: ${uat_path}"
    log_debug "Arguments: ${build_args[*]}"

    local log_file="${LOGS_DIR}/build_macos_${BUILD_TYPE}_$(date '+%Y%m%d_%H%M%S').log"

    "${uat_path}" "${build_args[@]}" 2>&1 | tee "${log_file}"
    local result=${PIPESTATUS[0]}

    if [ ${result} -ne 0 ]; then
        log_error "Build failed with exit code: ${result}"
        log_error "See log: ${log_file}"
        exit 1
    fi

    log_info "Build completed successfully"
    log_info "Archive location: ${ARCHIVE_DIR}"

    # List built artifacts
    find "${ARCHIVE_DIR}" -maxdepth 2 -type d -name "*.app" 2>/dev/null | while read -r app; do
        log_info "Built app: ${app}"
    done
}

# ==============================================================================
# CODE SIGNING
# ==============================================================================
cmd_sign() {
    log_step "Code signing macOS build..."

    if [ "${SKIP_SIGN}" = true ]; then
        log_warn "Skipping code signing (--skip-sign)"
        return 0
    fi

    # Validate signing environment
    if ! validate_mac_signing_env; then
        log_error "Signing environment validation failed"
        exit 1
    fi

    # Find the .app bundle
    local app_bundle
    app_bundle=$(find "${ARCHIVE_DIR}" -maxdepth 3 -type d -name "*.app" | head -1)

    if [ -z "${app_bundle}" ]; then
        log_error "No .app bundle found in ${ARCHIVE_DIR}"
        log_error "Run 'build' command first"
        exit 1
    fi

    log_info "Signing: ${app_bundle}"
    log_info "Identity: ${MAC_SIGNING_IDENTITY}"

    # Step 1: Sign all embedded frameworks and dylibs first
    log_step "Signing embedded frameworks and libraries..."
    find "${app_bundle}" -name "*.dylib" -o -name "*.framework" | while read -r item; do
        log_debug "Signing: ${item}"
        codesign --force --sign "${MAC_SIGNING_IDENTITY}" \
            --timestamp \
            --options runtime \
            "${item}" 2>&1 || log_warn "Failed to sign: ${item}"
    done

    # Step 2: Sign the main app bundle with hardened runtime
    log_step "Signing main application bundle..."
    codesign --force --sign "${MAC_SIGNING_IDENTITY}" \
        --timestamp \
        --options runtime \
        --deep \
        --entitlements "${SCRIPT_DIR}/entitlements.plist" \
        "${app_bundle}" 2>&1

    if [ $? -ne 0 ]; then
        log_error "Code signing failed"
        exit 1
    fi

    # Step 3: Verify signature
    log_step "Verifying code signature..."
    codesign --verify --deep --strict --verbose=2 "${app_bundle}" 2>&1

    if [ $? -ne 0 ]; then
        log_error "Code signature verification failed"
        exit 1
    fi

    # Step 4: Display signature info
    log_info "Signature details:"
    codesign -dvvv "${app_bundle}" 2>&1 | grep -E "(Authority|TeamIdentifier|Timestamp|Runtime)" | while read -r line; do
        log_info "  ${line}"
    done

    log_info "Code signing completed successfully"
}

# ==============================================================================
# NOTARIZATION
# ==============================================================================
cmd_notarize() {
    log_step "Notarizing macOS build..."

    if [ "${SKIP_NOTARIZE}" = true ]; then
        log_warn "Skipping notarization (--skip-notarize)"
        return 0
    fi

    # Validate notarization environment
    if [ -z "${MAC_NOTARIZE_APPLE_ID}" ] || [ -z "${MAC_NOTARIZE_PASSWORD}" ]; then
        log_error "Notarization credentials not set"
        log_error "Set MAC_NOTARIZE_APPLE_ID and MAC_NOTARIZE_PASSWORD"
        exit 1
    fi

    local team_id="${MAC_NOTARIZE_TEAM_ID:-${MAC_TEAM_ID}}"
    if [ -z "${team_id}" ]; then
        log_error "Team ID not set (MAC_NOTARIZE_TEAM_ID or MAC_TEAM_ID)"
        exit 1
    fi

    # Find the .app bundle
    local app_bundle
    app_bundle=$(find "${ARCHIVE_DIR}" -maxdepth 3 -type d -name "*.app" | head -1)

    if [ -z "${app_bundle}" ]; then
        log_error "No .app bundle found in ${ARCHIVE_DIR}"
        exit 1
    fi

    # Step 1: Create a ZIP for notarization submission
    local app_name
    app_name=$(basename "${app_bundle}" .app)
    local zip_path="${PACKAGES_DIR}/${app_name}_notarize.zip"

    log_step "Creating ZIP for notarization: ${zip_path}"
    ditto -c -k --keepParent "${app_bundle}" "${zip_path}"

    # Step 2: Submit for notarization
    log_step "Submitting to Apple notarization service..."
    local submit_output
    submit_output=$(xcrun notarytool submit "${zip_path}" \
        --apple-id "${MAC_NOTARIZE_APPLE_ID}" \
        --password "${MAC_NOTARIZE_PASSWORD}" \
        --team-id "${team_id}" \
        --wait \
        --timeout 30m 2>&1)

    local submit_result=$?
    echo "${submit_output}"

    if [ ${submit_result} -ne 0 ]; then
        log_error "Notarization submission failed"
        exit 1
    fi

    # Check for success
    if echo "${submit_output}" | grep -q "status: Accepted"; then
        log_info "Notarization accepted"
    else
        log_error "Notarization was not accepted"
        # Try to get the log
        local submission_id
        submission_id=$(echo "${submit_output}" | grep -oP 'id: \K[a-f0-9-]+' | head -1)
        if [ -n "${submission_id}" ]; then
            log_info "Fetching notarization log for: ${submission_id}"
            xcrun notarytool log "${submission_id}" \
                --apple-id "${MAC_NOTARIZE_APPLE_ID}" \
                --password "${MAC_NOTARIZE_PASSWORD}" \
                --team-id "${team_id}" 2>&1 || true
        fi
        exit 1
    fi

    # Step 3: Staple the notarization ticket
    log_step "Stapling notarization ticket..."
    xcrun stapler staple "${app_bundle}" 2>&1

    if [ $? -ne 0 ]; then
        log_error "Stapling failed"
        exit 1
    fi

    # Step 4: Verify stapling
    log_step "Verifying stapled notarization..."
    xcrun stapler validate "${app_bundle}" 2>&1

    # Clean up temporary zip
    rm -f "${zip_path}"

    log_info "Notarization completed successfully"
}

# ==============================================================================
# PACKAGE (DMG Creation)
# ==============================================================================
cmd_package() {
    log_step "Creating DMG installer..."

    if [ "${SKIP_PACKAGE}" = true ]; then
        log_warn "Skipping packaging (--skip-package)"
        return 0
    fi

    # Find the .app bundle
    local app_bundle
    app_bundle=$(find "${ARCHIVE_DIR}" -maxdepth 3 -type d -name "*.app" | head -1)

    if [ -z "${app_bundle}" ]; then
        log_error "No .app bundle found in ${ARCHIVE_DIR}"
        exit 1
    fi

    local app_name
    app_name=$(basename "${app_bundle}" .app)
    local dmg_name="${app_name}_${PROJECT_VERSION}_macos.dmg"
    local dmg_path="${PACKAGES_DIR}/${dmg_name}"
    local temp_dmg="${PACKAGES_DIR}/${app_name}_temp.dmg"
    local mount_point="/Volumes/${DMG_VOLUME_NAME}"

    # Clean up any existing DMG
    rm -f "${dmg_path}" "${temp_dmg}"

    # Unmount if previously mounted
    hdiutil detach "${mount_point}" 2>/dev/null || true

    log_step "Creating DMG: ${dmg_name}"

    # Create a temporary directory for DMG contents
    local dmg_staging="${PACKAGES_DIR}/dmg_staging"
    rm -rf "${dmg_staging}"
    mkdir -p "${dmg_staging}"

    # Copy app bundle to staging
    log_info "Copying app bundle to staging..."
    cp -R "${app_bundle}" "${dmg_staging}/"

    # Create Applications symlink for drag-to-install
    ln -s /Applications "${dmg_staging}/Applications"

    # Create DMG
    log_info "Building DMG image..."
    hdiutil create \
        -volname "${DMG_VOLUME_NAME}" \
        -srcfolder "${dmg_staging}" \
        -ov \
        -format UDZO \
        -imagekey zlib-level=9 \
        "${dmg_path}" 2>&1

    local result=$?

    # Clean up staging
    rm -rf "${dmg_staging}"

    if [ ${result} -ne 0 ]; then
        log_error "DMG creation failed"
        exit 1
    fi

    # Sign the DMG if signing is enabled
    if [ "${SKIP_SIGN}" != true ] && [ -n "${MAC_SIGNING_IDENTITY}" ]; then
        log_step "Signing DMG..."
        codesign --force --sign "${MAC_SIGNING_IDENTITY}" \
            --timestamp \
            "${dmg_path}" 2>&1
    fi

    # Notarize the DMG if notarization is enabled
    if [ "${SKIP_NOTARIZE}" != true ] && [ -n "${MAC_NOTARIZE_APPLE_ID}" ]; then
        log_step "Notarizing DMG..."
        local team_id="${MAC_NOTARIZE_TEAM_ID:-${MAC_TEAM_ID}}"

        xcrun notarytool submit "${dmg_path}" \
            --apple-id "${MAC_NOTARIZE_APPLE_ID}" \
            --password "${MAC_NOTARIZE_PASSWORD}" \
            --team-id "${team_id}" \
            --wait \
            --timeout 30m 2>&1

        if [ $? -eq 0 ]; then
            log_step "Stapling DMG..."
            xcrun stapler staple "${dmg_path}" 2>&1
        else
            log_warn "DMG notarization failed (non-fatal, DMG still usable)"
        fi
    fi

    # Generate checksums
    log_step "Generating checksums..."
    local checksum_file="${PACKAGES_DIR}/${dmg_name}.sha256"
    shasum -a 256 "${dmg_path}" > "${checksum_file}"
    log_info "SHA256: $(cat "${checksum_file}")"

    local size
    size=$(du -h "${dmg_path}" | cut -f1)
    log_info "DMG created: ${dmg_path} (${size})"

    log_info "Packaging completed successfully"
}

# ==============================================================================
# VERIFY
# ==============================================================================
cmd_verify() {
    log_step "Verifying build signatures..."

    local app_bundle
    app_bundle=$(find "${ARCHIVE_DIR}" -maxdepth 3 -type d -name "*.app" | head -1)

    if [ -z "${app_bundle}" ]; then
        log_error "No .app bundle found in ${ARCHIVE_DIR}"
        exit 1
    fi

    log_info "Verifying: ${app_bundle}"
    echo ""

    # Gatekeeper assessment
    log_step "Gatekeeper assessment:"
    spctl --assess --type execute --verbose --context context:primary-signature "${app_bundle}" 2>&1 || true
    echo ""

    # Code signature details
    log_step "Code signature details:"
    codesign -dvvv "${app_bundle}" 2>&1
    echo ""

    # Check for hardened runtime
    log_step "Hardened runtime check:"
    if codesign -dvvv "${app_bundle}" 2>&1 | grep -q "runtime"; then
        log_info "Hardened runtime: ENABLED"
    else
        log_warn "Hardened runtime: NOT DETECTED"
    fi
    echo ""

    # Check embedded signature count
    local sig_count
    sig_count=$(find "${app_bundle}" -name "*.dylib" -o -name "*.framework" | wc -l | tr -d ' ')
    log_info "Embedded signed components: ${sig_count}"

    # Verify DMG if it exists
    local dmg_file
    dmg_file=$(find "${PACKAGES_DIR}" -name "*.dmg" | head -1)
    if [ -n "${dmg_file}" ]; then
        log_step "Verifying DMG: $(basename "${dmg_file}")"
        codesign --verify --deep --strict "${dmg_file}" 2>&1 || true
        hdiutil verify "${dmg_file}" 2>&1 || true
    fi

    log_info "Verification complete"
}

# ==============================================================================
# CLEAN
# ==============================================================================
cmd_clean() {
    log_step "Cleaning build artifacts..."

    local dirs_to_clean=(
        "${BUILD_DIR}"
        "${PROJECT_DIR}/Intermediate"
        "${PROJECT_DIR}/Binaries"
        "${PROJECT_DIR}/Saved/StagedBuilds"
    )

    for dir in "${dirs_to_clean[@]}"; do
        if [ -d "${dir}" ]; then
            log_info "Removing: ${dir}"
            rm -rf "${dir}"
        fi
    done

    log_info "Clean completed"
}

# ==============================================================================
# FULL RELEASE PIPELINE
# ==============================================================================
cmd_release() {
    log_step "Starting full release pipeline for macOS..."
    echo ""

    local start_time
    start_time=$(date +%s)

    # Validate environment early
    if [ "${SKIP_SIGN}" != true ]; then
        if ! validate_mac_signing_env; then
            log_error "Signing environment validation failed. Use --skip-sign to skip."
            exit 1
        fi
    fi

    # Pipeline stages
    cmd_build

    echo ""
    log_step "========================================="
    log_step "Build complete, starting sign stage..."
    log_step "========================================="
    echo ""

    cmd_sign

    echo ""
    log_step "========================================="
    log_step "Signing complete, starting notarize stage..."
    log_step "========================================="
    echo ""

    cmd_notarize

    echo ""
    log_step "========================================="
    log_step "Notarization complete, starting packaging stage..."
    log_step "========================================="
    echo ""

    cmd_package

    echo ""
    log_step "========================================="
    log_step "Packaging complete, running verification..."
    log_step "========================================="
    echo ""

    cmd_verify

    local end_time
    end_time=$(date +%s)
    local duration=$(( end_time - start_time ))
    local minutes=$(( duration / 60 ))
    local seconds=$(( duration % 60 ))

    echo ""
    log_info "==========================================="
    log_info "  Release Pipeline Complete"
    log_info "  Build Type:  ${BUILD_TYPE}"
    log_info "  Duration:    ${minutes}m ${seconds}s"
    log_info "  Output:      ${PACKAGES_DIR}/"
    log_info "==========================================="
}

# ==============================================================================
# HELP
# ==============================================================================
cmd_help() {
    cat << 'HELP'
NIO Racing Plus - macOS Build Automation
=========================================

Usage: ./build_macos.sh [command] [options]

Commands:
  build       Build the UE5 project for macOS
  sign        Code sign an existing build
  notarize    Notarize and staple a signed build
  package     Create DMG installer
  release     Full pipeline: build -> sign -> notarize -> package
  verify      Verify code signatures
  clean       Remove build artifacts
  help        Show this help message

Options:
  --build-type [Development|Shipping|Debug]  Build configuration (default: Shipping)
  --ue5 PATH                                 Path to UE5 installation
  --skip-sign                                Skip code signing step
  --skip-notarize                            Skip notarization step
  --skip-package                             Skip DMG creation
  --verbose                                  Enable verbose output

Environment Variables:
  MAC_SIGNING_IDENTITY     Code signing identity (e.g., "Developer ID Application: Name (TEAM)")
  MAC_TEAM_ID              Apple Team ID
  MAC_NOTARIZE_APPLE_ID    Apple ID for notarization
  MAC_NOTARIZE_PASSWORD    App-specific password
  MAC_NOTARIZE_TEAM_ID     Team ID for notarization
  UE5_ROOT                 Path to UE5 installation

Examples:
  # Build only (no signing)
  ./build_macos.sh build --skip-sign

  # Full release with signing
  ./build_macos.sh release

  # Build and package without notarization
  ./build_macos.sh release --skip-notarize

  # Sign an existing build
  ./build_macos.sh sign

  # Create DMG from signed build
  ./build_macos.sh package

  # Verify signatures
  ./build_macos.sh verify
HELP
}

# ==============================================================================
# MAIN
# ==============================================================================
main() {
    # Shift past command for argument parsing
    local cmd="${1:-help}"
    if [ $# -gt 0 ]; then
        parse_args "$@"
    fi

    log_info "${PROJECT_DISPLAY_NAME} - macOS Build System"
    log_info "Project: ${PROJECT_DIR}"
    echo ""

    case "${cmd}" in
        build)
            cmd_build
            ;;
        sign)
            cmd_sign
            ;;
        notarize)
            cmd_notarize
            ;;
        package)
            cmd_package
            ;;
        release)
            cmd_release
            ;;
        verify)
            cmd_verify
            ;;
        clean)
            cmd_clean
            ;;
        help|--help|-h)
            cmd_help
            ;;
        *)
            log_error "Unknown command: ${cmd}"
            cmd_help
            exit 1
            ;;
    esac
}

main "$@"
