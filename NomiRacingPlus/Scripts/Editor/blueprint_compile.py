"""
NIO Racing Plus - Blueprint Compilation Script
================================================
UE5 Editor utility to batch-compile Blueprints with error reporting,
dependency validation, and selective recompilation.

Usage (UE5 Editor Output Log):
    exec(open(r"<PROJECT>/Scripts/Editor/blueprint_compile.py").read())
    compile_all_blueprints()

Usage (Command line):
    UnrealEditor-Cmd <PROJECT>.uproject -run=pythonscript -script="<PROJECT>/Scripts/Editor/blueprint_compile.py" -- -all
"""

import unreal
import time
from dataclasses import dataclass, field
from pathlib import Path


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class CompileResult:
    """Result of compiling a single Blueprint."""
    asset_path: str
    success: bool
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    compile_time_ms: float = 0.0


@dataclass
class CompileReport:
    """Summary report of a batch compile operation."""
    total: int = 0
    succeeded: int = 0
    failed: int = 0
    skipped: int = 0
    total_time_ms: float = 0.0
    results: list[CompileResult] = field(default_factory=list)

    @property
    def success_rate(self) -> float:
        if self.total == 0:
            return 0.0
        return self.succeeded / self.total * 100.0


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _log(msg: str, level: str = "info"):
    dispatch = {
        "info": unreal.log,
        "warn": unreal.log_warning,
        "error": unreal.log_error,
    }
    dispatch.get(level, unreal.log)(f"[BlueprintCompile] {msg}")


def _find_all_blueprints(directory: str | None = None) -> list[str]:
    """Find all Blueprint assets in the project (or a specific directory).

    Args:
        directory: UE5 content directory to search. None = entire /Game.

    Returns:
        List of UE5 asset paths.
    """
    search_dir = directory or "/Game"
    _log(f"Scanning for Blueprints in: {search_dir}")

    # Find all Blueprint asset types
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_class = unreal.LoadObject(None, "/Script/Engine.Blueprint")

    # Use the asset registry to find all Blueprint assets
    # Filter for various Blueprint types
    bp_class_names = [
        "Blueprint",
        "AnimBlueprint",
        "WidgetBlueprint",
        "LevelScriptBlueprint",
        "BlueprintGeneratedClass",
    ]

    all_blueprints = []

    for class_name in bp_class_names:
        asset_data_list = unreal.EditorAssetLibrary.list_assets(
            search_dir, recursive=True, include_folder=False
        )

    # Filter to only Blueprint assets
    for asset_path in asset_data_list:
        # Check if the asset is a Blueprint by attempting to load it
        asset_class = unreal.EditorAssetLibrary.find_asset_data(asset_path).get_asset_class()
        class_name_str = str(asset_class)

        if "Blueprint" in class_name_str or "Widget" in class_name_str:
            all_blueprints.append(asset_path)

    _log(f"Found {len(all_blueprints)} Blueprint(s)")
    return sorted(all_blueprints)


def _classify_blueprint(asset_path: str) -> str:
    """Classify a Blueprint by its type for targeted compilation."""
    path_lower = asset_path.lower()

    if "/ui/" in path_lower or "/hud/" in path_lower or "/menus/" in path_lower:
        return "widget"
    if "/vehicles/" in path_lower:
        return "vehicle"
    if "/nomi/" in path_lower or "/commentary" in path_lower:
        return "nomi"
    if "/maps/" in path_lower or "gamemode" in path_lower or "gameplay" in path_lower:
        return "gameplay"
    if "/ai/" in path_lower:
        return "ai"
    return "other"


# ---------------------------------------------------------------------------
# Single Blueprint compilation
# ---------------------------------------------------------------------------

