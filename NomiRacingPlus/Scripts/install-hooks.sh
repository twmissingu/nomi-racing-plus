#!/bin/bash
# ==============================================================================
# NIO Racing Plus - Git Hook Installer
# ==============================================================================
# Installs git hooks for automated pre-commit verification.
#
# Usage:
#   ./Scripts/install-hooks.sh          # Install hooks
#   ./Scripts/install-hooks.sh --remove # Remove hooks
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
GIT_DIR="${PROJECT_DIR}/.git"
HOOKS_DIR="${GIT_DIR}/hooks"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create pre-commit hook
create_pre_commit_hook() {
    local hook_path="${HOOKS_DIR}/pre-commit"

    cat > "${hook_path}" << 'HOOK'
#!/bin/bash
# NIO Racing Plus - Pre-commit Hook
# Runs fast verification before each commit.
# Bypass with: git commit --no-verify

set -euo pipefail

# Get the project root (parent of .git)
GIT_DIR="$(git rev-parse --git-dir)"
PROJECT_DIR="$(cd "${GIT_DIR}/.." && pwd)"
VERIFY_SCRIPT="${PROJECT_DIR}/Scripts/verify.sh"

if [ ! -f "${VERIFY_SCRIPT}" ]; then
    echo "[WARN] verify.sh not found, skipping pre-commit checks"
    exit 0
fi

echo "Running pre-commit verification..."
"${VERIFY_SCRIPT}" --quick

exit_code=$?

if [ ${exit_code} -ne 0 ]; then
    echo ""
    echo "Pre-commit checks failed. Fix the issues above and try again."
    echo "To bypass (not recommended): git commit --no-verify"
    exit 1
fi
HOOK

    chmod +x "${hook_path}"
    log_info "Created pre-commit hook: ${hook_path}"
}

# Remove pre-commit hook
remove_pre_commit_hook() {
    local hook_path="${HOOKS_DIR}/pre-commit"

    if [ -f "${hook_path}" ]; then
        # Only remove if it's our hook
        if grep -q "NIO Racing Plus" "${hook_path}" 2>/dev/null; then
            rm "${hook_path}"
            log_info "Removed pre-commit hook"
        else
            log_warn "Pre-commit hook exists but was not created by this script"
        fi
    else
        log_info "No pre-commit hook to remove"
    fi
}

# Main
main() {
    local action="install"

    while [[ $# -gt 0 ]]; do
        case $1 in
            --remove)
                action="remove"
                shift
                ;;
            --help|-h)
                echo "NIO Racing Plus - Git Hook Installer"
                echo ""
                echo "Usage: $0 [options]"
                echo ""
                echo "Options:"
                echo "  (default)     Install pre-commit hook"
                echo "  --remove      Remove pre-commit hook"
                echo "  --help        Show this help"
                exit 0
                ;;
            *)
                shift
                ;;
        esac
    done

    # Check if we're in a git repo
    if [ ! -d "${GIT_DIR}" ]; then
        log_error "Not a git repository: ${PROJECT_DIR}"
        exit 1
    fi

    # Create hooks directory if needed
    mkdir -p "${HOOKS_DIR}"

    case ${action} in
        install)
            log_info "Installing git hooks..."
            create_pre_commit_hook
            log_info "Done! Pre-commit hook installed."
            log_info "Bypass with: git commit --no-verify"
            ;;
        remove)
            log_info "Removing git hooks..."
            remove_pre_commit_hook
            log_info "Done!"
            ;;
    esac
}

main "$@"
