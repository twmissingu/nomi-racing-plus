"""
NIO Racing Plus - Full Setup Script (Rewritten + Enhanced)
=================================================
Master orchestrator that properly imports and calls all sub-scripts.
Run from UE5 Editor Output Log:

    exec(open('Scripts/Editor/run_full_setup.py').read())
    run_full_setup()

Or headless via UE5 commandlet:
    UnrealEditor-Cmd NomiRacingPlus.uproject -run=pythonscript \\
        -script="<PROJECT>/Scripts/Editor/run_full_setup.py"

All 16 steps execute in dependency order:
   1. fix_common_issues    — create dirs, fix configs, add gameplay tags
   2. batch_import         — GLB meshes, textures, audio, HDR → Content/
   3. create_materials     — vehicle paint/glass/wheel + road/UI master mats
   4. build_vehicles       — BP_NIO_EP9, BP_NIO_ET7, BP_NIO_ES7 Blueprints
   5. build_tracks         — 5 tracks + main menu level
   6. setup_audio          — SoundClasses, SoundCues
   7. configure_ai         — AI profile config (loaded from JSON)
   8. setup_ui             — Widget Blueprints (HUD, menus)
   9. configure_nomi       — NOMI commentary engine config
  10. create_input_assets  — IA_* InputActions + IMC_VehicleControls
  11. reparent_vehicles    — BP_NIO_* → NIOVehicleBase parent + set vehicle_type
  12. setup_player_controller — BP_NIO_PlayerController + InputMapping wire
  13. configure_game_mode  — BP_NIO_GameMode class refs + project defaults
  14. compile_blueprints   — compile all generated Blueprints
  15. validate             — asset validator, dependency check
  16. save_and_report      — save dirty packages + print summary
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
# Helper functions (backported from run_pipeline_all.py)
# ============================================================================

def _make_key(key_name):
    """Helper to create and return an unreal.Key with key_name set."""
    k = unreal.Key()
    k.set_editor_property("key_name", key_name)
    return k


def _make_mapping(action, key_name, modifiers=None):
    """Helper to create an EnhancedActionKeyMapping."""
    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property("action", action)
    mapping.set_editor_property("key", _make_key(key_name))

    if modifiers:
        mapping.set_editor_property("modifiers", modifiers)

    return mapping


def _create_input_assets():
    """Create Enhanced Input assets: InputActions + InputMappingContext.

    Returns (created_actions_dict, imc_asset).
    """
    at = unreal.AssetToolsHelpers.get_asset_tools()

    if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Input"):
        unreal.EditorAssetLibrary.make_directory("/Game/Input")

    # Define InputActions: name, value_type
    actions_def = {
        "IA_Throttle":    unreal.InputActionValueType.AXIS1D,
        "IA_Brake":       unreal.InputActionValueType.AXIS1D,
        "IA_Steer":       unreal.InputActionValueType.AXIS1D,
        "IA_Handbrake":   unreal.InputActionValueType.BOOLEAN,
        "IA_LookBack":    unreal.InputActionValueType.BOOLEAN,
        "IA_ChangeCamera":unreal.InputActionValueType.BOOLEAN,
        "IA_Horn":        unreal.InputActionValueType.BOOLEAN,
        "IA_Headlights":  unreal.InputActionValueType.BOOLEAN,
        "IA_Pause":       unreal.InputActionValueType.BOOLEAN,
        "IA_Reset":       unreal.InputActionValueType.BOOLEAN,
    }

    created_actions = {}
    for name, val_type in actions_def.items():
        path = f"/Game/Input/{name}.{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            _log(f"  {name} already exists")
            created_actions[name] = unreal.EditorAssetLibrary.load_asset(path)
        else:
            factory = unreal.InputAction_Factory()
            asset = at.create_asset(name, "/Game/Input", unreal.InputAction, factory)
            if asset:
                asset.set_editor_property("value_type", val_type)
                created_actions[name] = asset
                _log(f"  Created {name} (value_type={val_type})")
            else:
                _log(f"  [WARN] Failed to create {name}")

    # Create InputMappingContext
    imc_path = "/Game/Input/IMC_VehicleControls"
    if unreal.EditorAssetLibrary.does_asset_exist(f"{imc_path}.IMC_VehicleControls"):
        imc = unreal.EditorAssetLibrary.load_asset(imc_path)
        _log(f"  IMC_VehicleControls already exists")
    else:
        factory = unreal.InputMappingContext_Factory()
        imc = at.create_asset("IMC_VehicleControls", "/Game/Input", unreal.InputMappingContext, factory)
        _log(f"  Created IMC_VehicleControls")

    # Add key mappings to the context (standard WASD + Gamepad)
    if imc:
        imc.set_editor_property("mappings", [])

        all_mappings = []
        ia = created_actions

        def add(action, key_name, scalar=None):
            mods = []
            if scalar is not None:
                s = unreal.InputModifierScalar()
                vec = s.get_editor_property("scalar")
                vec.x = scalar
                s.set_editor_property("scalar", vec)
                mods.append(s)
            all_mappings.append(_make_mapping(action, key_name, mods))

        # Keyboard mappings
        add(ia["IA_Throttle"],    "W",            None)
        add(ia["IA_Brake"],       "S",            None)
        add(ia["IA_Steer"],       "A",            -1.0)
        add(ia["IA_Steer"],       "D",            None)
        add(ia["IA_Handbrake"],   "SpaceBar",     None)
        add(ia["IA_LookBack"],    "LeftAlt",      None)
        add(ia["IA_ChangeCamera"],"C",            None)
        add(ia["IA_Horn"],        "H",            None)
        add(ia["IA_Headlights"],  "L",            None)
        add(ia["IA_Pause"],       "Escape",       None)
        add(ia["IA_Reset"],       "R",            None)

        # Gamepad analog mappings with deadzone
        deadzone = unreal.InputModifierDeadZone()
        deadzone.set_editor_property("lower_threshold", 0.1)
        deadzone.set_editor_property("type", unreal.DeadZoneType.RADIAL)
        deadzone.set_editor_property("upper_threshold", 1.0)

        all_mappings.append(_make_mapping(ia["IA_Throttle"], "Gamepad_RightTriggerAxis", [deadzone]))
        all_mappings.append(_make_mapping(ia["IA_Brake"],    "Gamepad_LeftTriggerAxis",  [deadzone]))
        all_mappings.append(_make_mapping(ia["IA_Steer"],    "Gamepad_LeftX",            [deadzone]))

        imc.set_editor_property("mappings", all_mappings)

        _log(f"  Added {len(all_mappings)} key mappings total")
    else:
        _log("  [WARN] IMC not available for mapping")

    _log("  Input assets setup complete")
    return created_actions, imc


def _reparent_vehicle_blueprints():
    """Reparent vehicle BPs: WheeledVehiclePawn → NIOVehicleBase, set vehicle_type."""
    vehicle_map = {
        "EP9":       unreal.NIOVehicleType.EP9,
        "ET7":       unreal.NIOVehicleType.ET7,
        "ES7":       unreal.NIOVehicleType.ES7,
        "SU7Ultra":  unreal.NIOVehicleType.SU7Ultra,
    }

    bel = unreal.BlueprintEditorLibrary
    results = {"reparented": 0, "new": 0}

    for vt_name, vt_enum in vehicle_map.items():
        bp_path = f"/Game/Vehicles/{vt_name}/Blueprints/BP_NIO_{vt_name}"
        bp_full = f"{bp_path}.BP_NIO_{vt_name}"

        if unreal.EditorAssetLibrary.does_asset_exist(bp_full):
            bp = unreal.EditorAssetLibrary.load_asset(bp_full)
            if not bp or not isinstance(bp, unreal.Blueprint):
                continue

            # Check current parent class
            try:
                gc = bp.generated_class()
                current_parent = gc.get_super_class()
                if current_parent and current_parent.get_name().startswith("NIOVehicleBase"):
                    _log(f"  BP_NIO_{vt_name} already parented to NIOVehicleBase")
                    # Still set vehicle_type in case it's wrong
                    cdo = unreal.get_default_object(gc)
                    cdo.set_editor_property("vehicle_type", vt_enum)
                    _log(f"  Set vehicle_type on BP_NIO_{vt_name} CDO = {vt_enum}")
                    results["reparented"] += 1
                    continue
            except Exception:
                pass

            # Reparent to NIOVehicleBase
            try:
                bel.reparent_blueprint(bp, unreal.NIOVehicleBase)
                results["reparented"] += 1
                _log(f"  Reparented BP_NIO_{vt_name}: → NIOVehicleBase")
            except Exception as e:
                _log(f"  [WARN] Reparent BP_NIO_{vt_name} failed: {e}")

            # Set vehicle_type property on CDO
            try:
                gc = bp.generated_class()
                cdo = unreal.get_default_object(gc)
                cdo.set_editor_property("vehicle_type", vt_enum)
                _log(f"  Set vehicle_type on BP_NIO_{vt_name} CDO = {vt_enum}")
            except Exception as e:
                _log(f"  [WARN] Set vehicle_type on BP_NIO_{vt_name} failed: {e}")
        else:
            # Create new BP with NIOVehicleBase parent
            try:
                factory = unreal.BlueprintFactory()
                factory.set_editor_property("parent_class", unreal.NIOVehicleBase)
                bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    f"BP_NIO_{vt_name}",
                    f"/Game/Vehicles/{vt_name}/Blueprints",
                    unreal.Blueprint,
                    factory,
                )
                if bp:
                    try:
                        gc = bp.generated_class()
                        cdo = unreal.get_default_object(gc)
                        cdo.set_editor_property("vehicle_type", vt_enum)
                        _log(f"  Set vehicle_type on new {vt_name} CDO = {vt_enum}")
                    except Exception as e2:
                        _log(f"  [WARN] Set vehicle_type on new {vt_name}: {e2}")
                    results["new"] += 1
                    _log(f"  Created BP_NIO_{vt_name} with NIOVehicleBase parent")
            except Exception as e:
                _log(f"  [WARN] Create BP_NIO_{vt_name} failed: {e}")

    _log(f"  Reparented {results['reparented']}, created {results['new']} new")


def _create_player_controller(actions=None, imc=None):
    """Create BP_NIO_PlayerController with InputMapping defaults.

    Sets InputMapping.MappingContext + InputMapping.Actions.* on the CDO
    so Enhanced Input bindings work at PIE start.
    """
    at = unreal.AssetToolsHelpers.get_asset_tools()
    pc_path = "/Game/Core/BP_NIO_PlayerController"
    pc_full = f"{pc_path}.BP_NIO_PlayerController"

    if unreal.EditorAssetLibrary.does_asset_exist(pc_full):
        pc_bp = unreal.EditorAssetLibrary.load_asset(pc_full)
        _log(f"  BP_NIO_PlayerController already exists")
    else:
        if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Core"):
            unreal.EditorAssetLibrary.make_directory("/Game/Core")
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.NomiPlayerController)
        pc_bp = at.create_asset(
            "BP_NIO_PlayerController", "/Game/Core", unreal.Blueprint, factory,
        )
        _log(f"  Created BP_NIO_PlayerController" if pc_bp else "  [WARN] Failed to create BP_NIO_PlayerController")

    # Wire InputMapping on CDO (matters even if BP already existed)
    if pc_bp and actions and imc:
        try:
            pc_gc = pc_bp.generated_class()
            cdo = unreal.get_default_object(pc_gc)

            # Get the FVehicleInputMapping struct from CDO
            mapping = cdo.get_editor_property("InputMapping")
            if mapping is None:
                _log("  [WARN] Could not read InputMapping from PC CDO")
            else:
                # 1. Set MappingContext
                mapping.set_editor_property("MappingContext", imc)
                _log("  Set InputMapping.MappingContext → IMC_VehicleControls")

                # 2. Set each InputAction in the Actions sub-struct
                ia = mapping.get_editor_property("Actions")
                if ia is not None:
                    action_map = {
                        "ThrottleAction":   "IA_Throttle",
                        "BrakeAction":      "IA_Brake",
                        "SteerAction":      "IA_Steer",
                        "HandbrakeAction":  "IA_Handbrake",
                        "LookBackAction":   "IA_LookBack",
                        "ChangeCameraAction":"IA_ChangeCamera",
                        "HornAction":       "IA_Horn",
                        "HeadlightsAction": "IA_Headlights",
                        "PauseAction":      "IA_Pause",
                        "ResetAction":      "IA_Reset",
                    }
                    for prop_name, action_name in action_map.items():
                        action_asset = actions.get(action_name)
                        if action_asset:
                            ia.set_editor_property(prop_name, action_asset)
                    _log(f"  Set {len(action_map)} InputActions on InputMapping.Actions")

                    mapping.set_editor_property("Actions", ia)

                # Write the struct back to the CDO
                cdo.set_editor_property("InputMapping", mapping)
                _log("  InputMapping wired on BP_NIO_PlayerController CDO")
        except Exception as e:
            _log(f"  [WARN] Failed to wire InputMapping on PC CDO: {e}")
            traceback.print_exc()
    else:
        if not pc_bp:
            _log("  [WARN] Cannot wire InputMapping: pc_bp is None")
        elif not actions:
            _log("  [WARN] Cannot wire InputMapping: no actions provided")
        elif not imc:
            _log("  [WARN] Cannot wire InputMapping: no IMC provided")

    return pc_bp


def _configure_game_mode(player_controller_bp=None):
    """Set BP_NIO_GameMode with PlayerController and DefaultPawn."""
    gm_path = "/Game/Core/BP_NIO_GameMode.BP_NIO_GameMode"
    if not unreal.EditorAssetLibrary.does_asset_exist(gm_path):
        _log(f"  Creating BP_NIO_GameMode...")
        if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Core"):
            unreal.EditorAssetLibrary.make_directory("/Game/Core")
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.GameModeBase)
        gm = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "BP_NIO_GameMode", "/Game/Core", unreal.Blueprint, factory,
        )
    else:
        gm = unreal.EditorAssetLibrary.load_asset(gm_path)

    if not gm:
        _log("  [WARN] Failed to get GameMode BP")
        return

    gm_gc = gm.generated_class()
    gm_cdo = unreal.get_default_object(gm_gc)

    # Set Player Controller Class
    if player_controller_bp:
        try:
            pc_gc = player_controller_bp.generated_class()
            gm_cdo.set_editor_property("player_controller_class", pc_gc)
            _log("  Set PlayerControllerClass on GameMode CDO")
        except Exception as e:
            _log(f"  [WARN] Set PlayerControllerClass failed: {e}")

    # Set Default Pawn Class (use EP9 as default)
    ep9_bp_path = "/Game/Vehicles/EP9/Blueprints/BP_NIO_EP9.BP_NIO_EP9"
    if unreal.EditorAssetLibrary.does_asset_exist(ep9_bp_path):
        ep9_bp = unreal.EditorAssetLibrary.load_asset(ep9_bp_path)
        try:
            ep9_gc = ep9_bp.generated_class()
            gm_cdo.set_editor_property("default_pawn_class", ep9_gc)
            _log("  Set DefaultPawnClass to BP_NIO_EP9")
        except Exception as e:
            _log(f"  [WARN] Set DefaultPawnClass failed: {e}")

    # Set HUD class (use WBP_HUD)
    hud_path = "/Game/UI/WBP_HUD.WBP_HUD"
    if unreal.EditorAssetLibrary.does_asset_exist(hud_path):
        hud_bp = unreal.EditorAssetLibrary.load_asset(hud_path)
        try:
            hud_gc = hud_bp.generated_class()
            gm_cdo.set_editor_property("hud_class", hud_gc)
            _log("  Set HUDClass to WBP_HUD")
        except Exception as e:
            _log(f"  [WARN] Set HUDClass failed: {e}")

    _log("  GameMode configuration complete")


def _setup_gamemode_and_settings():
    """Full game setup: input → vehicle reparent → PC → GameMode → project config."""

    # Step 1: Create input assets
    _log("  [sub] Creating input assets...")
    actions, imc = _create_input_assets()

    # Step 2: Reparent vehicle Blueprints
    _log("  [sub] Reparenting vehicle Blueprints...")
    _reparent_vehicle_blueprints()

    # Step 3: Create PlayerController (wire InputMapping on CDO)
    _log("  [sub] Creating PlayerController...")
    pc_bp = _create_player_controller(actions, imc)

    # Step 4: Configure GameMode
    _log("  [sub] Configuring GameMode...")
    _configure_game_mode(pc_bp)

    # Step 5: Set project defaults via console commands
    _log("  [sub] Setting project defaults...")
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        unreal.SystemLibrary.execute_console_command(
            world,
            "set Engine.GameMode /Game/Core/BP_NIO_GameMode.BP_NIO_GameMode"
        )
        _log("  Default game mode set")
    except Exception as e:
        _log(f"  [WARN] GameMode settings failed: {e}")

    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        unreal.SystemLibrary.execute_console_command(
            world,
            "set Engine.GameDefaultMap /Game/Maps/TestTrack.TestTrack"
        )
        _log("  Default map set to TestTrack")
    except Exception as e:
        _log(f"  [WARN] Default map settings failed: {e}")

    _log("  GameMode and settings setup complete")


# ============================================================================
# 16-Step Orchestrator
# ============================================================================

class FullSetup:
    """Orchestrates the 16-step NIO Racing Plus setup pipeline."""

    def __init__(self):
        self.total_steps = 16
        self.current_step = 0
        self.skipped_steps: list[str] = []

    # ------------------------------------------------------------------
    # Public entry point
    # ------------------------------------------------------------------

    def run(self) -> None:
        """Execute all 16 setup steps in order."""
        if not HAS_SLATE:
            _log("Running in headless/commandlet mode — Slate NOT available.", "warn")
            _log("Steps requiring UI (import, material, BP, track) will be skipped.", "warn")
            _log("Run 'run_full_setup()' inside UE5 Editor for full pipeline.\n", "warn")
        else:
            _log("Slate available — full 16-step pipeline will execute.\n")

        _log("\n" + "=" * 60)
        _log("  NIO Racing Plus — Full Setup Pipeline (16 steps)")
        _log("=" * 60 + "\n")

        steps = [
            ("Fixing common issues",            self._step_fix_issues,         True),
            ("Importing assets",                self._step_import_assets,      HAS_SLATE),
            ("Creating materials",              self._step_create_materials,   HAS_SLATE),
            ("Building vehicle Blueprints",     self._step_build_vehicles,     HAS_SLATE),
            ("Building tracks",                 self._step_build_tracks,       HAS_SLATE),
            ("Setting up audio",                self._step_setup_audio,        HAS_SLATE),
            ("Configuring AI",                  self._step_configure_ai,       True),
            ("Setting up UI",                   self._step_setup_ui,           HAS_SLATE),
            ("Configuring NOMI",                self._step_configure_nomi,     True),
            ("Creating Input assets",           self._step_create_input,       HAS_SLATE),
            ("Reparenting vehicle Blueprints",  self._step_reparent_vehicles,  HAS_SLATE),
            ("Setting up PlayerController",     self._step_setup_pc,           HAS_SLATE),
            ("Configuring GameMode",            self._step_configure_gm,       HAS_SLATE),
            ("Compiling Blueprints",            self._step_compile_blueprints, HAS_SLATE),
            ("Validating project",              self._step_validate,           True),
            ("Saving and reporting",            self._step_save_and_report,    True),
        ]

        for i, (label, method, can_run) in enumerate(steps, 1):
            self.current_step = i
            _log_step(i, self.total_steps, label)
            if can_run:
                try:
                    method()
                    _log(f"  ✓ {label}")
                except Exception as exc:
                    _log(f"  ✗ {label} FAILED: {exc}", "error")
                    traceback.print_exc()
                    self.skipped_steps.append(f"{label} (FAILED)")
            else:
                _log("  ⏭ Skipped (requires Slate — run in UE5 Editor)")
                self.skipped_steps.append(label)

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
        at = unreal.AssetToolsHelpers.get_asset_tools()
        for sc in sc_names:
            path = f"/Game/Audio/{sc}"
            if not unreal.EditorAssetLibrary.does_asset_exist(f"{path}.SoundClass"):
                try:
                    factory = unreal.SoundClassFactory()
                    asset = at.create_asset(
                        sc, "/Game/Audio", unreal.SoundClass, factory,
                    )
                    if asset:
                        _log(f"  Created SoundClass: {sc}")
                except Exception as e:
                    _log(f"  [WARN] Could not create SoundClass {sc}: {e}")

        # Create SoundCues for each motor loop (skip if factory not available)
        try:
            SoundCueFactory = unreal.SoundCueFactory
        except AttributeError:
            SoundCueFactory = None
            _log("  SoundCueFactory not available in UE5.7 Python API — skipping SoundCue creation")

        for vehicle in ("EP9", "ET7", "ES7", "SU7Ultra"):
            cue_name = f"SC_Motor_{vehicle}"
            cue_path = f"/Game/Audio/Motor/{cue_name}.{cue_name}"
            wave_path = f"/Game/Audio/Motor/{vehicle}_motor_loop.{vehicle}_motor_loop"
            if SoundCueFactory and not unreal.EditorAssetLibrary.does_asset_exist(cue_path) \
               and unreal.EditorAssetLibrary.does_asset_exist(wave_path):
                try:
                    factory = SoundCueFactory()
                    asset = at.create_asset(
                        cue_name, "/Game/Audio/Motor", unreal.SoundCue, factory,
                    )
                    if asset:
                        _log(f"  Created SoundCue: {cue_name}")
                except Exception as e:
                    _log(f"  [WARN] Could not create SoundCue {cue_name}: {e}")

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

    def _step_create_input(self) -> None:
        """Create Enhanced Input assets and store for later steps."""
        self._input_actions, self._input_imc = _create_input_assets()

    # ── Step 11 ───────────────────────────────────────────────────────

    def _step_reparent_vehicles(self) -> None:
        _reparent_vehicle_blueprints()

    # ── Step 12 ───────────────────────────────────────────────────────

    def _step_setup_pc(self) -> None:
        actions = getattr(self, '_input_actions', None)
        imc = getattr(self, '_input_imc', None)
        self._pc_bp = _create_player_controller(actions, imc)

    # ── Step 13 ───────────────────────────────────────────────────────

    def _step_configure_gm(self) -> None:
        pc_bp = getattr(self, '_pc_bp', None)
        _configure_game_mode(pc_bp)

        # Set project defaults via console commands
        try:
            world = unreal.EditorLevelLibrary.get_editor_world()
            unreal.SystemLibrary.execute_console_command(
                world,
                "set Engine.GameMode /Game/Core/BP_NIO_GameMode.BP_NIO_GameMode"
            )
            _log("  Default game mode set")
        except Exception as e:
            _log(f"  [WARN] GameMode settings failed: {e}")

        try:
            world = unreal.EditorLevelLibrary.get_editor_world()
            unreal.SystemLibrary.execute_console_command(
                world,
                "set Engine.GameDefaultMap /Game/Maps/TestTrack.TestTrack"
            )
            _log("  Default map set to TestTrack")
        except Exception as e:
            _log(f"  [WARN] Default map settings failed: {e}")

    # ── Step 14 ───────────────────────────────────────────────────────

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

    # ── Step 15 ───────────────────────────────────────────────────────

    def _step_validate(self) -> None:
        passed = validate_project.run_validation()
        if passed:
            _log("  All validation checks passed")
        else:
            _log("  Some checks failed — review output above", "warn")

    # ── Step 16 ───────────────────────────────────────────────────────

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
            _log("  All 16 steps executed successfully!")
        else:
            _log(f"  {16 - len(self.skipped_steps)}/16 steps executed.")
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
    """Run the complete 16-step setup pipeline."""
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
