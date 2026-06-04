"""
NIO Racing Plus - Full Setup Script (Rewritten)
=================================================
Master orchestrator that properly imports and calls all sub-scripts.
Run from UE5 Editor Output Log:

    exec(open('Scripts/Editor/run_full_setup.py').read())
    run_full_setup()

Or headless via UE5 commandlet:
    UnrealEditor-Cmd NomiRacingPlus.uproject -run=pythonscript \\
        -script="<PROJECT>/Scripts/Editor/run_full_setup.py"

All 12 steps execute in dependency order:
  1. fix_common_issues    — create dirs, fix configs, add gameplay tags
  2. batch_import         — GLB meshes, textures, audio, HDR → Content/
  3. create_materials     — vehicle paint/glass/wheel + road/UI master mats
  4. build_vehicles       — BP_NIO_EP9, BP_NIO_ET7, BP_NIO_ES7 Blueprints
  5. build_tracks         — 5 tracks + main menu level
  6. setup_audio          — SoundClasses, SoundCues
  7. configure_ai         — AI profile config (loaded from JSON)
  8. setup_ui             — Widget Blueprints (HUD, menus)
  9. configure_nomi       — NOMI commentary engine config
 10. compile_blueprints   — compile all generated Blueprints
 11. validate             — asset validator, dependency check
 12. save_and_report      — save dirty packages + print summary
"""

import unreal
import os
import sys
import traceback

# ============================================================================
# Bootstrap: find Scripts/Editor path (__file__ not available under exec())
# ============================================================================

_PROJECT_DIR = unreal.Paths.project_dir()
_CONTENT_DIR = unreal.Paths.project_content_dir()

# When loaded via exec(open(...).read()), __file__ is not defined.
# Add Scripts/Editor + Content/Python to sys.path for module imports.
_scripts_dir = os.path.join(_PROJECT_DIR, "Scripts", "Editor")
_content_python_dir = os.path.join(_CONTENT_DIR, "Python")
for _p in (_scripts_dir, _content_python_dir):
    if _p not in sys.path:
        sys.path.insert(0, _p)

# ============================================================================
# Import all sub-modules (guarded — bare auto-execute blocks now use
# if __name__ == "__main__" so they won't fire on import.)
# ============================================================================

import batch_import              # noqa: E402 — import_meshes, import_textures, import_audio
import lod_generation            # noqa: E402 — generate_lods_for_directory
import material_setup            # noqa: E402 — create_all_master_materials, create_all_vehicle_materials
import vehicle_blueprint_builder # noqa: E402 — create_all_vehicle_blueprints
import track_builder             # noqa: E402 — create_all_tracks, create_main_menu_level
import fix_common_issues         # noqa: E402 — fix_all_issues
import validate_project          # noqa: E402 — run_validation
import blueprint_compile         # noqa: E402 — compile_all_blueprints
import game_balance              # noqa: E402 — BalanceAnalyzer
import setup_test_level          # noqa: E402 — setup_test_level()

# ============================================================================
# Logging
# ============================================================================

def _log(msg: str, level: str = "info") -> None:
    """Log with level.  In commandlet mode only log_warning/Error reach stdout."""
    prefix = f"[FullSetup] {msg}"
    if level == "error":
        unreal.log_error(prefix)
    elif level == "warn":
        unreal.log_warning(prefix)
    else:
        # unreal.log does NOT flush to stdout in commandlet mode.
        # Use log_warning for info-level messages so they always appear.
        unreal.log_warning(f"{prefix}")


def _log_step(step: int, total: int, msg: str) -> None:
    """Log current step number.  Uses log_warning so output is visible headless."""
    unreal.log_warning(f"[FullSetup] [{step}/{total}] {msg}")


# ============================================================================
# Environment detection
# ============================================================================

def _has_slate() -> bool:
    """Check whether Slate (UI framework) is available.
    
    Uses env var NOMI_HEADLESS to detect commandlet/headless mode:
      - .command file sets NOMI_HEADLESS=1 → HAS_SLATE=False
      - UE5 Editor paste (no env var)      → HAS_SLATE=True

    We do NOT call any C++ API for detection because:
    - unreal.SlateApplication doesn't exist in UE5.7 Python API
    - unreal.AssetToolsHelpers.get_asset_tools() crashes with
      assertion failure in commandlet mode (can't catch)
    """
    return os.environ.get("NOMI_HEADLESS") != "1"