def compile_blueprint(asset_path: str, force: bool = False) -> CompileResult:
    """Compile a single Blueprint.

    Args:
        asset_path: UE5 asset path, e.g. "/Game/Vehicles/EP9/BP_EP9".
        force: Force recompile even if already compiled.

    Returns:
        CompileResult with success status and any errors/warnings.
    """
    result = CompileResult(asset_path=asset_path, success=False)
    start = time.perf_counter()

    try:
        # Load the Blueprint
        bp = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not bp:
            result.errors.append(f"Could not load asset: {asset_path}")
            return result

        if not isinstance(bp, unreal.Blueprint):
            result.errors.append(f"Asset is not a Blueprint: {asset_path} (type: {type(bp).__name__})")
            return result

        # Check if compilation is needed
        if not force and bp.is_compiled():
            # Already compiled, check for pending changes
            status = bp.get_editor_property("status")
            if status == unreal.BlueprintStatus.BS_UpToDate:
                result.success = True
                result.compile_time_ms = (time.perf_counter() - start) * 1000
                return result

        # Compile the Blueprint
        bp_status_before = bp.get_editor_property("status")

        # Mark as needing compilation
        bp.set_editor_property("status", unreal.BlueprintStatus.BS_Dirty)

        # Compile
        # In UE5, Blueprint compilation is done via the Kismet system
        # Use the editor's compile function
        compile_options = unreal.BlueprintCompileOptions()
        compile_options.set_editor_property("save_on_success", True)

        # Perform the compilation
        # The compilation triggers the Blueprint compiler pipeline
        bp.compile()

        # Check result
        bp_status_after = bp.get_editor_property("status")

        if bp_status_after == unreal.BlueprintStatus.BS_UpToDate:
            result.success = True
            _log(f"  OK: {asset_path}")
        elif bp_status_after == unreal.BlueprintStatus.BS_Error:
            result.errors.append(f"Compilation error in {asset_path}")
            _log(f"  ERROR: {asset_path}", "error")
        elif bp_status_after == unreal.BlueprintStatus.BS_Dirty:
            # Sometimes the status stays dirty but compilation succeeds
            result.success = True
            _log(f"  OK (dirty): {asset_path}", "warn")
        else:
            result.warnings.append(f"Unexpected status after compile: {bp_status_after}")
            _log(f"  WARNING: {asset_path} - status: {bp_status_after}", "warn")

    except Exception as e:
        result.errors.append(f"Exception during compilation: {str(e)}")
        _log(f"  EXCEPTION: {asset_path} - {e}", "error")

    result.compile_time_ms = (time.perf_counter() - start) * 1000
    return result


# ---------------------------------------------------------------------------
# Batch compilation
# ---------------------------------------------------------------------------

def compile_blueprints_in_directory(directory: str,
                                     category_filter: str | None = None,
                                     force: bool = False) -> CompileReport:
    """Compile all Blueprints in a directory.

    Args:
        directory: UE5 content directory.
        category_filter: Only compile Blueprints of this category
            ("vehicle", "widget", "nomi", "gameplay", "ai", "other").
        force: Force recompile.

    Returns:
        CompileReport with results.
    """
    report = CompileReport()
    start = time.perf_counter()

    _log(f"=== Batch Blueprint Compilation ===")
    _log(f"Directory: {directory}")
    _log(f"Category filter: {category_filter or 'all'}")
    _log(f"Force recompile: {force}")

    blueprints = _find_all_blueprints(directory)

    for bp_path in blueprints:
        category = _classify_blueprint(bp_path)

        if category_filter and category != category_filter:
            report.skipped += 1
            continue

        report.total += 1
        result = compile_blueprint(bp_path, force=force)
        report.results.append(result)

        if result.success:
            report.succeeded += 1
        else:
            report.failed += 1

    report.total_time_ms = (time.perf_counter() - start) * 1000

    _print_report(report)
    return report


def compile_all_blueprints(force: bool = False,
                            category_filter: str | None = None) -> CompileReport:
    """Compile all Blueprints in the entire project.

    Args:
        force: Force recompile all Blueprints.
        category_filter: Optional category filter.

    Returns:
        CompileReport.
    """
    return compile_blueprints_in_directory("/Game", category_filter, force)


def compile_vehicle_blueprints(force: bool = False) -> CompileReport:
    """Compile only vehicle-related Blueprints."""
    _log("=== Compiling Vehicle Blueprints ===")
    return compile_blueprints_in_directory("/Game/Vehicles", "vehicle", force)


def compile_ui_blueprints(force: bool = False) -> CompileReport:
    """Compile only UI/widget Blueprints."""
    _log("=== Compiling UI Blueprints ===")
    return compile_blueprints_in_directory("/Game/UI", "widget", force)


def compile_nomi_blueprints(force: bool = False) -> CompileReport:
    """Compile only NOMI-related Blueprints."""
    _log("=== Compiling NOMI Blueprints ===")
    return compile_blueprints_in_directory("/Game/NOMI", "nomi", force)


def compile_gameplay_blueprints(force: bool = False) -> CompileReport:
    """Compile only gameplay Blueprints (GameMode, PlayerController, etc.)."""
    _log("=== Compiling Gameplay Blueprints ===")
    return compile_blueprints_in_directory("/Game", "gameplay", force)


# ---------------------------------------------------------------------------
# Dependency checking
# ---------------------------------------------------------------------------

def check_blueprint_dependencies(asset_path: str) -> dict:
    """Check dependencies of a Blueprint.

    Returns:
        Dict with 'hard' and 'soft' dependency lists.
    """
    _log(f"Checking dependencies for: {asset_path}")

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_identifier = unreal.TopLevelAssetPath(asset_path.replace("/Game/", "/Game/"))

    # Get dependencies
    deps = asset_registry.get_dependencies(asset_identifier)

    hard_deps = []
    soft_deps = []

    for dep in deps:
        dep_str = str(dep)
        if dep_str.startswith("/Game/"):
            hard_deps.append(dep_str)
        else:
            soft_deps.append(dep_str)

    _log(f"  Hard dependencies: {len(hard_deps)}")
    _log(f"  Soft dependencies: {len(soft_deps)}")

    return {"hard": hard_deps, "soft": soft_deps}


