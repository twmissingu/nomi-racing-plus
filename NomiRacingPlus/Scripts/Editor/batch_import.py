"""
NIO Racing Plus - Batch Asset Import Script
=============================================
UE5 Editor utility to batch-import FBX/OBJ meshes, textures, audio, and HDR files.

Usage (UE5 Editor Output Log):
    exec(open(r"<PROJECT>/Scripts/Editor/batch_import.py").read())
    import_assets("/path/to/source/dir", "/Game/Vehicles/EP9")

Usage (Command line):
    UnrealEditor-Cmd <PROJECT>.uproject -run=pythonscript -script="<PROJECT>/Scripts/Editor/batch_import.py" -- -source /path/to/source -dest /Game/Vehicles/EP9
"""

import unreal
import os
import json
from pathlib import Path


# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

MESH_IMPORT_TASKS = {
    "fbx": {
        "class": unreal.FbxImportUI,
        "factory": unreal.FbxFactory,
        "task_class": unreal.AssetImportTask,
    },
    "obj": {
        "class": unreal.FbxImportUI,  # OBJ uses the same pipeline
        "factory": unreal.FbxFactory,
        "task_class": unreal.AssetImportTask,
    },
}

# Mesh import uses default factory settings — UE5 auto-detects format (FBX/GLB/OBJ).
# No explicit FbxImportUI configuration: setting removed/renamed properties crashes UE5.7.

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _log(msg: str, level: str = "info"):
    """Log to UE5 Output Log."""
    dispatch = {
        "info": unreal.log,
        "warn": unreal.log_warning,
        "error": unreal.log_error,
    }
    dispatch.get(level, unreal.log)(f"[BatchImport] {msg}")


def _make_import_task(filepath: str, destination_path: str) -> unreal.AssetImportTask:
    """Build an AssetImportTask for the given file."""
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filepath)
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    return task


def _detect_asset_type(filepath: str) -> str:
    """Return 'mesh', 'texture', 'audio', or 'hdr' based on extension."""
    ext = Path(filepath).suffix.lower()
    if ext in (".fbx", ".obj", ".gltf", ".glb"):
        return "mesh"
    if ext in (".png", ".jpg", ".jpeg", ".tga", ".bmp", ".exr", ".tif", ".tiff"):
        return "texture"
    if ext in (".wav", ".mp3", ".ogg", ".flac"):
        return "audio"
    if ext in (".hdr", ".hdri"):
        return "hdr"
    return "unknown"


def _is_normal_map(filepath: str) -> bool:
    """Heuristic: filename contains 'normal' or 'nrm'."""
    name = Path(filepath).stem.lower()
    return any(tag in name for tag in ("normal", "nrm", "_n", "norm"))


def _get_imported_paths(task: unreal.AssetImportTask) -> list[str]:
    """Get imported asset paths from a completed AssetImportTask.

    UE5.7's import_asset_tasks() return value (List[List[Object]]) is unreliable
    — it often returns empty even when imports succeed. This helper reads the
    task's imported_object_paths property (populated by the engine on success)
    and falls back to constructing the expected path from filename stem when
    that property is empty.

    Args:
        task: A completed AssetImportTask.

    Returns:
        List of imported asset path strings.
    """
    # Try the engine-populated property first (most reliable)
    paths = task.get_editor_property("imported_object_paths")
    if paths:
        return list(paths)

    # Fallback: construct expected path from filename stem + destination
    fname = Path(task.get_editor_property("filename")).stem
    dest = task.get_editor_property("destination_path")
    expected = f"{dest}/{fname}"
    if unreal.EditorAssetLibrary.does_asset_exist(expected):
        return [expected]

    # Nothing found
    return []


# ---------------------------------------------------------------------------
# Import functions
# ---------------------------------------------------------------------------

