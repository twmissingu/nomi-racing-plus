"""
NIO Racing Plus - Asset Validation Tool
========================================
Validates game assets against project standards for polygon counts, texture
sizes, material compatibility, and naming conventions.

Supports two modes:
  1. UE5 Editor mode  -- uses the `unreal` module for live asset inspection.
  2. Standalone mode   -- scans the filesystem for FBX/PNG/JSON metadata.

Usage (UE5 Editor Output Log):
    exec(open(r"<PROJECT>/Scripts/Editor/asset_validator.py").read())
    report = validate_all_assets()
    report = validate_vehicle("EP9")
    report = validate_directory("/Game/Vehicles/EP9")

Usage (Command line -- standalone):
    python asset_validator.py --project-root /path/to/NomiRacingPlus
    python asset_validator.py --vehicle EP9 --project-root /path/to/NomiRacingPlus
    python asset_validator.py --content-dir /path/to/Content --project-root /path/to/NomiRacingPlus
"""

from __future__ import annotations

import json
import os
import re
import sys
import math
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

# ---------------------------------------------------------------------------
# Try importing unreal; fall back to standalone mode
# ---------------------------------------------------------------------------

try:
    import unreal
    UE5_MODE = True
except ImportError:
    UE5_MODE = False

# ---------------------------------------------------------------------------
# Configuration loader
# ---------------------------------------------------------------------------

_CONFIG_CACHE: dict | None = None
_VEHICLE_CONFIG_CACHE: dict | None = None

_SCRIPT_DIR = Path(__file__).resolve().parent
_PROJECT_ROOT = _SCRIPT_DIR.parent.parent  # NomiRacingPlus/

def _load_config() -> dict:
    """Load validation_config.json (cached)."""
    global _CONFIG_CACHE
    if _CONFIG_CACHE is not None:
        return _CONFIG_CACHE

    config_path = _SCRIPT_DIR / "validation_config.json"
    if not config_path.exists():
        _log(f"Validation config not found: {config_path}", "error")
        _CONFIG_CACHE = {}
        return _CONFIG_CACHE

    with open(config_path, "r", encoding="utf-8") as f:
        _CONFIG_CACHE = json.load(f)
    return _CONFIG_CACHE


def _load_vehicle_config() -> dict:
    """Load VehicleConfig.json for polygon budgets (cached)."""
    global _VEHICLE_CONFIG_CACHE
    if _VEHICLE_CONFIG_CACHE is not None:
        return _VEHICLE_CONFIG_CACHE

    vc_path = _PROJECT_ROOT / "Content" / "Vehicles" / "VehicleConfig.json"
    if not vc_path.exists():
        _VEHICLE_CONFIG_CACHE = {}
        return _VEHICLE_CONFIG_CACHE

    with open(vc_path, "r", encoding="utf-8") as f:
        _VEHICLE_CONFIG_CACHE = json.load(f)
    return _VEHICLE_CONFIG_CACHE


# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------

def _log(msg: str, level: str = "info"):
    prefix = "[AssetValidator]"
    if UE5_MODE:
        dispatch = {
            "info": unreal.log,
            "warn": unreal.log_warning,
            "error": unreal.log_error,
        }
        dispatch.get(level, unreal.log)(f"{prefix} {msg}")
    else:
        tag = level.upper()
        print(f"{prefix} [{tag}] {msg}", file=sys.stderr)


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class ValidationIssue:
    """A single validation finding."""
    severity: str        # "error", "warning", "info"
    category: str        # "polygon", "texture", "material", "naming", "lod", "structure"
    asset_path: str
    message: str
    details: str = ""

    def __str__(self) -> str:
        s = f"[{self.severity.upper():7s}] [{self.category:10s}] {self.asset_path}: {self.message}"
        if self.details:
            s += f"\n           {self.details}"
        return s


@dataclass
class ValidationReport:
    """Aggregated validation results."""
    issues: list[ValidationIssue] = field(default_factory=list)
    checked_assets: int = 0
    passed_assets: int = 0

    @property
    def errors(self) -> list[ValidationIssue]:
        return [i for i in self.issues if i.severity == "error"]

    @property
    def warnings(self) -> list[ValidationIssue]:
        return [i for i in self.issues if i.severity == "warning"]

    @property
    def infos(self) -> list[ValidationIssue]:
        return [i for i in self.issues if i.severity == "info"]

    @property
    def has_errors(self) -> bool:
        return len(self.errors) > 0

    @property
    def passed(self) -> bool:
        return not self.has_errors

    def add(self, issue: ValidationIssue):
        self.issues.append(issue)

    def merge(self, other: "ValidationReport"):
        self.issues.extend(other.issues)
        self.checked_assets += other.checked_assets
        self.passed_assets += other.passed_assets

    def summary(self) -> str:
        lines = [
            "",
            "=" * 70,
            "  ASSET VALIDATION REPORT",
            "=" * 70,
            f"  Assets checked : {self.checked_assets}",
            f"  Assets passed  : {self.passed_assets}",
            f"  Errors         : {len(self.errors)}",
            f"  Warnings       : {len(self.warnings)}",
            f"  Info           : {len(self.infos)}",
            f"  Result         : {'PASS' if self.passed else 'FAIL'}",
            "=" * 70,
        ]
        if self.issues:
            lines.append("")
            for issue in self.issues:
                lines.append(f"  {issue}")
            lines.append("")
            lines.append("=" * 70)
        return "\n".join(lines)


