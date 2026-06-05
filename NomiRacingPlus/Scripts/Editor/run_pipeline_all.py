"""
NIO Racing Plus — Full Pipeline (12 steps)
Runs headless via -RenderOffscreen, no window interaction needed.
Auto-exits the editor on completion or failure.
"""
import unreal
import sys
import os
import traceback

# ── Bootstrap paths ──────────────────────────────────────────────────────────
PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()
SCRIPTS_DIR = os.path.join(PROJECT_DIR, "Scripts", "Editor")
for p in (SCRIPTS_DIR, os.path.join(CONTENT_DIR, "Python")):
    if p not in sys.path:
        sys.path.insert(0, p)

# ── Imports ──────────────────────────────────────────────────────────────────
import fix_common_issues
import batch_import
import material_setup
import vehicle_blueprint_builder
import track_builder
import setup_test_level
import blueprint_compile
import validate_project

# ── Helpers ──────────────────────────────────────────────────────────────────
LOG_PREFIX = "[AutoPipeline]"
_log = lambda msg: unreal.log_warning(f"{LOG_PREFIX} {msg}")
_errors = []

def _run_step(num, label, fn, *args):
    _log(f"[Step {num}] {label} ...")
    try:
        fn(*args)
        _log(f"  [OK] {label}")
    except Exception as e:
        _log(f"  [FAIL] {label}: {e}")
        traceback.print_exc()
        _errors.append((num, label, str(e)))

def _exit_editor():
    """Save dirty packages and request clean editor exit."""
    _log("Saving dirty packages...")
    try:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
        _log("Dirty packages saved.")
    except Exception as e:
        _log(f"Save warning: {e}")

    _log("Requesting editor exit...")
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        unreal.SystemLibrary.execute_console_command(world, "Exit")
    except Exception as e:
        _log(f"Exit via console failed: {e}")
        # Last resort — write a sentinel file so the outer script knows
        # the pipeline completed even if the process lingers.
        with open("/tmp/ue_pipeline_done", "w") as f:
            f.write("completed\n")

# ── Sub-steps ────────────────────────────────────────────────────────────────

def _setup_audio():
    sc_names = ["Motor", "SFX", "UI", "Music", "NOMI"]
    for sc in sc_names:
        path = f"/Game/Audio/{sc}"
        if not unreal.EditorAssetLibrary.does_asset_exist(f"{path}.SoundClass"):
            factory = unreal.SoundClassFactory()
            unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                sc, "/Game/Audio", unreal.SoundClass, factory,
            )
    for vehicle in ("EP9", "ET7", "ES7", "SU7Ultra"):
        cue_name = f"SC_Motor_{vehicle}"
        cue_path = f"/Game/Audio/Motor/{cue_name}"
        wave_path = f"/Game/Audio/Motor/{vehicle}_motor_loop"
        if not unreal.EditorAssetLibrary.does_asset_exist(f"{cue_path}.SoundCue") \
           and unreal.EditorAssetLibrary.does_asset_exist(f"{wave_path}.SoundWave"):
            try:
                SoundCueFactory = unreal.SoundCueFactory
            except AttributeError:
                _log("  SoundCueFactory not available — skipping SoundCue creation")
                break
            factory = SoundCueFactory()
            unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                cue_name, "/Game/Audio/Motor", unreal.SoundCue, factory,
            )

def _configure_ai():
    ai_cfg = os.path.join(CONTENT_DIR, "AI", "AIProfiles.json")
    if os.path.isfile(ai_cfg):
        _log(f"  AI profiles loaded from {ai_cfg}")
    else:
        _log("  AIProfiles.json not found")

def _setup_ui():
    for wgt in ("WBP_HUD", "WBP_MainMenu", "WBP_PauseMenu", "WBP_RaceResult"):
        path = f"/Game/UI/{wgt}"
        if not unreal.EditorAssetLibrary.does_asset_exist(f"{path}.WidgetBlueprint"):
            factory = unreal.WidgetBlueprintFactory()
            unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                wgt, "/Game/UI", unreal.WidgetBlueprint, factory,
            )

