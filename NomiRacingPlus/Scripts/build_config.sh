#!/bin/bash
# ==============================================================================
# NIO Racing Plus - Build Configuration (Shared)
# ==============================================================================
# This file contains all build configuration variables.
# Source this file from both build_macos.sh and CI/CD workflows.
# ==============================================================================

# --- Project Settings ---
export PROJECT_NAME="NomiRacingPlus"
export PROJECT_VERSION="1.0.0"
export PROJECT_DISPLAY_NAME="NIO Racing Plus"
export PROJECT_BUNDLE_ID="com.nio.racingplus"
export PROJECT_COMPANY="NIO Racing Plus"
export PROJECT_COPYRIGHT="Copyright 2026 NIO Racing Plus. All rights reserved."

# --- Build Defaults ---
export DEFAULT_BUILD_TYPE="Shipping"
export DEFAULT_PLATFORM_MAC="Mac"
export DEFAULT_PLATFORM_WIN="Win64"

# --- Directory Layout (relative to project root) ---
export BUILD_DIR_NAME="Build"
export ARCHIVE_DIR_NAME="Archive"
export PACKAGES_DIR_NAME="Packages"
export LOGS_DIR_NAME="Logs"
export SIGNING_DIR_NAME="Signing"

# --- macOS Signing ---
# Code signing identity (Developer ID Application)
# Set via environment: MAC_SIGNING_IDENTITY
# Example: "Developer ID Application: Your Name (TEAM_ID)"
export MAC_SIGNING_IDENTITY="${MAC_SIGNING_IDENTITY:-}"

# Apple Team ID
export MAC_TEAM_ID="${MAC_TEAM_ID:-}"

# Notarization credentials (set via environment or keychain)
export MAC_NOTARIZE_APPLE_ID="${MAC_NOTARIZE_APPLE_ID:-}"
export MAC_NOTARIZE_PASSWORD="${MAC_NOTARIZE_PASSWORD:-}"
export MAC_NOTARIZE_TEAM_ID="${MAC_NOTARIZE_TEAM_ID:-}"

# Keychain profile for notarization (stored via `xcrun notarytool store-credentials`)
export MAC_NOTARIZE_KEYCHAIN_PROFILE="${MAC_NOTARIZE_KEYCHAIN_PROFILE:-notarytool-profile}"

# DMG settings
export DMG_VOLUME_NAME="${PROJECT_DISPLAY_NAME} ${PROJECT_VERSION}"
export DMG_BACKGROUND_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Windows Signing ---
# Path to PFX certificate file
export WIN_SIGNING_CERT="${WIN_SIGNING_CERT:-}"

# Certificate password (set via environment)
export WIN_SIGNING_PASSWORD="${WIN_SIGNING_PASSWORD:-}"

# Timestamp server for signing
export WIN_TIMESTAMP_SERVER="${WIN_TIMESTAMP_SERVER:-http://timestamp.digicert.com}"

# SignTool path (auto-detected from Windows SDK)
export WIN_SIGNTOOL_PATH="${WIN_SIGNTOOL_PATH:-}"

# --- Windows Packaging ---
# Installer framework: "nsis" or "inno"
export WIN_INSTALLER_FRAMEWORK="${WIN_INSTALLER_FRAMEWORK:-nsis}"

# NSIS path
export WIN_NSIS_PATH="${WIN_NSIS_PATH:-C:/Program Files (x86)/NSIS}"

# Inno Setup path
export WIN_INNO_PATH="${WIN_INNO_PATH:-C:/Program Files (x86)/Inno Setup 6}"

# --- Distribution ---
# GitHub release tag prefix
export RELEASE_TAG_PREFIX="v"

# Distribution channels
export DIST_CHANNEL="${DIST_CHANNEL:-github}"

# --- Logging ---
export LOG_LEVEL="${LOG_LEVEL:-INFO}"  # DEBUG, INFO, WARN, ERROR

# --- Helper Functions ---

# Resolve absolute paths from project root
resolve_project_root() {
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[1]:-${BASH_SOURCE[0]}}")" && pwd)"
    echo "$(cd "${script_dir}/.." && pwd)"
}

# Get build directory path
get_build_dir() {
    local project_root="$1"
    echo "${project_root}/${BUILD_DIR_NAME}"
}

# Get archive directory path
get_archive_dir() {
    local project_root="$1"
    echo "${project_root}/${BUILD_DIR_NAME}/${ARCHIVE_DIR_NAME}"
}

# Get packages directory path
get_packages_dir() {
    local project_root="$1"
    echo "${project_root}/${BUILD_DIR_NAME}/${PACKAGES_DIR_NAME}"
}

# Get logs directory path
get_logs_dir() {
    local project_root="$1"
    echo "${project_root}/${BUILD_DIR_NAME}/${LOGS_DIR_NAME}"
}

# Validate required signing environment variables
validate_mac_signing_env() {
    local errors=0
    if [ -z "${MAC_SIGNING_IDENTITY}" ]; then
        echo "[ERROR] MAC_SIGNING_IDENTITY is not set"
        errors=$((errors + 1))
    fi
    if [ -z "${MAC_TEAM_ID}" ]; then
        echo "[WARN] MAC_TEAM_ID is not set (required for notarization)"
    fi
    return ${errors}
}

validate_win_signing_env() {
    local errors=0
    if [ -z "${WIN_SIGNING_CERT}" ]; then
        echo "[ERROR] WIN_SIGNING_CERT is not set"
        errors=$((errors + 1))
    elif [ ! -f "${WIN_SIGNING_CERT}" ]; then
        echo "[ERROR] WIN_SIGNING_CERT file not found: ${WIN_SIGNING_CERT}"
        errors=$((errors + 1))
    fi
    if [ -z "${WIN_SIGNING_PASSWORD}" ]; then
        echo "[ERROR] WIN_SIGNING_PASSWORD is not set"
        errors=$((errors + 1))
    fi
    return ${errors}
}