# ---------------------------------------------------------------------------
# Naming convention helpers
# ---------------------------------------------------------------------------

def _get_naming_rules() -> dict:
    config = _load_config()
    return config.get("naming_conventions", {}).get("prefixes", {})


def _get_suffix_rules() -> dict:
    config = _load_config()
    return config.get("naming_conventions", {}).get("suffixes", {})


def _detect_asset_type_from_name(name: str) -> str | None:
    """Guess asset type from its prefix."""
    naming = _get_naming_rules()
    for asset_type, rule in naming.items():
        prefix = rule.get("prefix", "")
        if name.startswith(prefix):
            return asset_type
    return None


def _validate_naming(name: str, expected_type: str | None = None) -> list[ValidationIssue]:
    """Check a single asset name against naming conventions."""
    issues: list[ValidationIssue] = []
    naming = _get_naming_rules()

    if expected_type and expected_type in naming:
        rule = naming[expected_type]
        pattern = rule.get("pattern", "")
        prefix = rule.get("prefix", "")
        if prefix and not name.startswith(prefix):
            issues.append(ValidationIssue(
                severity="warning",
                category="naming",
                asset_path=name,
                message=f"Expected prefix '{prefix}' for {expected_type}",
                details=f"Name '{name}' does not start with '{prefix}'"
            ))
        elif pattern and not re.match(pattern, name):
            issues.append(ValidationIssue(
                severity="warning",
                category="naming",
                asset_path=name,
                message=f"Name does not match pattern for {expected_type}",
                details=f"Expected pattern: {pattern}"
            ))
    else:
        # Auto-detect: check if any known prefix matches
        detected = _detect_asset_type_from_name(name)
        if detected:
            rule = naming[detected]
            pattern = rule.get("pattern", "")
            if pattern and not re.match(pattern, name):
                issues.append(ValidationIssue(
                    severity="warning",
                    category="naming",
                    asset_path=name,
                    message=f"Name has prefix '{rule['prefix']}' but does not match {detected} pattern",
                    details=f"Expected pattern: {pattern}"
                ))
        # If no prefix at all, it may be a legacy/auto-generated name -- just info
        elif not any(name.startswith(r["prefix"]) for r in naming.values() if r.get("prefix")):
            issues.append(ValidationIssue(
                severity="info",
                category="naming",
                asset_path=name,
                message="Asset has no recognized type prefix",
                details="Consider renaming with T_, M_, SM_, BP_, etc."
            ))

    return issues


def _validate_texture_suffix(name: str, filepath: str = "") -> list[ValidationIssue]:
    """Check texture naming suffixes (_D, _N, _M, etc.)."""
    issues: list[ValidationIssue] = []
    suffixes = _get_suffix_rules()

    texture_suffixes = [
        suffixes.get("texture_diffuse", "_D"),
        suffixes.get("texture_normal", "_N"),
        suffixes.get("texture_metallic", "_M"),
        suffixes.get("texture_roughness", "_R"),
        suffixes.get("texture_ao", "_AO"),
        suffixes.get("texture_emissive", "_E"),
        suffixes.get("texture_opacity", "_O"),
        suffixes.get("texture_height", "_H"),
    ]

    has_suffix = any(name.endswith(s) for s in texture_suffixes if s)
    if not has_suffix and not any(name.endswith(s) for s in ["_D", "_N", "_M", "_R", "_AO", "_E", "_O", "_H"]):
        issues.append(ValidationIssue(
            severity="info",
            category="naming",
            asset_path=filepath or name,
            message="Texture missing type suffix",
            details=f"Expected one of: {', '.join(s for s in texture_suffixes if s)}"
        ))

    return issues


# ---------------------------------------------------------------------------
# Polygon count validation (standalone -- reads FBX metadata via simple parse)
# ---------------------------------------------------------------------------

def _count_fbx_triangles_simple(filepath: str) -> int | None:
    """Estimate triangle count from an FBX binary file by counting PolygonVertexIndex entries.

    This is a rough heuristic for standalone mode.  For accurate counts use
    the UE5 Editor mode which reads actual mesh data.
    """
    try:
        with open(filepath, "rb") as f:
            data = f.read()
        # FBX binary stores "PolygonVertexIndex" as a property name
        marker = b"PolygonVertexIndex"
        count = data.count(marker)
        if count > 0:
            # Each occurrence is one polygon definition; triangulate quads
            # Rough: most vehicle meshes are triangulated already
            return count
    except Exception:
        pass
    return None