def import_meshes(source_dir: str, destination_path: str,
                  import_as_skeletal: bool = False,
                  combine_meshes: bool = False) -> list[str]:
    """Import all FBX/OBJ/GLB files from source_dir into destination_path.

    Args:
        source_dir: Local filesystem path containing mesh files.
        destination_path: UE5 content path, e.g. "/Game/Vehicles/EP9".
        import_as_skeletal: True for skeletal meshes (vehicles).
        combine_meshes: Combine all meshes into one static mesh.

    Returns:
        List of imported asset paths.
    """
    imported = []
    exts = {".fbx", ".obj", ".gltf", ".glb"}
    files = sorted(
        f for f in Path(source_dir).iterdir()
        if f.suffix.lower() in exts and f.is_file()
    )

    if not files:
        _log(f"No mesh files found in {source_dir}", "warn")
        return imported

    _log(f"Importing {len(files)} mesh file(s) from {source_dir}")

    for fpath in files:
        task = _make_import_task(str(fpath), destination_path)

        # Apply default factory — UE5 auto-detects format (FBX/GLB/OBJ).
        # Avoid explicit FbxImportUI configuration which crashes on UE5.7.
        ext = fpath.suffix.lower()
        if ext in (".fbx", ".obj"):
            task.set_editor_property("factory", unreal.FbxFactory())
        # GLTF/GLB uses UnrealEd's built-in importer — no factory needed.

        try:
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        except Exception as e:
            _log(f"  Import task failed for {fpath.name}: {e}", "error")
            continue

        paths = _get_imported_paths(task)
        if paths:
            for path in paths:
                imported.append(path)
                _log(f"  Imported: {path}")
        else:
            _log(f"  Failed to import: {fpath}", "error")

    _log(f"Mesh import complete: {len(imported)} asset(s)")
    return imported


def import_textures(source_dir: str, destination_path: str,
                    is_normal_map_dir: bool = False) -> list[str]:
    """Import all texture files from source_dir.

    Args:
        source_dir: Local filesystem path containing texture files.
        destination_path: UE5 content path.
        is_normal_map_dir: If True, treat all textures as normal maps.

    Returns:
        List of imported asset paths.
    """
    imported = []
    exts = {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".exr", ".tif", ".tiff"}
    files = sorted(
        f for f in Path(source_dir).iterdir()
        if f.suffix.lower() in exts and f.is_file()
    )

    if not files:
        _log(f"No texture files found in {source_dir}", "warn")
        return imported

    _log(f"Importing {len(files)} texture(s) from {source_dir}")

    tasks = []
    for fpath in files:
        task = _make_import_task(str(fpath), destination_path)
        tasks.append(task)

    # Bulk import — read imported_object_paths from each task (more reliable
    # than the function return value in UE5.7)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    for task in tasks:
        paths = _get_imported_paths(task)
        if paths:
            for path in paths:
                imported.append(path)
                _log(f"  Imported: {path}")

                # Post-import: configure compression settings
                _configure_texture(path, is_normal_map_dir or _is_normal_map(task.get_editor_property("filename")))
        else:
            _log(f"  Failed: {task.get_editor_property('filename')}", "error")

    _log(f"Texture import complete: {len(imported)} asset(s)")
    return imported


def _configure_texture(asset_path: str, is_normal: bool):
    """Set SRGB and compression on an imported texture."""
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not isinstance(texture, unreal.Texture2D):
        return

    if is_normal:
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        _log(f"  Configured as normal map: {asset_path}")
    else:
        texture.set_editor_property("srgb", True)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
        _log(f"  Configured as sRGB: {asset_path}")


def import_audio(source_dir: str, destination_path: str) -> list[str]:
    """Import all audio files from source_dir.

    Args:
        source_dir: Local filesystem path containing audio files.
        destination_path: UE5 content path.

    Returns:
        List of imported asset paths.
    """
    imported = []
    exts = {".wav", ".mp3", ".ogg", ".flac"}
    files = sorted(
        f for f in Path(source_dir).iterdir()
        if f.suffix.lower() in exts and f.is_file()
    )

    if not files:
        _log(f"No audio files found in {source_dir}", "warn")
        return imported

    _log(f"Importing {len(files)} audio file(s) from {source_dir}")

    tasks = []
    for fpath in files:
        task = _make_import_task(str(fpath), destination_path)
        tasks.append(task)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    for task in tasks:
        paths = _get_imported_paths(task)
        if paths:
            for path in paths:
                imported.append(path)
                _log(f"  Imported: {path}")
        else:
            _log(f"  Failed: {task.get_editor_property('filename')}", "error")

    _log(f"Audio import complete: {len(imported)} asset(s)")
    return imported