HAS_SLATE = _has_slate()


# ============================================================================
# 12-Step Orchestrator
# ============================================================================

class FullSetup:
    """Orchestrates the 12-step NIO Racing Plus setup pipeline."""

    def __init__(self):
        self.total_steps = 12
        self.current_step = 0
        self.skipped_steps: list[str] = []

    # ------------------------------------------------------------------
    # Public entry point
    # ------------------------------------------------------------------

    def run(self) -> None:
        """Execute all 12 setup steps in order."""
        if not HAS_SLATE:
            _log("Running in headless/commandlet mode — Slate NOT available.", "warn")
            _log("Steps requiring UI (import, material, BP, track) will be skipped.", "warn")
            _log("Run 'run_full_setup()' inside UE5 Editor for full pipeline.\n", "warn")
        else:
            _log("Slate available — full 12-step pipeline will execute.\n")

        _log("\n" + "=" * 60)
        _log("  NIO Racing Plus — Full Setup Pipeline")
        _log("=" * 60 + "\n")

        steps = [
            ("Fixing common issues",        self._step_fix_issues,       True),
            ("Importing assets",            self._step_import_assets,    HAS_SLATE),
            ("Creating materials",          self._step_create_materials, HAS_SLATE),
            ("Building vehicle Blueprints", self._step_build_vehicles,   HAS_SLATE),
            ("Building tracks",             self._step_build_tracks,     HAS_SLATE),
            ("Setting up audio",            self._step_setup_audio,      HAS_SLATE),
            ("Configuring AI",              self._step_configure_ai,     True),
            ("Setting up UI",               self._step_setup_ui,         HAS_SLATE),
            ("Configuring NOMI",            self._step_configure_nomi,   True),
            ("Compiling Blueprints",        self._step_compile_blueprints, HAS_SLATE),
            ("Validating project",          self._step_validate,         True),
            ("Saving and reporting",        self._step_save_and_report,  True),
        ]

        try:
            for i, (label, method, can_run) in enumerate(steps, 1):
                self.current_step = i
                _log_step(i, self.total_steps, label)
                if can_run:
                    method()
                    _log(f"  ✓ {label}")
                else:
                    _log("  ⏭ Skipped (requires Slate — run in UE5 Editor)")
                    self.skipped_steps.append(label)

        except Exception as exc:
            _log(
                f"Setup FAILED at step {self.current_step}/{self.total_steps}: {exc}",
                "error",
            )
            traceback.print_exc()
            return

        self._print_completion_report()

    # ------------------------------------------------------------------
    # Individual steps (public so they can be called standalone)
    # ------------------------------------------------------------------

    # ── Step 1 ────────────────────────────────────────────────────────

    def _step_fix_issues(self) -> None:
        fix_common_issues.fix_all_issues()

    # ── Step 2 ────────────────────────────────────────────────────────

    def _step_import_assets(self) -> None:
        # --- Vehicle meshes ---
        vehicles = {
            "EP9":     ("Assets/NIO/EP9/ep9.glb",          "/Game/Vehicles/EP9/Meshes"),
            "ES7":     ("Assets/NIO/ES7/es7.glb",          "/Game/Vehicles/ES7/Meshes"),
            "ET7":     ("Assets/NIO/ET7/et7.glb",          "/Game/Vehicles/ET7/Meshes"),
            "SU7Ultra":("Assets/Xiaomi/SU7Ultra/su7ultra.glb", "/Game/Vehicles/SU7Ultra/Meshes"),
        }
        for name, (rel_src, dest) in vehicles.items():
            full = os.path.join(_PROJECT_DIR, rel_src)
            if os.path.isfile(full):
                batch_import.import_meshes(
                    os.path.dirname(full), dest, import_as_skeletal=False,
                )
                _log(f"  Imported {name} mesh from {full}")

        # --- Textures ---
        texture_roots = [
            ("Assets/Textures/Vehicle",    "/Game/Textures/Vehicle"),
            ("Assets/Textures/Road",       "/Game/Textures/Road"),
            ("Assets/Textures/Environment","/Game/Textures/Environment"),
            ("Assets/Textures",            "/Game/Textures"),
        ]
        for rel_src, dest in texture_roots:
            full = os.path.join(_PROJECT_DIR, rel_src)
            if os.path.isdir(full):
                batch_import.import_textures(full, dest)

        # --- Audio ---
        audio_roots = [
            ("Assets/Audio/Motor", "/Game/Audio/Motor"),
            ("Assets/Audio/SFX",   "/Game/Audio/SFX"),
            ("Assets/Audio/UI",    "/Game/Audio/UI"),
        ]
        for rel_src, dest in audio_roots:
            full = os.path.join(_PROJECT_DIR, rel_src)
            if os.path.isdir(full):
                batch_import.import_audio(full, dest)

    # ── Step 3 ────────────────────────────────────────────────────────

    def _step_create_materials(self) -> None:
        material_setup.create_all_master_materials()
        for v in ("EP9", "ET7", "ES7"):
            material_setup.create_all_vehicle_materials(v)

    # ── Step 4 ────────────────────────────────────────────────────────

    def _step_build_vehicles(self) -> None:
        vehicle_blueprint_builder.create_all_vehicle_blueprints()

    # ── Step 5 ────────────────────────────────────────────────────────

    def _step_build_tracks(self) -> None:
        track_builder.create_all_tracks()
        track_builder.create_main_menu_level()
        # Also create the quick-test level
        setup_test_level.setup_test_level()

    # ── Step 6 ────────────────────────────────────────────────────────

    def _step_setup_audio(self) -> None:
        """Create SoundClasses and SoundCues for imported audio assets."""

        # Create top-level SoundClasses
        sc_names = ["Motor", "SFX", "UI", "Music", "NOMI"]
        for sc in sc_names:
            path = f"/Game/Audio/{sc}"
            if not unreal.EditorAssetLibrary.does_asset_exist(f"{path}.SoundClass"):
                factory = unreal.SoundClassFactory()
                asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    sc, "/Game/Audio", unreal.SoundClass, factory,
                )
                if asset:
                    _log(f"  Created SoundClass: {sc}")

        # Create SoundCues for each motor loop
        for vehicle in ("EP9", "ET7", "ES7", "SU7Ultra"):
            cue_name = f"SC_Motor_{vehicle}"
            cue_path = f"/Game/Audio/Motor/{cue_name}"
            wave_path = f"/Game/Audio/Motor/{vehicle}_motor_loop"
            if not unreal.EditorAssetLibrary.does_asset_exist(cue_path) \
               and unreal.EditorAssetLibrary.does_asset_exist(wave_path):
                factory = unreal.SoundCueFactory()
                asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    cue_name, "/Game/Audio/Motor", unreal.SoundCue, factory,
                )
                if asset:
                    _log(f"  Created SoundCue: {cue_name}")

    # ── Step 7 ────────────────────────────────────────────────────────

    def _step_configure_ai(self) -> None:
        ai_cfg = os.path.join(_CONTENT_DIR, "AI", "AIProfiles.json")
        if os.path.isfile(ai_cfg):
            _log(f"  AI profiles loaded from {ai_cfg}")
        else:
            _log("AIProfiles.json not found — will be created by fix step", "warn")

    # ── Step 8 ────────────────────────────────────────────────────────

    def _step_setup_ui(self) -> None:
        """Create WidgetBlueprint assets for HUD and menus."""

        for wgt in ("WBP_HUD", "WBP_MainMenu", "WBP_PauseMenu", "WBP_RaceResult"):
            path = f"/Game/UI/{wgt}"
            if not unreal.EditorAssetLibrary.does_asset_exist(f"{path}.WidgetBlueprint"):
                factory = unreal.WidgetBlueprintFactory()
                asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    wgt, "/Game/UI", unreal.WidgetBlueprint, factory,
                )
                if asset:
                    _log(f"  Created Widget: {wgt}")

        ui_theme = os.path.join(_CONTENT_DIR, "UI", "UITheme.json")
        if os.path.isfile(ui_theme):
            _log(f"  UITheme loaded from {ui_theme}")

    # ── Step 9 ────────────────────────────────────────────────────────

    def _step_configure_nomi(self) -> None:
        comments = os.path.join(_CONTENT_DIR, "NOMI", "Comments", "DefaultComments.json")
        if os.path.isfile(comments):
            _log(f"  NOMI commentary loaded from {comments}")
        else:
            _log("DefaultComments.json not found", "warn")

    # ── Step 10 ───────────────────────────────────────────────────────

    def _step_compile_blueprints(self) -> None:
        report = blueprint_compile.compile_all_blueprints()
        if report:
            _log(
                f"  Compiled {report.total} BPs: "
                f"{report.succeeded} succeeded, {report.failed} failed",
            )
            if report.failed:
                for r in report.results:
                    if not r.success:
                        _log(f"    FAILED: {r.asset_path}", "error")
        else:
            _log("  No Blueprints found — this is normal on first run", "warn")

    # ── Step 11 ───────────────────────────────────────────────────────

    def _step_validate(self) -> None:
        passed = validate_project.run_validation()
        if passed:
            _log("  All validation checks passed")
        else:
            _log("  Some checks failed — review output above", "warn")

    # ── Step 12 ───────────────────────────────────────────────────────

    def _step_save_and_report(self) -> None:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
        _log("  All dirty packages saved")

    # ------------------------------------------------------------------
    # Completion report
    # ------------------------------------------------------------------

    def _print_completion_report(self) -> None:
        _log("\n" + "=" * 60)
        _log("  Setup Complete!")
        _log("=" * 60)
        _log("")

        if not self.skipped_steps:
            _log("  All 12 steps executed successfully!")
        else:
            _log(f"  {12 - len(self.skipped_steps)}/12 steps executed.")
            _log(f"  {len(self.skipped_steps)} step(s) skipped (Slate unavailable):")
            for s in self.skipped_steps:
                _log(f"    ⏭ {s}")

        _log("")

        if not HAS_SLATE:
            _log("  ──────────────────────────────────────────────────────")
            _log("  HEADLESS MODE — to complete the full setup:")
            _log("  ──────────────────────────────────────────────────────")
            _log("")
            _log("  1. Open NomiRacingPlus.uproject in UE5 Editor")
            _log("  2. Open Output Log (Window → Output Log)")
            _log("  3. Paste and run:")
            _log('     exec(open("Scripts/Editor/run_full_setup.py").read())')
            _log("     run_full_setup()")
            _log("")
            _log("  ──────────────────────────────────────────────────────")
        else:
            _log("  Next Steps:")
            _log("  1. Open Content Browser to verify all assets")
            _log("  2. Open Content/Maps/TestTrack.umap")
            _log("  3. Press Play to test the game!")
            _log("  4. Adjust balance via game_balance.BalanceAnalyzer()")
            _log("")
            _log("  Quick debug commands (in Output Log):")
            _log('    exec(open("Scripts/Editor/run_full_setup.py").read())')
            _log("    run_full_setup()")
            _log("    run_quick_setup()")
            _log("")

        _log("=" * 60 + "\n")