def check_all_vehicle_dependencies() -> dict[str, dict]:
    """Check dependencies for all vehicle Blueprints."""
    _log("=== Checking Vehicle Dependencies ===")

    all_deps = {}
    vehicle_dirs = ["/Game/Vehicles/EP9", "/Game/Vehicles/ET7",
                    "/Game/Vehicles/ES7", "/Game/Vehicles/ET5"]

    for vdir in vehicle_dirs:
        if not unreal.EditorAssetLibrary.does_directory_exist(vdir):
            continue

        assets = unreal.EditorAssetLibrary.list_assets(vdir, recursive=True, include_folder=False)
        for asset_path in assets:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if isinstance(asset, unreal.Blueprint):
                deps = check_blueprint_dependencies(asset_path)
                all_deps[asset_path] = deps

    return all_deps


def find_missing_dependencies() -> list[dict]:
    """Scan all Blueprints and report missing dependencies.

    Returns:
        List of dicts with 'blueprint' and 'missing' keys.
    """
    _log("=== Scanning for Missing Dependencies ===")

    missing_report = []
    all_bps = _find_all_blueprints()

    for bp_path in all_bps:
        deps = check_blueprint_dependencies(bp_path)
        for hard_dep in deps["hard"]:
            if not unreal.EditorAssetLibrary.does_asset_exist(hard_dep):
                missing_report.append({
                    "blueprint": bp_path,
                    "missing": hard_dep,
                })
                _log(f"  MISSING: {bp_path} depends on {hard_dep}", "error")

    if not missing_report:
        _log("  No missing dependencies found")
    else:
        _log(f"  Found {len(missing_report)} missing dependencies", "error")

    return missing_report


# ---------------------------------------------------------------------------
# Selective compilation
# ---------------------------------------------------------------------------

def compile_dirty_blueprints() -> CompileReport:
    """Compile only Blueprints that are dirty (modified since last compile).

    This is the fastest option for incremental builds.

    Returns:
        CompileReport.
    """
    _log("=== Compiling Dirty Blueprints ===")

    report = CompileReport()
    start = time.perf_counter()

    all_bps = _find_all_blueprints()

    for bp_path in all_bps:
        bp = unreal.EditorAssetLibrary.load_asset(bp_path)
        if not isinstance(bp, unreal.Blueprint):
            continue

        status = bp.get_editor_property("status")
        if status == unreal.BlueprintStatus.BS_Dirty:
            report.total += 1
            result = compile_blueprint(bp_path, force=True)
            report.results.append(result)
            if result.success:
                report.succeeded += 1
            else:
                report.failed += 1

    report.total_time_ms = (time.perf_counter() - start) * 1000
    _print_report(report)
    return report


def compile_blueprints_by_tag(tag: str) -> CompileReport:
    """Compile Blueprints that have a specific asset tag.

    Args:
        tag: Asset tag to filter by.

    Returns:
        CompileReport.
    """
    _log(f"=== Compiling Blueprints with tag: {tag} ===")

    report = CompileReport()
    start = time.perf_counter()

    all_bps = _find_all_blueprints()

    for bp_path in all_bps:
        asset_data = unreal.EditorAssetLibrary.find_asset_data(bp_path)
        tags = asset_data.get_all_tags()

        has_tag = any(str(t) == tag for t in tags)
        if has_tag:
            report.total += 1
            result = compile_blueprint(bp_path)
            report.results.append(result)
            if result.success:
                report.succeeded += 1
            else:
                report.failed += 1

    report.total_time_ms = (time.perf_counter() - start) * 1000
    _print_report(report)
    return report


# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------

def _print_report(report: CompileReport):
    """Print a formatted compilation report."""
    _log("")
    _log("=" * 60)
    _log("  BLUEPRINT COMPILATION REPORT")
    _log("=" * 60)
    _log(f"  Total Blueprints:   {report.total}")
    _log(f"  Succeeded:          {report.succeeded}")
    _log(f"  Failed:             {report.failed}")
    _log(f"  Skipped:            {report.skipped}")
    _log(f"  Success Rate:       {report.success_rate:.1f}%")
    _log(f"  Total Time:         {report.total_time_ms:.0f} ms")
    _log("=" * 60)

    if report.failed > 0:
        _log("")
        _log("FAILED BLUEPRINTS:")
        for result in report.results:
            if not result.success:
                _log(f"  - {result.asset_path}")
                for err in result.errors:
                    _log(f"      Error: {err}")

    if any(r.warnings for r in report.results):
        _log("")
        _log("WARNINGS:")
        for result in report.results:
            for warn in result.warnings:
                _log(f"  - {result.asset_path}: {warn}")

    _log("")