def _get_mesh_triangle_count(asset_path: str) -> int | None:
    """Get triangle count for a mesh (UE5 Editor mode)."""
    if not UE5_MODE:
        return None

    try:
        mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
        if mesh is None:
            return None

        if isinstance(mesh, unreal.StaticMesh):
            # Get LOD 0 triangle count
            render_data = mesh.get_editor_property("render_data")
            if render_data and len(render_data) > 0:
                lod0 = render_data[0]
                sections = lod0.get_editor_property("sections")
                total_tris = 0
                if sections:
                    for section in sections:
                        num_tris = section.get_editor_property("num_triangles")
                        total_tris += num_tris
                    return total_tris
        elif isinstance(mesh, unreal.SkeletalMesh):
            # Skeletal mesh LOD info
            lods = mesh.get_editor_property("lod_info")
            if lods and len(lods) > 0:
                lod0 = lods[0]
                return lod0.get_editor_property("num_triangles") if hasattr(lod0, "num_triangles") else None
    except Exception as e:
        _log(f"Could not read triangle count for {asset_path}: {e}", "warn")

    return None


def _validate_polygon_count(asset_path: str, triangle_count: int,
                            asset_category: str, vehicle_name: str = "",
                            lod_level: str = "") -> list[ValidationIssue]:
    """Validate polygon count against budgets."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    budgets = config.get("polygon_budgets", {})

    if asset_category == "vehicle" and vehicle_name and lod_level:
        vehicle_budgets = budgets.get("vehicle", {})
        lod_budgets = vehicle_budgets.get(lod_level, {})
        budget = lod_budgets.get(vehicle_name, lod_budgets.get("default", {}))

        if budget:
            min_faces = budget.get("min", 0)
            max_faces = budget.get("max", float("inf"))
            target = budget.get("target", 0)

            if triangle_count > max_faces:
                issues.append(ValidationIssue(
                    severity="error",
                    category="polygon",
                    asset_path=asset_path,
                    message=f"Polygon count ({triangle_count:,}) exceeds maximum ({max_faces:,}) for {vehicle_name} {lod_level}",
                    details=f"Target: {target:,} faces. Reduce polygon count with Blender Decimate."
                ))
            elif triangle_count < min_faces:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="polygon",
                    asset_path=asset_path,
                    message=f"Polygon count ({triangle_count:,}) below minimum ({min_faces:,}) for {vehicle_name} {lod_level}",
                    details=f"Model may lack sufficient detail. Target: {target:,} faces."
                ))
            else:
                deviation = abs(triangle_count - target) / target * 100 if target > 0 else 0
                if deviation > 20:
                    issues.append(ValidationIssue(
                        severity="info",
                        category="polygon",
                        asset_path=asset_path,
                        message=f"Polygon count ({triangle_count:,}) deviates {deviation:.0f}% from target ({target:,}) for {vehicle_name} {lod_level}",
                    ))

    elif asset_category == "environment":
        env_budgets = budgets.get("environment", {})
        # Try to guess the sub-category from the path
        sub_cat = "default"
        lower_path = asset_path.lower()
        if "building" in lower_path:
            sub_cat = "building"
        elif "road" in lower_path:
            sub_cat = "road_segment"
        elif "prop" in lower_path or "furniture" in lower_path:
            sub_cat = "prop"
        elif "tree" in lower_path or "bush" in lower_path or "grass" in lower_path:
            sub_cat = "vegetation"

        budget = env_budgets.get(sub_cat, env_budgets.get("default", {}))
        max_faces = budget.get("max", float("inf"))
        warn_faces = budget.get("warn", float("inf"))

        if triangle_count > max_faces:
            issues.append(ValidationIssue(
                severity="error",
                category="polygon",
                asset_path=asset_path,
                message=f"Environment mesh ({triangle_count:,} tris) exceeds budget ({max_faces:,}) for {sub_cat}",
            ))
        elif triangle_count > warn_faces:
            issues.append(ValidationIssue(
                severity="warning",
                category="polygon",
                asset_path=asset_path,
                message=f"Environment mesh ({triangle_count:,} tris) approaching budget ({max_faces:,}) for {sub_cat}",
            ))

    return issues


# ---------------------------------------------------------------------------
# Texture validation
# ---------------------------------------------------------------------------

def _get_texture_info_ue5(asset_path: str) -> dict | None:
    """Get texture metadata from UE5 Editor."""
    if not UE5_MODE:
        return None
    try:
        texture = unreal.EditorAssetLibrary.load_asset(asset_path)
        if texture is None or not isinstance(texture, unreal.Texture2D):
            return None
        return {
            "width": texture.get_editor_property("size_x"),
            "height": texture.get_editor_property("size_y"),
            "srgb": texture.get_editor_property("s_rgb"),
            "compression": str(texture.get_editor_property("compression_settings")),
            "has_alpha": texture.get_editor_property("has_alpha_channel"),
            "mip_count": texture.get_editor_property("lod_bias") if hasattr(texture, "lod_bias") else 0,
        }
    except Exception as e:
        _log(f"Could not read texture info for {asset_path}: {e}", "warn")
        return None


def _get_texture_info_standalone(filepath: str) -> dict | None:
    """Get texture dimensions from file header (PNG/TGA/BMP/JPEG)."""
    try:
        with open(filepath, "rb") as f:
            header = f.read(32)

        ext = Path(filepath).suffix.lower()

        # PNG
        if ext == ".png" and header[:8] == b"\x89PNG\r\n\x1a\n":
            w = int.from_bytes(header[16:20], "big")
            h = int.from_bytes(header[20:24], "big")
            return {"width": w, "height": h, "format": "png"}

        # BMP
        if ext == ".bmp" and header[:2] == b"BM":
            w = int.from_bytes(header[18:22], "little")
            h = int.from_bytes(header[22:26], "little")
            return {"width": w, "height": h, "format": "bmp"}

        # JPEG
        if ext in (".jpg", ".jpeg") and header[:2] == b"\xff\xd8":
            # JPEG dimension parsing is more complex; skip for now
            return {"width": 0, "height": 0, "format": "jpeg", "note": "dimensions not parsed"}

        # TGA -- header byte 12-15 contain width/height (little-endian)
        if ext == ".tga":
            if len(header) >= 16:
                w = int.from_bytes(header[12:14], "little")
                h = int.from_bytes(header[14:16], "little")
                return {"width": w, "height": h, "format": "tga"}

        # HDR/EXR -- skip dimension parsing in standalone
        if ext in (".hdr", ".hdri", ".exr", ".tif", ".tiff"):
            return {"width": 0, "height": 0, "format": ext.lstrip(".")}

    except Exception:
        pass
    return None


def _is_power_of_two(n: int) -> bool:
    return n > 0 and (n & (n - 1)) == 0


def _validate_texture(asset_path: str, info: dict, texture_category: str) -> list[ValidationIssue]:
    """Validate a single texture against rules."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    rules = config.get("texture_rules", {}).get(texture_category, {})
    global_rules = config.get("texture_rules", {}).get("global", {})

    if not rules:
        rules = config.get("texture_rules", {}).get("environment", {})

    w = info.get("width", 0)
    h = info.get("height", 0)
    max_size = rules.get("max_size", 4096)
    min_size = rules.get("min_size", 64)
    recommended = rules.get("recommended_size", 2048)
    require_pot = rules.get("power_of_two", True)

    if w > 0 and h > 0:
        larger = max(w, h)
        smaller = min(w, h)

        if larger > max_size:
            issues.append(ValidationIssue(
                severity="error",
                category="texture",
                asset_path=asset_path,
                message=f"Texture size {w}x{h} exceeds maximum {max_size}px for {texture_category}",
                details=f"Resize to {recommended}x{recommended} or smaller."
            ))

        if larger < min_size:
            issues.append(ValidationIssue(
                severity="warning",
                category="texture",
                asset_path=asset_path,
                message=f"Texture size {w}x{h} below minimum {min_size}px",
            ))

        if require_pot:
            if not _is_power_of_two(w) or not _is_power_of_two(h):
                issues.append(ValidationIssue(
                    severity="warning",
                    category="texture",
                    asset_path=asset_path,
                    message=f"Texture size {w}x{h} is not power-of-two",
                    details="UE5 performs best with power-of-two textures (256, 512, 1024, 2048, 4096)."
                ))

        if w != h and texture_category != "ui":
            issues.append(ValidationIssue(
                severity="info",
                category="texture",
                asset_path=asset_path,
                message=f"Non-square texture {w}x{h} -- verify this is intentional",
            ))

    # File size check
    if "file_size_mb" in info:
        file_mb = info["file_size_mb"]
        max_mb = global_rules.get("max_file_size_mb", 50)
        warn_mb = global_rules.get("warn_file_size_mb", 20)
        if file_mb > max_mb:
            issues.append(ValidationIssue(
                severity="error",
                category="texture",
                asset_path=asset_path,
                message=f"Texture file size ({file_mb:.1f} MB) exceeds {max_mb} MB limit",
            ))
        elif file_mb > warn_mb:
            issues.append(ValidationIssue(
                severity="warning",
                category="texture",
                asset_path=asset_path,
                message=f"Texture file size ({file_mb:.1f} MB) is large (warn threshold: {warn_mb} MB)",
            ))

    return issues


