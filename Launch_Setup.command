#!/bin/bash
# ============================================================
# NIO Racing Plus — UE5 Editor Launch Script
# ============================================================
# Launches UE5 5.7 Editor with the NomiRacingPlus project.
# The auto_setup.py in Content/Python/Startup/ will:
#   1. Check .setup_completed marker (skips if done)
#   2. Run the full 16-step pipeline
#   3. Create .setup_completed when done
#
# Usage:
#   Double-click this file in Finder, OR
#   ./Launch_Setup.command
# ============================================================

# Get the directory where this script is located
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT="$DIR/NomiRacingPlus/NomiRacingPlus.uproject"
ENGINE="/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor.app"

echo ""
echo "============================================================"
echo "  NIO Racing Plus — UE5 Editor Launcher"
echo "============================================================"
echo "  Project: $PROJECT"
echo "  Engine:  $ENGINE"
echo ""
echo "  The auto-setup pipeline will run automatically."
echo "  Check UE5 Editor Output Log for progress."
echo ""
echo "  When setup completes, '.setup_completed' will be created."
echo "============================================================"
echo ""

# Remove old marker to force re-run
if [ -f "$DIR/NomiRacingPlus/.setup_completed" ]; then
    rm "$DIR/NomiRacingPlus/.setup_completed"
    echo "Removed old .setup_completed — pipeline will re-run."
fi

# Launch the UE5 Editor
open "$ENGINE" --args "$PROJECT"

echo ""
echo "Editor launching... waiting for setup to complete."
echo "The Terminal will close automatically when setup finishes."
echo ""

