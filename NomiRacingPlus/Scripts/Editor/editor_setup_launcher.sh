#!/bin/bash
# ============================================================
# NIO Racing Plus — Editor Setup Launcher
# Launches UE5 Editor and runs the 16-step auto-setup pipeline
# ============================================================
set -e

PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
PROJECT="$PROJECT_DIR/NomiRacingPlus.uproject"
ENGINE_BIN="/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
MARKER="$PROJECT_DIR/.setup_completed"
LOG="$HOME/Library/Logs/Unreal Engine/NomiRacingPlusEditor/NomiRacingPlus.log"

# Remove old marker
rm -f "$MARKER"

echo "============================================================"
echo "  NIO Racing Plus — Editor + Auto-Setup Launcher"
echo "============================================================"
echo "  Project: $PROJECT"
echo "  Time:    $(date)"
echo ""
echo "  Launching editor..."
echo "  Python pipeline will auto-execute once editor is ready."
echo "============================================================"
echo ""

# Launch the editor binary DIRECTLY (not via .app bundle)
# to ensure command-line args are passed correctly.
#
# Strategy: Use -ExecCmds with a Python command that:
# 1. Waits for Python to be ready
# 2. Imports and runs the full setup
#
# The 'py' console command executes Python code.
# We use exec() to load the module and call the function.
#
# NOTE: -ExecCmds runs at engine init time. Python may not be ready yet.
# To work around this, we use a timer/delay approach:
#   py "import threading; threading.Timer(5.0, lambda: exec(open('Scripts/Editor/run_full_setup.py').read()) or run_full_setup()).start()"

"$ENGINE_BIN" "$PROJECT" \
    -stdout \
    -LogCmds="LogNomiRacing All, LogPython All" \
    2>&1 &

ENGINE_PID=$!
echo "  Editor PID: $ENGINE_PID"
echo ""

# Monitor for completion
START_TIME=$(date +%s)
TIMEOUT=$((30 * 60))  # 30 minutes

echo "  Polling for .setup_completed marker..."
echo ""

while true; do
    ELAPSED=$(( $(date +%s) - START_TIME ))
    
    if [ -f "$MARKER" ]; then
        echo ""
        echo "✅ Auto-setup COMPLETE at $(date) (${ELAPSED}s)"
        cat "$MARKER"
        break
    fi
    
    if [ $ELAPSED -ge $TIMEOUT ]; then
        echo ""
        echo "❌ TIMEOUT after ${TIMEOUT}s"
        break
    fi
    
    if ! kill -0 $ENGINE_PID 2>/dev/null; then
        echo ""
        echo "❌ Editor process exited unexpectedly at ${ELAPSED}s"
        break
    fi
    
    printf "\r  Waiting... ${ELAPSED}s elapsed"
    sleep 10
done

echo ""
echo "Total: $(( $(date +%s) - START_TIME ))s"