def _validate_required_texture_maps(texture_paths: list[str], vehicle_name: str) -> list[ValidationIssue]:
    """Check that required texture maps (_D, _N, _M) exist for a vehicle."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    required = config.get("texture_rules", {}).get("vehicle", {}).get("required_maps", ["_D", "_N", "_M"])

    found_suffixes = set()
    for p in texture_paths:
        stem = Path(p).stem.upper()
        for suffix in required:
            if stem.endswith(suffix.upper()):
                found_suffixes.add(suffix)

    missing = [s for s in required if s not in found_suffixes]
    if missing:
        issues.append(ValidationIssue(
            severity="warning",
            category="texture",
            asset_path=f"/Game/Vehicles/{vehicle_name}/Textures",
            message=f"Missing required texture maps for {vehicle_name}: {', '.join(missing)}",
            details=f"Required: {', '.join(required)}. Found: {', '.join(found_suffixes) or 'none'}"
        ))

    return issues


# ---------------------------------------------------------------------------
# Material validation
# ---------------------------------------------------------------------------

def _validate_material_compatibility_ue5(asset_path: str) -> list[ValidationIssue]:
    """Check material properties in UE5 Editor mode."""
    issues: list[ValidationIssue] = []
    if not UE5_MODE:
        return issues

    try:
        mat = unreal.EditorAssetLibrary.load_asset(asset_path)
        if mat is None:
            issues.append(ValidationIssue(
                severity="error",
                category="material",
                asset_path=asset_path,
                message="Material could not be loaded",
            ))
            return issues

        if isinstance(mat, unreal.Material):
            # Check shading model
            shading = mat.get_editor_property("shading_model")
            blend = mat.get_editor_property("blend_mode")

            # Glass should be translucent
            name_lower = Path(asset_path).stem.lower()
            if "glass" in name_lower and blend != unreal.BlendMode.BLEND_TRANSLUCENT:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="material",
                    asset_path=asset_path,
                    message="Glass material should use translucent blend mode",
                    details=f"Current blend mode: {blend}"
                ))

            # Vehicle paint should use clear coat
            if "paint" in name_lower and shading != unreal.MaterialShadingModel.MSM_CLEAR_COAT:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="material",
                    asset_path=asset_path,
                    message="Vehicle paint material should use Clear Coat shading model",
                    details=f"Current shading model: {shading}"
                ))

            # Two-sided check for non-vehicle materials
            two_sided = mat.get_editor_property("two_sided")
            if two_sided and "vehicle" in name_lower and "glass" not in name_lower:
                issues.append(ValidationIssue(
                    severity="info",
                    category="material",
                    asset_path=asset_path,
                    message="Vehicle material is two-sided -- verify this is intentional",
                ))

        elif isinstance(mat, unreal.MaterialInstanceConstant):
            # Check parent reference
            parent = mat.get_editor_property("parent")
            if parent is None:
                issues.append(ValidationIssue(
                    severity="error",
                    category="material",
                    asset_path=asset_path,
                    message="Material instance has no parent material",
                ))

    except Exception as e:
        _log(f"Material validation error for {asset_path}: {e}", "warn")

    return issues


def _validate_material_naming(asset_path: str) -> list[ValidationIssue]:
    """Check material naming conventions."""
    name = Path(asset_path).stem
    issues: list[ValidationIssue] = []

    if name.startswith("MI_"):
        issues.extend(_validate_naming(name, "material_instance"))
    elif name.startswith("M_"):
        issues.extend(_validate_naming(name, "material"))
    else:
        issues.append(ValidationIssue(
            severity="warning",
            category="naming",
            asset_path=asset_path,
            message="Material does not follow M_/MI_ naming convention",
        ))

    return issues


def _check_required_master_materials_exist(content_dir: str) -> list[ValidationIssue]:
    """Verify that all required master materials are present."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    required = config.get("material_rules", {}).get("required_master_materials", [])

    for mat_name in required:
        if UE5_MODE:
            path = f"/Game/Materials/{mat_name}"
            # Check vehicle and environment paths
            found = False
            for sub in ["Vehicles", "Environment", "UI"]:
                test_path = f"/Game/Materials/{sub}/{mat_name}"
                if unreal.EditorAssetLibrary.does_asset_exist(test_path):
                    found = True
                    break
            if not found:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="material",
                    asset_path=mat_name,
                    message=f"Required master material not found: {mat_name}",
                    details="Run material_setup.create_all_master_materials() to create it."
                ))
        else:
            # Standalone: check Content directory
            materials_dir = Path(content_dir) / "Materials"
            found = False
            if materials_dir.exists():
                for f in materials_dir.rglob(f"{mat_name}.*"):
                    found = True
                    break
            if not found:
                issues.append(ValidationIssue(
                    severity="info",
                    category="material",
                    asset_path=mat_name,
                    message=f"Master material file not found in Content: {mat_name}",
                ))

    return issues