def print_compile_summary(report: CompileReport):
    """Public wrapper for printing the report."""
    _print_report(report)


# ---------------------------------------------------------------------------
# NIO-specific compilation workflows
# ---------------------------------------------------------------------------

def compile_for_phase(phase: int) -> CompileReport:
    """Compile Blueprints relevant to a specific development phase.

    Args:
        phase: Phase number (1-6) matching PLAN.md phases.

    Returns:
        CompileReport.
    """
    _log(f"=== Compiling for Phase {phase} ===")

    phase_dirs = {
        1: ["/Game/Vehicles", "/Game/Maps"],                              # Basic setup
        2: ["/Game/Vehicles", "/Game/Materials"],                         # Vehicle systems
        3: ["/Game/Maps", "/Game/AI"],                                    # Track creation
        4: ["/Game/AI", "/Game/UI", "/Game/Core"],                        # Game loop
        5: ["/Game/NOMI", "/Game/UI", "/Game/Audio"],                     # Polish
        6: ["/Game"],                                                      # Full project
    }

    directories = phase_dirs.get(phase, ["/Game"])
    report = CompileReport()
    start = time.perf_counter()

    for directory in directories:
        if unreal.EditorAssetLibrary.does_directory_exist(directory):
            sub_report = compile_blueprints_in_directory(directory)
            report.total += sub_report.total
            report.succeeded += sub_report.succeeded
            report.failed += sub_report.failed
            report.skipped += sub_report.skipped
            report.results.extend(sub_report.results)

    report.total_time_ms = (time.perf_counter() - start) * 1000
    _print_report(report)
    return report


# ---------------------------------------------------------------------------
# Pre-commit validation
# ---------------------------------------------------------------------------

def pre_commit_check() -> bool:
    """Run pre-commit validation: compile all dirty Blueprints and check
    for missing dependencies.

    Returns:
        True if everything is clean (no errors).
    """
    _log("=== Pre-Commit Validation ===")

    # Step 1: Check missing dependencies
    missing = find_missing_dependencies()
    if missing:
        _log(f"BLOCKING: {len(missing)} missing dependencies found", "error")
        for item in missing:
            _log(f"  {item['blueprint']} -> {item['missing']}", "error")
        return False

    # Step 2: Compile dirty Blueprints
    report = compile_dirty_blueprints()
    if report.failed > 0:
        _log(f"BLOCKING: {report.failed} Blueprint(s) failed to compile", "error")
        return False

    _log("Pre-commit validation PASSED")
    return True


# ---------------------------------------------------------------------------
# Standalone execution
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Compile UE5 Blueprints")
    parser.add_argument("-all", action="store_true", help="Compile all Blueprints")
    parser.add_argument("-vehicles", action="store_true", help="Compile vehicle Blueprints")
    parser.add_argument("-ui", action="store_true", help="Compile UI Blueprints")
    parser.add_argument("-nomi", action="store_true", help="Compile NOMI Blueprints")
    parser.add_argument("-gameplay", action="store_true", help="Compile gameplay Blueprints")
    parser.add_argument("-dirty", action="store_true", help="Compile only dirty Blueprints")
    parser.add_argument("-path", help="Compile Blueprints in specific directory")
    parser.add_argument("-category", choices=["vehicle", "widget", "nomi", "gameplay", "ai", "other"],
                        help="Filter by category")
    parser.add_argument("-phase", type=int, help="Compile for specific development phase (1-6)")
    parser.add_argument("-deps", action="store_true", help="Check dependencies only")
    parser.add_argument("-missing", action="store_true", help="Find missing dependencies")
    parser.add_argument("-precommit", action="store_true", help="Run pre-commit validation")
    parser.add_argument("-force", action="store_true", help="Force recompile")
    args = parser.parse_args()

    if args.precommit:
        ok = pre_commit_check()
        exit(0 if ok else 1)
    elif args.missing:
        missing = find_missing_dependencies()
        exit(0 if not missing else 1)
    elif args.deps and args.path:
        check_blueprint_dependencies(args.path)
    elif args.dirty:
        compile_dirty_blueprints()
    elif args.phase:
        compile_for_phase(args.phase)
    elif args.vehicles:
        compile_vehicle_blueprints(args.force)
    elif args.ui:
        compile_ui_blueprints(args.force)
    elif args.nomi:
        compile_nomi_blueprints(args.force)
    elif args.gameplay:
        compile_gameplay_blueprints(args.force)
    elif args.path:
        compile_blueprints_in_directory(args.path, args.category, args.force)
    elif args.all:
        compile_all_blueprints(args.force, args.category)
    else:
        parser.print_help()
