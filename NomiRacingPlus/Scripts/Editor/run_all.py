"""
NIO Racing Plus - Unified Editor Runner
========================================
Convenience script to run common editor workflows from a single entry point.

Usage (UE5 Editor Output Log):
    exec(open(r"<PROJECT>/Scripts/Editor/run_all.py").read())

    # Then call any workflow:
    setup_new_vehicle("EP9", "/path/to/processed/EP9")
    setup_full_project()
    quick_validate()
"""

import unreal
import sys
import os

_log = lambda msg: unreal.log(f"[Runner] {msg}")
_log_warn = lambda msg: unreal.log_warning(f"[Runner] {msg}")
_log_err = lambda msg: unreal.log_error(f"[Runner] {msg}")

# Determine the scripts directory.
# When loaded via exec(open(...)), __file__ is not defined, so we
# fall back to the known project-relative path.
_scripts_dir = ""
if "__file__" in dir():
    _scripts_dir = os.path.dirname(os.path.abspath(__file__))

# Candidate search paths for the editor scripts
_SEARCH_PATHS = [
    _scripts_dir,
    # Project-relative paths (common locations)
    os.path.join(os.getcwd(), "NomiRacingPlus", "Scripts", "Editor"),
    os.path.join(os.getcwd(), "Scripts", "Editor"),
]

def _find_script(name: str) -> str:
    """Locate a sibling script by name."""
    for base in _SEARCH_PATHS:
        if not base:
            continue
        candidate = os.path.join(base, name)
        if os.path.isfile(candidate):
            return candidate
    return ""


def _load_module(name: str):
    """Load a sibling script by executing it into the caller's globals."""
    path = _find_script(name)
    if not path:
        _log_warn(f"Module not found: {name}")
        return False
    try:
        exec(open(path, encoding="utf-8").read(), globals())
        _log(f"Loaded module: {name}")
        return True
    except Exception as e:
        _log_err(f"Failed to load {name}: {e}")
        return False

# Load all sub-modules into this script's namespace
_load_module("batch_import.py")
_load_module("lod_generation.py")
_load_module("material_setup.py")
_load_module("blueprint_compile.py")
_load_module("asset_validator.py")


# ---------------------------------------------------------------------------
# Workflows
# ---------------------------------------------------------------------------

def setup_new_vehicle(vehicle_name: str, source_dir: str,
                       import_as_skeletal: bool = False):
    """Full pipeline to set up a new vehicle.

    Steps:
        1. Import meshes (LOD0, LOD1, LOD2)
        2. Import textures
        3. Generate LODs (if single mesh imported)
        4. Create materials and paint presets
        5. Compile vehicle Blueprints

    Args:
        vehicle_name: "EP9", "ET7", "ES7", or "ET5".
        source_dir: Local path to processed vehicle files.
        import_as_skeletal: Import as skeletal mesh.
    """
    _log(f"=== Setting up vehicle: {vehicle_name} ===")
    dest = f"/Game/Vehicles/{vehicle_name}"

    # Step 1: Import
    _log("Step 1/5: Importing assets...")
    batch_import.import_nio_vehicle(vehicle_name, source_dir)

    # Step 2: Generate LODs
    _log("Step 2/5: Generating LODs...")
    mesh_dir = f"{dest}/Meshes"
    if unreal.EditorAssetLibrary.does_directory_exist(mesh_dir):
        lod_generation.generate_lods_for_directory(
            mesh_dir, profile="moderate", vehicle_name=vehicle_name
        )

    # Step 3: Create materials
    _log("Step 3/5: Creating materials...")
    material_setup.create_all_vehicle_materials(vehicle_name)

    # Step 4: Auto-assign textures
    _log("Step 4/5: Assigning textures...")
    material_setup.auto_assign_vehicle_textures(vehicle_name)

    # Step 5: Compile Blueprints
    _log("Step 5/5: Compiling Blueprints...")
    bp_dir = f"{dest}/Blueprints"
    if unreal.EditorAssetLibrary.does_directory_exist(bp_dir):
        blueprint_compile.compile_blueprints_in_directory(bp_dir)

    _log(f"=== Vehicle {vehicle_name} setup complete ===")


def setup_full_project():
    """Run the full project setup pipeline.

    Steps:
        1. Create all master materials
        2. Configure LOD group settings
        3. Compile all Blueprints
        4. Validate (no missing dependencies)
    """
    _log("=== Full Project Setup ===")

    _log("Step 1/4: Creating master materials...")
    material_setup.create_all_master_materials()

    _log("Step 2/4: Configuring LOD settings...")
    lod_generation.configure_lod_group_settings()

    _log("Step 3/4: Compiling all Blueprints...")
    blueprint_compile.compile_all_blueprints()

    _log("Step 4/4: Validating dependencies...")
    missing = blueprint_compile.find_missing_dependencies()

    if missing:
        _log(f"WARNING: {len(missing)} missing dependencies found!")
    else:
        _log("All dependencies satisfied")

    _log("=== Full Project Setup Complete ===")


