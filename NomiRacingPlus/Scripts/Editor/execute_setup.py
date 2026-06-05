"""
NIO Racing Plus - Execute Setup Script
=======================================
This script is designed to run via:
  -ExecutePythonScript="Scripts/Editor/execute_setup.py"

It waits for the editor to be ready, then runs the full 16-step pipeline.
"""

import unreal
import os
import sys
import json
import time
import traceback

# ── Configuration ──────────────────────────────────────────────────────────

_MARKER_PATH = os.path.normpath(os.path.join(
    unreal.Paths.project_content_dir(), "..", ".setup_completed"
))
_SCRIPTS_DIR = os.path.join(unreal.Paths.project_dir(), "Scripts", "Editor")


def log(msg: str):
    unreal.log_warning(f"[SetupLauncher] {msg}")


def is_asset_tools_ready() -> bool:
    """Check if editor is fully initialized with AssetTools."""
    try:
        tools = unreal.AssetToolsHelpers.get_asset_tools()
        return tools is not None
    except Exception as e:
        log(f"AssetTools not ready: {e}")
        return False


def mark_done():
    with open(_MARKER_PATH, "w") as f:
        json.dump({"completed": True, "version": 1, "timestamp": time.time()}, f)
    log(f"Created .setup_completed marker at {_MARKER_PATH}")


def main():
    log("=" * 60)
    log("  Setup Launcher started")
    log(f"  Time: {time.ctime()}")
    log("=" * 60)

    # Check if already completed
    if os.path.exists(_MARKER_PATH):
        log("Setup already completed. Skipping.")
        return

    # Wait for editor readiness
    log("Waiting for AssetTools to be ready...")
    max_retries = 60  # 5 minutes at 5s intervals
    for attempt in range(1, max_retries + 1):
        if is_asset_tools_ready():
            log(f"AssetTools ready after ~{attempt * 5}s")
            break
        if attempt % 6 == 0:
            log(f"Still waiting... ({attempt * 5}s elapsed)")
        time.sleep(5)
    else:
        log(f"ERROR: AssetTools not ready after {max_retries * 5}s")
        return

    # Add Scripts/Editor to sys.path and import
    if _SCRIPTS_DIR not in sys.path:
        sys.path.insert(0, _SCRIPTS_DIR)

    log("Importing run_full_setup...")
    try:
        import run_full_setup
        log("Calling run_full_setup.run_full_setup()...")
        run_full_setup.run_full_setup()
        mark_done()
        log("=" * 60)
        log("  FULL SETUP COMPLETE!")
        log("=" * 60)
    except Exception as e:
        log(f"SETUP FAILED: {e}")
        traceback.print_exc()


if __name__ == "__main__":
    main()