def import_hdr(source_dir: str, destination_path: str) -> list[str]:
    """Import HDR/HDRI skybox textures.

    Args:
        source_dir: Local filesystem path containing HDR files.
        destination_path: UE5 content path, e.g. "/Game/Textures/HDR".

    Returns:
        List of imported asset paths.
    """
    imported = []
    exts = {".hdr", ".hdri", ".exr"}
    files = sorted(
        f for f in Path(source_dir).iterdir()
        if f.suffix.lower() in exts and f.is_file()
    )

    if not files:
        _log(f"No HDR files found in {source_dir}", "warn")
        return imported

    _log(f"Importing {len(files)} HDR file(s) from {source_dir}")

    tasks = []
    for fpath in files:
        task = _make_import_task(str(fpath), destination_path)
        tasks.append(task)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    for task in tasks:
        paths = _get_imported_paths(task)
        if paths:
            for path in paths:
                imported.append(path)
                _log(f"  Imported: {path}")

                # Configure HDR texture settings
                texture = unreal.EditorAssetLibrary.load_asset(path)
                if isinstance(texture, unreal.TextureCube):
                    texture.set_editor_property("srgb", False)
                    _log(f"  Configured HDR cubemap: {path}")
        else:
            _log(f"  Failed: {task.get_editor_property('filename')}", "error")

    _log(f"HDR import complete: {len(imported)} asset(s)")
    return imported


# ---------------------------------------------------------------------------
# Orchestrator
# ---------------------------------------------------------------------------

def import_assets(source_dir: str, destination_path: str,
                  asset_type: str = "auto",
                  import_as_skeletal: bool = False) -> dict[str, list[str]]:
    """Batch-import all assets from source_dir into UE5.

    Args:
        source_dir: Root directory to scan recursively.
        destination_path: UE5 content base path.
        asset_type: "mesh", "texture", "audio", "hdr", or "auto" (detect).
        import_as_skeletal: Import meshes as skeletal (for vehicles).

    Returns:
        Dict mapping asset type to list of imported paths.
    """
    results: dict[str, list[str]] = {
        "mesh": [],
        "texture": [],
        "audio": [],
        "hdr": [],
    }

    source = Path(source_dir)
    if not source.exists():
        _log(f"Source directory does not exist: {source_dir}", "error")
        return results

    _log(f"=== Batch Import Start ===")
    _log(f"Source:      {source_dir}")
    _log(f"Destination: {destination_path}")
    _log(f"Type filter: {asset_type}")

    if asset_type != "auto":
        # Import everything in the directory as the specified type
        type_map = {
            "mesh": lambda: import_meshes(source_dir, destination_path, import_as_skeletal),
            "texture": lambda: import_textures(source_dir, destination_path),
            "audio": lambda: import_audio(source_dir, destination_path),
            "hdr": lambda: import_hdr(source_dir, destination_path),
        }
        if asset_type in type_map:
            results[asset_type] = type_map[asset_type]()
        else:
            _log(f"Unknown asset type: {asset_type}", "error")
    else:
        # Auto-detect: walk the directory tree
        for dirpath, dirnames, filenames in os.walk(source_dir):
            mesh_files = []
            texture_files = []
            audio_files = []
            hdr_files = []

            for fname in filenames:
                ftype = _detect_asset_type(os.path.join(dirpath, fname))
                if ftype == "mesh":
                    mesh_files.append(fname)
                elif ftype == "texture":
                    texture_files.append(fname)
                elif ftype == "audio":
                    audio_files.append(fname)
                elif ftype == "hdr":
                    hdr_files.append(fname)

            # Compute relative UE5 destination path
            rel = os.path.relpath(dirpath, source_dir)
            if rel == ".":
                ue_dest = destination_path
            else:
                ue_dest = f"{destination_path}/{rel.replace(os.sep, '/')}"

            if mesh_files:
                _log(f"[auto] Meshes in {dirpath}")
                results["mesh"].extend(import_meshes(dirpath, ue_dest, import_as_skeletal))
            if texture_files:
                _log(f"[auto] Textures in {dirpath}")
                results["texture"].extend(import_textures(dirpath, ue_dest))
            if audio_files:
                _log(f"[auto] Audio in {dirpath}")
                results["audio"].extend(import_audio(dirpath, ue_dest))
            if hdr_files:
                _log(f"[auto] HDR in {dirpath}")
                results["hdr"].extend(import_hdr(dirpath, ue_dest))

    total = sum(len(v) for v in results.values())
    _log(f"=== Batch Import Complete: {total} asset(s) ===")
    return results