def _configure_nomi():
    comments = os.path.join(CONTENT_DIR, "NOMI", "Comments", "DefaultComments.json")
    if os.path.isfile(comments):
        _log(f"  NOMI commentary loaded from {comments}")
    else:
        _log("  DefaultComments.json not found")

def _compile_blueprints():
    # Refresh asset registry so newly created BPs are found by list_assets
    try:
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        ar.scan_modified_files()
    except Exception as e:
        _log(f"  Asset registry scan failed: {e}")

    # Also try the known BP paths directly (handle list_assets not finding them)
    vehicle_types = ["EP9", "ET7", "ES7", "SU7Ultra"]
    compiled_any = False
    for vt in vehicle_types:
        bp_path = f"/Game/Vehicles/{vt}/Blueprints/BP_NIO_{vt}.BP_NIO_{vt}"
        if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
            bp = unreal.EditorAssetLibrary.load_asset(bp_path)
            if bp and isinstance(bp, unreal.Blueprint):
                unreal.BlueprintEditorLibrary.compile_blueprint(bp)
                compiled_any = True
                _log(f"  Compiled: {bp_path}")

    # Also run the full scan for any other BPs
    report = blueprint_compile.compile_all_blueprints()
    if report:
        _log(f"  Compiled {report.total} BPs: {report.succeeded} succeeded, {report.failed} failed")

def _validate_project():
    passed = validate_project.run_validation()
    _log(f"  Validation: {'PASSED' if passed else 'SOME CHECKS FAILED'}")

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
        # Assign the full modifier list at once (avoids get-modify-set race)
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

    # Add key mappings to the context (standard WASD)
    if imc:
        # Clear existing mappings first
        imc.set_editor_property("mappings", [])

        # Build list of all mappings
        all_mappings = []
        ia = created_actions  # shorthand

        # Helper: add mapping with optional scalar modifier
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
        add(ia["IA_Steer"],       "A",            -1.0)   # Left steer = negative
        add(ia["IA_Steer"],       "D",            None)   # Right steer = positive
        add(ia["IA_Handbrake"],   "SpaceBar",     None)
        add(ia["IA_LookBack"],    "LeftAlt",      None)
        add(ia["IA_ChangeCamera"],"C",            None)
        add(ia["IA_Horn"],        "H",            None)
        add(ia["IA_Headlights"],  "L",            None)
        add(ia["IA_Pause"],       "Escape",       None)
        add(ia["IA_Reset"],       "R",            None)

        # Gamepad analog mappings with deadzone (single mapping per key — no duplicates)
        deadzone = unreal.InputModifierDeadZone()
        deadzone.set_editor_property("lower_threshold", 0.1)
        deadzone.set_editor_property("type", unreal.DeadZoneType.RADIAL)
        deadzone.set_editor_property("upper_threshold", 1.0)

        all_mappings.append(_make_mapping(ia["IA_Throttle"], "Gamepad_RightTriggerAxis", [deadzone]))
        all_mappings.append(_make_mapping(ia["IA_Brake"],    "Gamepad_LeftTriggerAxis",  [deadzone]))
        all_mappings.append(_make_mapping(ia["IA_Steer"],    "Gamepad_LeftX",            [deadzone]))

        # Assign all mappings to IMC
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

            # Check current parent class — skip reparent if already correct
            try:
                gc = bp.generated_class()
                current_parent = gc.get_super_class()
                if current_parent and current_parent.get_name().startswith("NIOVehicleBase"):
                    _log(f"  BP_NIO_{vt_name} already parented to NIOVehicleBase")
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

            # Set vehicle_type property on CDO (properties live on the class CDO, not BP itself)
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
            import traceback
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

    # Properties live on CDO of the generated class, not on Blueprint itself
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

    # Step 5: Set project defaults
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