# ---------------------------------------------------------------------------
# LOD validation
# ---------------------------------------------------------------------------

def _validate_lod_chain(mesh_path: str, triangle_counts: dict[str, int],
                        vehicle_name: str) -> list[ValidationIssue]:
    """Validate LOD chain for a vehicle mesh."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    lod_rules = config.get("lod_rules", {})
    required_levels = lod_rules.get("required_lod_levels", 3)
    reduction = lod_rules.get("face_reduction_ratio", {})

    available_lods = sorted(triangle_counts.keys())
    if len(available_lods) < required_levels:
        issues.append(ValidationIssue(
            severity="warning",
            category="lod",
            asset_path=mesh_path,
            message=f"Only {len(available_lods)} LOD levels found, expected {required_levels}",
            details=f"Found: {', '.join(available_lods)}. Generate LODs with lod_generation.py."
        ))

    # Check reduction ratios between consecutive LODs
    for i in range(len(available_lods) - 1):
        lod_a = available_lods[i]
        lod_b = available_lods[i + 1]
        count_a = triangle_counts[lod_a]
        count_b = triangle_counts[lod_b]

        if count_a > 0:
            ratio = count_b / count_a
            ratio_key = f"{lod_a}_to_{lod_b}"
            ratio_rules = reduction.get(ratio_key, {})
            min_ratio = ratio_rules.get("min", 0.1)
            max_ratio = ratio_rules.get("max", 0.5)

            if ratio > max_ratio:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="lod",
                    asset_path=mesh_path,
                    message=f"LOD reduction from {lod_a} to {lod_b} is too small ({ratio:.2f}, expected {min_ratio:.2f}-{max_ratio:.2f})",
                    details=f"{lod_a}: {count_a:,} tris, {lod_b}: {count_b:,} tris. Consider more aggressive decimation."
                ))
            elif ratio < min_ratio:
                issues.append(ValidationIssue(
                    severity="warning",
                    category="lod",
                    asset_path=mesh_path,
                    message=f"LOD reduction from {lod_a} to {lod_b} is too aggressive ({ratio:.2f}, expected {min_ratio:.2f}-{max_ratio:.2f})",
                    details=f"{lod_a}: {count_a:,} tris, {lod_b}: {count_b:,} tris. Model may lose too much detail."
                ))

    return issues


# ---------------------------------------------------------------------------
# Directory structure validation
# ---------------------------------------------------------------------------

def _validate_directory_structure(content_dir: str, vehicle_name: str = "") -> list[ValidationIssue]:
    """Check that expected directories exist."""
    issues: list[ValidationIssue] = []
    config = _load_config()
    folder_rules = config.get("naming_conventions", {}).get("folder_structure", {})

    if vehicle_name:
        for rule_name, template in folder_rules.items():
            if "{name}" in template:
                expected = template.replace("{name}", vehicle_name)
                if UE5_MODE:
                    ue_path = f"/Game{expected.replace('/Game', '')}"
                    if not unreal.EditorAssetLibrary.does_directory_exist(ue_path):
                        issues.append(ValidationIssue(
                            severity="info",
                            category="structure",
                            asset_path=ue_path,
                            message=f"Expected directory not found: {rule_name}",
                            details=f"Expected at: {ue_path}"
                        ))
                else:
                    fs_path = Path(content_dir) / expected.lstrip("/Game/").lstrip("Content/")
                    if not fs_path.exists():
                        issues.append(ValidationIssue(
                            severity="info",
                            category="structure",
                            asset_path=str(fs_path),
                            message=f"Expected directory not found: {rule_name}",
                        ))

    return issues


# ---------------------------------------------------------------------------
# High-level validation workflows
# ---------------------------------------------------------------------------

def validate_vehicle(vehicle_name: str) -> ValidationReport:
    """Validate all assets for a single NIO vehicle.

    Checks:
        - Polygon counts per LOD against VehicleConfig.json budgets
        - Texture sizes and required maps
        - Material naming and properties
        - LOD chain completeness
        - Directory structure

    Args:
        vehicle_name: "EP9", "ET7", "ES7", or "ET5".

    Returns:
        ValidationReport with all findings.
    """
    report = ValidationReport()
    _log(f"=== Validating vehicle: {vehicle_name} ===")

    # 1. Directory structure
    content_dir = str(_PROJECT_ROOT / "Content")
    report.issues.extend(_validate_directory_structure(content_dir, vehicle_name))

    if UE5_MODE:
        report.merge(_validate_vehicle_ue5(vehicle_name))
    else:
        report.merge(_validate_vehicle_standalone(vehicle_name))

    _log(f"=== Vehicle {vehicle_name}: {len(report.errors)} errors, {len(report.warnings)} warnings ===")
    return report


def _validate_vehicle_ue5(vehicle_name: str) -> ValidationReport:
    """UE5 Editor mode vehicle validation."""
    report = ValidationReport()
    base = f"/Game/Vehicles/{vehicle_name}"

    # --- Meshes ---
    mesh_dir = f"{base}/Meshes"
    if unreal.EditorAssetLibrary.does_directory_exist(mesh_dir):
        assets = unreal.EditorAssetLibrary.list_assets(mesh_dir, recursive=True, include_folder=False)
        lod_counts: dict[str, int] = {}

        for asset_path in assets:
            name = Path(asset_path).stem
            report.checked_assets += 1

            # Check naming
            report.issues.extend(_validate_naming(name, "static_mesh"))

            # Get triangle count
            tri_count = _get_mesh_triangle_count(asset_path)
            if tri_count is not None:
                # Detect LOD level from name
                lod_level = ""
                upper = name.upper()
                for lod in ("LOD0", "LOD1", "LOD2"):
                    if lod in upper:
                        lod_level = lod
                        break

                if lod_level:
                    lod_counts[lod_level] = tri_count
                    report.issues.extend(_validate_polygon_count(
                        asset_path, tri_count, "vehicle", vehicle_name, lod_level
                    ))
                else:
                    # Assume LOD0 if no suffix
                    report.issues.extend(_validate_polygon_count(
                        asset_path, tri_count, "vehicle", vehicle_name, "LOD0"
                    ))

            report.passed_assets += 1

        # Validate LOD chain
        if lod_counts:
            report.issues.extend(_validate_lod_chain(mesh_dir, lod_counts, vehicle_name))
    else:
        report.add(ValidationIssue(
            severity="info", category="structure",
            asset_path=mesh_dir,
            message=f"Mesh directory not found: {mesh_dir}"
        ))

    # --- Textures ---
    tex_dir = f"{base}/Textures"
    texture_paths: list[str] = []
    if unreal.EditorAssetLibrary.does_directory_exist(tex_dir):
        assets = unreal.EditorAssetLibrary.list_assets(tex_dir, recursive=True, include_folder=False)

        for asset_path in assets:
            name = Path(asset_path).stem
            report.checked_assets += 1
            texture_paths.append(asset_path)

            report.issues.extend(_validate_naming(name, "texture"))
            report.issues.extend(_validate_texture_suffix(name, asset_path))

            info = _get_texture_info_ue5(asset_path)
            if info:
                report.issues.extend(_validate_texture(asset_path, info, "vehicle"))

            report.passed_assets += 1

        # Check required maps
        report.issues.extend(_validate_required_texture_maps(texture_paths, vehicle_name))
    else:
        report.add(ValidationIssue(
            severity="info", category="structure",
            asset_path=tex_dir,
            message=f"Texture directory not found: {tex_dir}"
        ))

    # --- Materials ---
    mat_dir = f"/Game/Materials/Vehicles/{vehicle_name}"
    if unreal.EditorAssetLibrary.does_directory_exist(mat_dir):
        assets = unreal.EditorAssetLibrary.list_assets(mat_dir, recursive=True, include_folder=False)

        for asset_path in assets:
            name = Path(asset_path).stem
            report.checked_assets += 1

            report.issues.extend(_validate_material_naming(asset_path))
            report.issues.extend(_validate_material_compatibility_ue5(asset_path))

            report.passed_assets += 1

    return report


def _validate_vehicle_standalone(vehicle_name: str) -> ValidationReport:
    """Standalone filesystem mode vehicle validation."""
    report = ValidationReport()
    content_dir = _PROJECT_ROOT / "Content"
    vehicle_dir = content_dir / "Vehicles" / vehicle_name

    if not vehicle_dir.exists():
        report.add(ValidationIssue(
            severity="info", category="structure",
            asset_path=str(vehicle_dir),
            message=f"Vehicle directory not found: {vehicle_dir}"
        ))
        return report

    # --- Meshes (FBX files) ---
    mesh_dir = vehicle_dir / "Meshes"
    if mesh_dir.exists():
        fbx_files = sorted(mesh_dir.glob("*.fbx"))
        lod_counts: dict[str, int] = {}

        for fbx in fbx_files:
            name = fbx.stem
            report.checked_assets += 1

            report.issues.extend(_validate_naming(name, "static_mesh"))

            # Detect LOD
            lod_level = ""
            upper = name.upper()
            for lod in ("LOD0", "LOD1", "LOD2"):
                if lod in upper:
                    lod_level = lod
                    break

            tri_count = _count_fbx_triangles_simple(str(fbx))
            if tri_count is not None and lod_level:
                lod_counts[lod_level] = tri_count
                report.issues.extend(_validate_polygon_count(
                    str(fbx), tri_count, "vehicle", vehicle_name, lod_level
                ))

            report.passed_assets += 1

        if lod_counts:
            report.issues.extend(_validate_lod_chain(str(mesh_dir), lod_counts, vehicle_name))
    else:
        # Also check for FBX files directly in the vehicle directory
        fbx_files = sorted(vehicle_dir.glob("*.fbx"))
        if fbx_files:
            lod_counts: dict[str, int] = {}
            for fbx in fbx_files:
                name = fbx.stem
                report.checked_assets += 1
                lod_level = ""
                upper = name.upper()
                for lod in ("LOD0", "LOD1", "LOD2"):
                    if lod in upper:
                        lod_level = lod
                        break
                tri_count = _count_fbx_triangles_simple(str(fbx))
                if tri_count is not None and lod_level:
                    lod_counts[lod_level] = tri_count
                    report.issues.extend(_validate_polygon_count(
                        str(fbx), tri_count, "vehicle", vehicle_name, lod_level
                    ))
                report.passed_assets += 1
            if lod_counts:
                report.issues.extend(_validate_lod_chain(str(vehicle_dir), lod_counts, vehicle_name))

    # --- Textures ---
    tex_dir = vehicle_dir / "Textures"
    texture_paths: list[str] = []
    if tex_dir.exists():
        tex_exts = {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".exr", ".hdr", ".hdri", ".tif", ".tiff"}
        tex_files = sorted(f for f in tex_dir.iterdir() if f.suffix.lower() in tex_exts and f.is_file())

        for tex in tex_files:
            name = tex.stem
            report.checked_assets += 1
            texture_paths.append(str(tex))

            report.issues.extend(_validate_naming(name, "texture"))
            report.issues.extend(_validate_texture_suffix(name, str(tex)))

            info = _get_texture_info_standalone(str(tex))
            if info:
                # Add file size
                try:
                    info["file_size_mb"] = tex.stat().st_size / (1024 * 1024)
                except OSError:
                    pass
                report.issues.extend(_validate_texture(str(tex), info, "vehicle"))

            report.passed_assets += 1

        report.issues.extend(_validate_required_texture_maps(
            [str(t) for t in tex_files], vehicle_name
        ))

    # --- Materials (check JSON configs for material references) ---
    # In standalone mode, check for material config files
    for json_file in vehicle_dir.rglob("*.json"):
        report.checked_assets += 1
        report.issues.extend(_validate_naming(json_file.stem))
        report.passed_assets += 1

    return report


def validate_content_directory(content_path: str) -> ValidationReport:
    """Validate all assets in a content directory.

    Args:
        content_path: Path to the Content directory (filesystem or UE5 /Game path).

    Returns:
        ValidationReport with all findings.
    """
    report = ValidationReport()
    _log(f"=== Validating content directory: {content_path} ===")

    # Check for known vehicle directories
    if UE5_MODE:
        for vehicle in ("EP9", "ET7", "ES7", "ET5"):
            veh_path = f"/Game/Vehicles/{vehicle}"
            if unreal.EditorAssetLibrary.does_directory_exist(veh_path):
                report.merge(validate_vehicle(vehicle))

        # Validate environment meshes
        env_path = "/Game/Maps"
        if unreal.EditorAssetLibrary.does_directory_exist(env_path):
            report.merge(_validate_environment_assets_ue5(env_path))

        # Check master materials
        report.issues.extend(_check_required_master_materials_exist(content_path))
    else:
        content_dir = Path(content_path)
        vehicles_dir = content_dir / "Vehicles"
        if vehicles_dir.exists():
            for veh_dir in vehicles_dir.iterdir():
                if veh_dir.is_dir() and veh_dir.name in ("EP9", "ET7", "ES7", "ET5"):
                    report.merge(validate_vehicle(veh_dir.name))

        # Check master materials
        report.issues.extend(_check_required_master_materials_exist(content_path))

    _log(f"=== Content validation complete ===")
    return report


def _validate_environment_assets_ue5(env_path: str) -> ValidationReport:
    """Validate environment meshes in UE5 mode."""
    report = ValidationReport()

    assets = unreal.EditorAssetLibrary.list_assets(env_path, recursive=True, include_folder=False)
    for asset_path in assets:
        name = Path(asset_path).stem
        ext = Path(asset_path).suffix.lower()

        if ext in (".uasset",):
            report.checked_assets += 1

            tri_count = _get_mesh_triangle_count(asset_path)
            if tri_count is not None:
                report.issues.extend(_validate_polygon_count(
                    asset_path, tri_count, "environment"
                ))

            report.passed_assets += 1

    return report


def validate_all_assets() -> ValidationReport:
    """Run a full validation of the entire project.

    Returns:
        ValidationReport with all findings.
    """
    report = ValidationReport()
    _log("=" * 70)
    _log("  NIO Racing Plus - Full Asset Validation")
    _log("=" * 70)

    content_dir = str(_PROJECT_ROOT / "Content")

    # 1. Validate all vehicles
    for vehicle in ("EP9", "ET7", "ES7"):
        report.merge(validate_vehicle(vehicle))

    # 2. Validate content directory (environment, shared materials)
    report.merge(validate_content_directory(content_dir))

    return report


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main():
    """Command-line entry point for standalone validation."""
    global _PROJECT_ROOT
    import argparse

    parser = argparse.ArgumentParser(
        description="NIO Racing Plus Asset Validator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python asset_validator.py --project-root /path/to/NomiRacingPlus
  python asset_validator.py --vehicle EP9 --project-root /path/to/NomiRacingPlus
  python asset_validator.py --content-dir /path/to/Content
        """
    )
    parser.add_argument("--project-root", type=str, default=str(_PROJECT_ROOT),
                        help="Path to the NomiRacingPlus project root")
    parser.add_argument("--vehicle", type=str, default=None,
                        help="Validate a specific vehicle (EP9, ET7, ES7, ET5)")
    parser.add_argument("--content-dir", type=str, default=None,
                        help="Validate a specific content directory")
    parser.add_argument("--json", action="store_true",
                        help="Output report as JSON")

    args = parser.parse_args()

    # Update project root if specified
    _PROJECT_ROOT = Path(args.project_root)

    if args.vehicle:
        report = validate_vehicle(args.vehicle)
    elif args.content_dir:
        report = validate_content_directory(args.content_dir)
    else:
        report = validate_all_assets()

    if args.json:
        output = {
            "passed": report.passed,
            "checked": report.checked_assets,
            "passed_assets": report.passed_assets,
            "errors": len(report.errors),
            "warnings": len(report.warnings),
            "infos": len(report.infos),
            "issues": [
                {
                    "severity": i.severity,
                    "category": i.category,
                    "asset": i.asset_path,
                    "message": i.message,
                    "details": i.details,
                }
                for i in report.issues
            ]
        }
        print(json.dumps(output, indent=2, ensure_ascii=False))
    else:
        print(report.summary())

    sys.exit(0 if report.passed else 1)


if __name__ == "__main__":
    main()