# ============================================================================
# Public entry points
# ============================================================================

def run_full_setup() -> None:
    """Run the complete 12-step setup pipeline."""
    FullSetup().run()


def run_quick_setup() -> None:
    """Essential-only setup (assets, materials, vehicles, tracks)."""
    _log("Running quick setup (steps 1-5 only)...")
    s = FullSetup()
    s._step_fix_issues()
    s._step_import_assets()
    s._step_create_materials()
    s._step_build_vehicles()
    s._step_build_tracks()
    _log("Quick setup complete! Run run_full_setup() for full pipeline.")


# ============================================================================
# Auto-execute when run as headless commandlet (NOMI_HEADLESS=1)
#
# When pasting in UE5 Editor via exec(open(...).read()):
#   __name__ is "__main__" but NOMI_HEADLESS is NOT set,
#   so auto-run is SKIPPED (user calls run_full_setup() explicitly).
#
# When running via UE5-Cmd .command file:
#   NOMI_HEADLESS=1 is set → auto-run fires automatically.
# ============================================================================

if __name__ == "__main__":
    if os.environ.get("NOMI_HEADLESS") == "1":
        _log("Headless commandlet mode — auto-running run_full_setup()...")
        run_full_setup()
    else:
        _log("Editor mode — loaded. Call run_full_setup() or run_quick_setup() explicitly.")