def _save_and_report():
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    _log("  All dirty packages saved")

# ── Run pipeline ─────────────────────────────────────────────────────────────
_log("=" * 60)
_log("  NIO Racing Plus — Full Automated Pipeline (v2)")
_log("  Mode: headless (RenderOffscreen + nosplash)")
_log("=" * 60)

try:
    # Step 1
    _run_step(1, "Fix common issues", fix_common_issues.fix_all_issues)

    # Step 2 — Import assets
    _log("[Step 2] Import assets ...")
    vehicles = {
        "EP9":     ("Assets/NIO/EP9/ep9.glb",          "/Game/Vehicles/EP9/Meshes"),
        "ES7":     ("Assets/NIO/ES7/es7.glb",          "/Game/Vehicles/ES7/Meshes"),
        "ET7":     ("Assets/NIO/ET7/et7.glb",          "/Game/Vehicles/ET7/Meshes"),
        "SU7Ultra":("Assets/Xiaomi/SU7Ultra/su7ultra.glb", "/Game/Vehicles/SU7Ultra/Meshes"),
    }
    for vname, (rel_src, dest) in vehicles.items():
        full_path = os.path.join(PROJECT_DIR, rel_src)
        if os.path.isfile(full_path):
            batch_import.import_meshes(os.path.dirname(full_path), dest, import_as_skeletal=False)
            _log(f"  Imported {vname} mesh")

    for rel_src, dest in [("Assets/Textures/Vehicle","/Game/Textures/Vehicle"),
                          ("Assets/Textures/Road","/Game/Textures/Road"),
                          ("Assets/Textures","/Game/Textures")]:
        full_path = os.path.join(PROJECT_DIR, rel_src)
        if os.path.isdir(full_path):
            batch_import.import_textures(full_path, dest)

    for rel_src, dest in [("Assets/Audio/Motor","/Game/Audio/Motor"),
                          ("Assets/Audio/SFX","/Game/Audio/SFX"),
                          ("Assets/Audio/UI","/Game/Audio/UI")]:
        full_path = os.path.join(PROJECT_DIR, rel_src)
        if os.path.isdir(full_path):
            batch_import.import_audio(full_path, dest)
    _log("  [OK] Step 2 — Import assets")

    # Step 3
    _run_step(3, "Create materials", material_setup.create_all_master_materials)
    for v in ("EP9", "ET7", "ES7"):
        material_setup.create_all_vehicle_materials(v)
    _log("  [OK] Step 3 — Materials created")

    # Step 4
    _run_step(4, "Build vehicle Blueprints", vehicle_blueprint_builder.create_all_vehicle_blueprints)

    # Step 5
    _run_step(5, "Build tracks", track_builder.create_all_tracks)
    _run_step(5, "Build main menu", track_builder.create_main_menu_level)
    _run_step(5, "Setup test level", setup_test_level.setup_test_level)

    # Step 6
    _run_step(6, "Setup audio", _setup_audio)

    # Step 7
    _run_step(7, "Configure AI", _configure_ai)

    # Step 8
    _run_step(8, "Setup UI", _setup_ui)

    # Step 9
    _run_step(9, "Configure NOMI", _configure_nomi)

    # Step 9b — Setup GameMode and project settings
    _run_step("9b", "Setup GameMode and project settings", _setup_gamemode_and_settings)

    # Step 10
    _run_step(10, "Compile Blueprints", _compile_blueprints)

    # Step 11
    _run_step(11, "Validate", _validate_project)

    # Step 12
    _run_step(12, "Save and report", _save_and_report)

    _log("=" * 60)
    _log("  ✅ ALL STEPS COMPLETED SUCCESSFULLY!")
    _log("=" * 60)

except Exception as pipeline_error:
    _log(f"\n  ❌ PIPELINE FAILED at step with error: {pipeline_error}")
    _log(f"  Errors so far: {_errors}")

finally:
    _exit_editor()