# ---------------------------------------------------------------------------
# NIO-specific presets
# ---------------------------------------------------------------------------

def import_nio_vehicle(vehicle_name: str, source_dir: str):
    """Import a NIO vehicle with the standard directory layout.

    Expected source_dir structure:
        source_dir/
            LOD0.fbx
            LOD1.fbx
            LOD2.fbx
            Textures/
                T_Vehicle_Body_D.png   (diffuse)
                T_Vehicle_Body_N.png   (normal)
                T_Vehicle_Body_M.png   (metallic/roughness)
                T_Vehicle_Glass_D.png
                ...

    Args:
        vehicle_name: "EP9", "ET7", "ES7", or "ET5".
        source_dir: Local path to the processed vehicle directory.
    """
    dest_base = f"/Game/Vehicles/{vehicle_name}"
    source = Path(source_dir)

    _log(f"=== Importing NIO {vehicle_name} ===")

    # Import LOD meshes
    for lod in ("LOD0", "LOD1", "LOD2"):
        lod_file = source / f"{lod}.fbx"
        if lod_file.exists():
            import_meshes(str(lod_file.parent), f"{dest_base}/Meshes", import_as_skeletal=False)
        else:
            _log(f"  LOD file not found: {lod_file}", "warn")

    # Import textures
    tex_dir = source / "Textures"
    if tex_dir.exists():
        # Diffuse / ORM textures (sRGB)
        import_textures(str(tex_dir), f"{dest_base}/Textures", is_normal_map_dir=False)
    else:
        _log(f"  Texture directory not found: {tex_dir}", "warn")

    _log(f"NIO {vehicle_name} import complete")


def import_carla_assets(carla_content_dir: str, town: str = "Town03"):
    """Import CARLA town map and vehicle assets.

    Args:
        carla_content_dir: Path to CARLA's Unreal/CarlaUnreal/Content/Carla directory.
        town: Town name, e.g. "Town03".
    """
    carla = Path(carla_content_dir)

    _log(f"=== Importing CARLA {town} ===")

    # Import town map
    maps_dir = carla / "Maps" / town
    if maps_dir.exists():
        import_meshes(str(maps_dir), f"/Game/Maps/{town}", import_as_skeletal=False)
    else:
        _log(f"  Town directory not found: {maps_dir}", "error")

    # Import CARLA vehicles (for GT mode)
    vehicles_dir = carla / "Vehicles"
    if vehicles_dir.exists():
        for vehicle_type in sorted(vehicles_dir.iterdir()):
            if vehicle_type.is_dir():
                dest = f"/Game/Vehicles/GT/{vehicle_type.name}"
                import_meshes(str(vehicle_type), dest, import_as_skeletal=False)
    else:
        _log(f"  Vehicles directory not found: {vehicles_dir}", "warn")

    _log(f"CARLA {town} import complete")


# ---------------------------------------------------------------------------
# Standalone execution
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Batch import assets into UE5")
    parser.add_argument("-source", required=True, help="Source directory")
    parser.add_argument("-dest", required=True, help="UE5 destination path")
    parser.add_argument("-type", default="auto",
                        choices=["auto", "mesh", "texture", "audio", "hdr"],
                        help="Asset type filter")
    parser.add_argument("-skeletal", action="store_true",
                        help="Import meshes as skeletal")
    args = parser.parse_args()

    import_assets(args.source, args.dest, args.type, args.skeletal)
