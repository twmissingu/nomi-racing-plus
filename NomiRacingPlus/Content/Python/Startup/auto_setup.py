"""
NIO Racing Plus - Auto Full Setup on Editor Launch
====================================================
Runs the complete 12-step pipeline automatically when the UE5 Editor opens.
Creates a marker file after completion so it only runs once.

Place in Content/Python/Startup/ to auto-run on editor startup.
"""

import unreal
import os
import sys
import json
import traceback

# ---------------------------------------------------------------------------
# Marker file — skip if setup was already completed
# ---------------------------------------------------------------------------

_MARKER_PATH = os.path.normpath(os.path.join(
    unreal.Paths.project_content_dir(), "..", ".setup_completed"
))


def _is_setup_done() -> bool:
    return os.path.exists(_MARKER_PATH)


def _mark_setup_done() -> None:
    with open(_MARKER_PATH, "w") as f:
        json.dump({"completed": True, "version": 1}, f)


# ---------------------------------------------------------------------------
# Bootstrap the orchestrator
# ---------------------------------------------------------------------------

_PROJECT_DIR = unreal.Paths.project_dir()
_SCRIPTS_DIR = os.path.join(_PROJECT_DIR, "Scripts", "Editor")


def _run_full_pipeline():
    """Import and run the 12-step orchestrator inside the editor."""
    unreal.log_warning("=" * 60)
    unreal.log_warning("[NIO Racing] Auto Setup: running full pipeline...")
    unreal.log_warning("[NIO Racing] Watch the Output Log for progress.")
    unreal.log_warning("=" * 60)

    # Add Scripts/Editor to sys.path so we can import modules
    if _SCRIPTS_DIR not in sys.path:
        sys.path.insert(0, _SCRIPTS_DIR)

    # Import and run
    import run_full_setup

    try:
        run_full_setup.run_full_setup()
        _mark_setup_done()
        unreal.log_warning("=" * 60)
        unreal.log_warning("[NIO Racing] Auto Setup COMPLETE!")
        unreal.log_warning("[NIO Racing] Open Content/Maps/TestTrack.umap and press Play!")
        unreal.log_warning("=" * 60)
    except Exception as exc:
        unreal.log_error(f"[NIO Racing] Auto Setup FAILED: {exc}")
        traceback.print_exc()


# ---------------------------------------------------------------------------
# Entry point (with retry for editor startup timing)
# ---------------------------------------------------------------------------

def _asset_tools_ready() -> bool:
    """Check if AssetTools are available (Editor fully initialized)."""
    try:
        tools = unreal.AssetToolsHelpers.get_asset_tools()
        return tools is not None
    except Exception:
        return False


def check_and_setup():
    if _is_setup_done():
        unreal.log("[NIO Racing] Setup already completed. Skipping.")
        return

    # Detect headless/commandlet mode — skip if no AssetTools
    if not _asset_tools_ready():
        unreal.log_warning(
            "[NIO Racing] AssetTools not available (commandlet mode?). "
            "Auto setup will run when Editor opens."
        )
        return

    unreal.log_warning(
        "[NIO Racing] First launch detected — running full setup pipeline..."
    )
    _run_full_pipeline()


check_and_setup()