def quick_validate():
    """Quick validation: compile dirty BPs + check dependencies + asset validation.

    Use before committing to catch issues early.
    """
    _log("=== Quick Validation ===")

    # Blueprint checks
    bp_ok = blueprint_compile.pre_commit_check()

    # Asset validation
    report = asset_validator.validate_all_assets()
    asset_ok = report.passed

    if bp_ok and asset_ok:
        _log("Validation PASSED - ready to commit")
    else:
        if not bp_ok:
            _log("Blueprint validation FAILED")
        if not asset_ok:
            _log(f"Asset validation FAILED: {len(report.errors)} error(s)")
    return bp_ok and asset_ok


def validate_assets(vehicle: str = ""):
    """Run asset validation.

    Args:
        vehicle: If specified, validate only this vehicle (EP9, ET7, ES7, ET5).
                 If empty, validate all project assets.

    Returns:
        ValidationReport with all findings.
    """
    if vehicle:
        report = asset_validator.validate_vehicle(vehicle)
    else:
        report = asset_validator.validate_all_assets()
    return report


def import_and_setup_carla(town: str = "Town03", carla_content_dir: str = ""):
    """Import CARLA assets and set up environment materials.

    Args:
        town: CARLA town name.
        carla_content_dir: Path to CARLA's Content/Carla directory.
    """
    _log(f"=== CARLA {town} Setup ===")

    if carla_content_dir:
        batch_import.import_carla_assets(carla_content_dir, town)

    material_setup.create_road_material()
    material_setup.create_concrete_material()
    material_setup.create_metal_material()

    _log(f"=== CARLA {town} Setup Complete ===")


def generate_lods_all_vehicles(profile: str = "moderate"):
    """Generate LODs for all NIO vehicles."""
    _log("=== Generating LODs for All Vehicles ===")
    lod_generation.generate_all_vehicle_lods(profile)
    _log("=== LOD Generation Complete ===")


def create_materials_all_vehicles():
    """Create materials for all NIO vehicles."""
    _log("=== Creating Materials for All Vehicles ===")
    material_setup.create_all_vehicles_materials()
    _log("=== Materials Complete ===")


def compile_everything(force: bool = False):
    """Compile all Blueprints in the project."""
    _log("=== Compiling All Blueprints ===")
    report = blueprint_compile.compile_all_blueprints(force=force)
    return report


# ---------------------------------------------------------------------------
# Informational
# ---------------------------------------------------------------------------

def print_help():
    """Print available workflows."""
    _log("")
    _log("=" * 60)
    _log("  NIO Racing Plus - Editor Utility Commands")
    _log("=" * 60)
    _log("")
    _log("  VEHICLE PIPELINE:")
    _log("    setup_new_vehicle(name, source_dir)")
    _log("      Full import + LOD + material + compile for one vehicle")
    _log("      Example: setup_new_vehicle('EP9', '/path/to/EP9')")
    _log("")
    _log("    generate_lods_all_vehicles(profile='moderate')")
    _log("      Generate LODs for all NIO vehicles")
    _log("")
    _log("    create_materials_all_vehicles()")
    _log("      Create materials for all NIO vehicles")
    _log("")
    _log("  PROJECT:")
    _log("    setup_full_project()")
    _log("      Run full project setup pipeline")
    _log("")
    _log("    compile_everything(force=False)")
    _log("      Compile all Blueprints")
    _log("")
    _log("    quick_validate()")
    _log("      Pre-commit validation (BPs + dependencies + assets)")
    _log("")
    _log("    validate_assets(vehicle='')")
    _log("      Validate assets: polygon counts, textures, materials, naming")
    _log("      Example: validate_assets('EP9')  # single vehicle")
    _log("")
    _log("  CARLA:")
    _log("    import_and_setup_carla(town, carla_content_dir)")
    _log("      Import CARLA assets and create environment materials")
    _log("")
    _log("  INDIVIDUAL MODULES:")
    _log("    batch_import.import_assets(source, dest)")
    _log("    lod_generation.generate_lods_for_mesh(path)")
    _log("    material_setup.create_vehicle_paint_material()")
    _log("    blueprint_compile.compile_all_blueprints()")
    _log("    asset_validator.validate_all_assets()")
    _log("    asset_validator.validate_vehicle('EP9')")
    _log("")
    _log("=" * 60)


# Auto-print help on load
print_help()
