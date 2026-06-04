"""
NIO Racing Plus - Material Setup Script
=========================================
UE5 Editor utility to create PBR materials for vehicles, environment, and UI.

Creates:
- Vehicle paint materials (clearcoat metallic paint)
- Vehicle glass (transparent, tinted)
- Wheel materials (chrome/matte)
- Headlight/taillight emissive materials
- Road/asphalt PBR materials
- NIO-themed UI materials

Usage (UE5 Editor Output Log):
    exec(open(r"<PROJECT>/Scripts/Editor/material_setup.py").read())
    create_all_vehicle_materials("EP9")

Usage (Command line):
    UnrealEditor-Cmd <PROJECT>.uproject -run=pythonscript -script="<PROJECT>/Scripts/Editor/material_setup.py" -- -vehicle EP9
"""

import unreal
import math


# ---------------------------------------------------------------------------
# NIO Brand Colors
# ---------------------------------------------------------------------------

NIO_COLORS = {
    "nio_blue":     (0.000, 0.631, 0.878, 1.0),   # #00A1E0
    "nio_cyan":     (0.000, 0.831, 1.000, 1.0),   # #00D4FF
    "deep_bg":      (0.039, 0.055, 0.102, 1.0),   # #0A0E1A
    "success_green":(0.498, 1.000, 0.000, 1.0),   # #7FFF00
    "danger_red":   (1.000, 0.133, 0.267, 1.0),   # #FF2244
    "text_white":   (0.941, 0.941, 0.941, 1.0),   # #F0F0F0
    "text_gray":    (0.533, 0.600, 0.667, 1.0),   # #8899AA
}

# Vehicle paint presets (NIO vehicle colors)
VEHICLE_PAINT_COLORS = {
    "EP9": {
        "base_color": (0.02, 0.02, 0.02, 1.0),     # Dark black
        "metallic": 0.8,
        "roughness": 0.15,
        "clearcoat": 1.0,
        "clearcoat_roughness": 0.05,
    },
    "ET7": {
        "base_color": (0.6, 0.62, 0.65, 1.0),      # Moonstone silver
        "metallic": 0.9,
        "roughness": 0.1,
        "clearcoat": 1.0,
        "clearcoat_roughness": 0.03,
    },
    "ES7": {
        "base_color": (0.05, 0.15, 0.35, 1.0),     # Deep blue
        "metallic": 0.7,
        "roughness": 0.2,
        "clearcoat": 1.0,
        "clearcoat_roughness": 0.05,
    },
    "ET5": {
        "base_color": (0.8, 0.75, 0.65, 1.0),      # Sand gold
        "metallic": 0.85,
        "roughness": 0.12,
        "clearcoat": 1.0,
        "clearcoat_roughness": 0.04,
    },
}

# Additional paint colors for customization (from PLAN.md: 5-8 preset colors)
CUSTOM_PAINT_COLORS = {
    "Arctic White":  (0.9, 0.9, 0.9, 1.0),
    "NIO Blue":      NIO_COLORS["nio_blue"],
    "Obsidian Black":(0.02, 0.02, 0.02, 1.0),
    "Moonstone Silver": (0.6, 0.62, 0.65, 1.0),
    "Lava Red":      (0.6, 0.05, 0.05, 1.0),
    "Forest Green":  (0.05, 0.3, 0.15, 1.0),
    "Deep Purple":   (0.2, 0.05, 0.3, 1.0),
    "Sunset Orange": (0.9, 0.45, 0.1, 1.0),
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _log(msg: str, level: str = "info"):
    dispatch = {
        "info": unreal.log,
        "warn": unreal.log_warning,
        "error": unreal.log_error,
    }
    dispatch.get(level, unreal.log)(f"[MaterialSetup] {msg}")


def _create_material(name: str, destination_path: str) -> unreal.Material | None:
    """Create a new material asset. Returns the material or None on failure."""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # Check if it already exists
    full_path = f"{destination_path}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        _log(f"  Material already exists, loading: {full_path}")
        mat = unreal.EditorAssetLibrary.load_asset(full_path)
        if isinstance(mat, unreal.Material):
            return mat

    mat = asset_tools.create_asset(name, destination_path, unreal.Material, unreal.MaterialFactoryNew())
    if mat:
        _log(f"  Created material: {full_path}")
    else:
        _log(f"  Failed to create material: {full_path}", "error")
    return mat


def _create_material_instance(parent: unreal.Material, name: str,
                               destination_path: str) -> unreal.MaterialInstanceConstant | None:
    """Create a material instance from a parent material."""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    full_path = f"{destination_path}/{name}"

    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        _log(f"  Material instance already exists, loading: {full_path}")
        mi = unreal.EditorAssetLibrary.load_asset(full_path)
        if isinstance(mi, unreal.MaterialInstanceConstant):
            return mi

    mi = asset_tools.create_asset(name, destination_path,
                                   unreal.MaterialInstanceConstant,
                                   unreal.MaterialInstanceConstantFactoryNew())
    if mi:
        # Set parent material
        unreal.MaterialEditingLibrary.set_material_instance_parent(mi, parent)
        _log(f"  Created material instance: {full_path}")
    else:
        _log(f"  Failed to create material instance: {full_path}", "error")
    return mi


def _set_scalar_param(mi: unreal.MaterialInstanceConstant, name: str, value: float):
    """Set a scalar parameter on a material instance."""
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, name, value)


def _set_vector_param(mi: unreal.MaterialInstanceConstant, name: str, color: tuple):
    """Set a vector (color) parameter on a material instance."""
    linear = unreal.LinearColor(color[0], color[1], color[2], color[3])
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mi, name, linear)


def _set_texture_param(mi: unreal.MaterialInstanceConstant, name: str, texture_path: str):
    """Set a texture parameter on a material instance."""
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if texture:
        unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi, name, texture)


# ---------------------------------------------------------------------------
# Material Parameter Collections
# ---------------------------------------------------------------------------

def create_vehicle_paint_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create the master vehicle paint material with clearcoat.

    This material uses the UE5 clearcoat shading model for realistic
    metallic car paint with a clear lacquer layer.

    Parameters exposed:
        BaseColor       - Paint color (vector)
        Metallic        - Metallic value (scalar, 0-1)
        Roughness       - Base roughness (scalar, 0-1)
        Clearcoat       - Clearcoat intensity (scalar, 0-1)
        ClearcoatRoughness - Clearcoat roughness (scalar, 0-1)
        Normal          - Normal map (texture, optional)
        FlipbookNormal  - Sparkle/flipbook normal (texture, optional)
    """
    _log("Creating master vehicle paint material...")
    mat = _create_material("M_Vehicle_Paint_Master", destination_path)
    if not mat:
        return None

    # Set shading model to Clear Coat
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_CLEAR_COAT)
    mat.set_editor_property("two_sided", False)

    # Note: Material expression nodes must be added via the editor UI or
    # MaterialEditingLibrary. The actual node graph is configured below.

    # For a production setup, you would create expression nodes here.
    # This script creates the material shell; the node graph should be
    # configured in the editor or via a more advanced script.

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Paint_Master")
    _log("  Vehicle paint master material created")
    return mat


def create_vehicle_glass_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create vehicle glass material (transparent, tinted).

    Parameters:
        GlassColor      - Tint color (vector)
        GlassOpacity    - Opacity (scalar, 0-1)
        OpacityMaskClip - Clip threshold (scalar)
        Roughness       - Glass roughness (scalar)
        IOR             - Index of refraction (scalar, default 1.52)
    """
    _log("Creating vehicle glass material...")
    mat = _create_material("M_Vehicle_Glass", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)
    mat.set_editor_property("translucency_lighting_mode",
                             unreal.TranslucencyLightingMode.TLM_SURFACE)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Glass")
    _log("  Vehicle glass material created")
    return mat


def create_vehicle_wheel_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create vehicle wheel/rim material (chrome or matte metal).

    Parameters:
        BaseColor       - Rim color (vector)
        Metallic        - Metallic value (scalar, default 1.0)
        Roughness       - Surface roughness (scalar)
        Normal          - Normal map (texture)
    """
    _log("Creating vehicle wheel material...")
    mat = _create_material("M_Vehicle_Wheel", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Wheel")
    _log("  Vehicle wheel material created")
    return mat


def create_vehicle_tire_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create vehicle tire material (rubber, matte black).

    Parameters:
        BaseColor       - Tire color (vector, default black)
        Roughness       - Rubber roughness (scalar, default 0.8)
        Normal          - Tread pattern normal map (texture)
    """
    _log("Creating vehicle tire material...")
    mat = _create_material("M_Vehicle_Tire", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Tire")
    _log("  Vehicle tire material created")
    return mat


def create_headlight_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create headlight emissive material.

    Parameters:
        EmissiveColor   - Light color (vector, default warm white)
        EmissiveIntensity - Brightness (scalar, default 10.0)
        EmissiveMask    - Emissive area mask (texture)
    """
    _log("Creating headlight material...")
    mat = _create_material("M_Vehicle_Headlight", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Headlight")
    _log("  Headlight material created")
    return mat


def create_taillight_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create taillight emissive material.

    Parameters:
        EmissiveColor   - Light color (vector, default red)
        EmissiveIntensity - Brightness (scalar)
        BrakeIntensity  - Brake light brightness (scalar)
        EmissiveMask    - Emissive area mask (texture)
    """
    _log("Creating taillight material...")
    mat = _create_material("M_Vehicle_Taillight", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_Taillight")
    _log("  Taillight material created")
    return mat


def create_brake_light_material(destination_path: str = "/Game/Materials/Vehicles") -> unreal.Material:
    """Create brake light emissive material (brighter red when braking).

    Parameters:
        EmissiveColor   - Base color (vector)
        EmissiveIntensity - Idle brightness (scalar)
        BrakeBrightness - Brake-applied brightness multiplier (scalar)
    """
    _log("Creating brake light material...")
    mat = _create_material("M_Vehicle_BrakeLight", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Vehicle_BrakeLight")
    _log("  Brake light material created")
    return mat


# ---------------------------------------------------------------------------
# Environment Materials
# ---------------------------------------------------------------------------

def create_road_material(destination_path: str = "/Game/Materials/Environment") -> unreal.Material:
    """Create PBR road/asphalt material.

    Parameters:
        BaseColor       - Road color (vector)
        Roughness       - Surface roughness (scalar)
        Normal          - Normal map (texture)
        AO              - Ambient occlusion (texture)
        DetailNormal    - Detail normal for close-up (texture)
        DetailScale     - Detail normal tiling (scalar)
    """
    _log("Creating road material...")
    mat = _create_material("M_Road_Asphalt", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Road_Asphalt")
    _log("  Road material created")
    return mat


def create_concrete_material(destination_path: str = "/Game/Materials/Environment") -> unreal.Material:
    """Create PBR concrete material for sidewalks and buildings."""
    _log("Creating concrete material...")
    mat = _create_material("M_Concrete", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Concrete")
    _log("  Concrete material created")
    return mat


def create_metal_material(destination_path: str = "/Game/Materials/Environment") -> unreal.Material:
    """Create PBR metal material for guardrails and barriers."""
    _log("Creating metal material...")
    mat = _create_material("M_Metal", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_Metal")
    _log("  Metal material created")
    return mat


# ---------------------------------------------------------------------------
# NIO UI Materials
# ---------------------------------------------------------------------------

def create_nio_ui_material(destination_path: str = "/Game/Materials/UI") -> unreal.Material:
    """Create NIO-themed UI panel material with glow effect.

    Parameters:
        PanelColor      - Panel background color (vector, default NIO Blue)
        GlowIntensity   - Glow brightness (scalar)
        Opacity         - Panel opacity (scalar)
    """
    _log("Creating NIO UI panel material...")
    mat = _create_material("M_UI_NIO_Panel", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_UI_NIO_Panel")
    _log("  NIO UI panel material created")
    return mat


def create_nio_emissive_border(destination_path: str = "/Game/Materials/UI") -> unreal.Material:
    """Create NIO-themed emissive border material for UI elements."""
    _log("Creating NIO emissive border material...")
    mat = _create_material("M_UI_NIO_Border", destination_path)
    if not mat:
        return None

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    unreal.EditorAssetLibrary.save_asset(f"{destination_path}/M_UI_NIO_Border")
    _log("  NIO emissive border material created")
    return mat


# ---------------------------------------------------------------------------
# Material Instance Presets
# ---------------------------------------------------------------------------

def create_paint_preset_instances(vehicle_name: str,
                                   destination_path: str | None = None) -> list:
    """Create material instance presets for a vehicle's paint colors.

    Args:
        vehicle_name: "EP9", "ET7", "ES7", or "ET5".
        destination_path: Where to save instances.

    Returns:
        List of created MaterialInstanceConstant assets.
    """
    dest = destination_path or f"/Game/Materials/Vehicles/{vehicle_name}"
    parent_path = "/Game/Materials/Vehicles/M_Vehicle_Paint_Master"

    parent = unreal.EditorAssetLibrary.load_asset(parent_path)
    if not isinstance(parent, unreal.Material):
        _log(f"Master paint material not found: {parent_path}", "error")
        return []

    _log(f"Creating paint presets for {vehicle_name}...")
    instances = []

    # Default vehicle paint
    defaults = VEHICLE_PAINT_COLORS.get(vehicle_name, VEHICLE_PAINT_COLORS["EP9"])
    default_mi = _create_material_instance(parent, f"MI_{vehicle_name}_Default", dest)
    if default_mi:
        _set_vector_param(default_mi, "BaseColor", defaults["base_color"])
        _set_scalar_param(default_mi, "Metallic", defaults["metallic"])
        _set_scalar_param(default_mi, "Roughness", defaults["roughness"])
        _set_scalar_param(default_mi, "Clearcoat", defaults["clearcoat"])
        _set_scalar_param(default_mi, "ClearcoatRoughness", defaults["clearcoat_roughness"])
        unreal.EditorAssetLibrary.save_asset(f"{dest}/MI_{vehicle_name}_Default")
        instances.append(default_mi)

    # Custom color presets
    for color_name, color_value in CUSTOM_PAINT_COLORS.items():
        safe_name = color_name.replace(" ", "_")
        mi = _create_material_instance(parent, f"MI_{vehicle_name}_{safe_name}", dest)
        if mi:
            _set_vector_param(mi, "BaseColor", color_value)
            _set_scalar_param(mi, "Metallic", 0.85)
            _set_scalar_param(mi, "Roughness", 0.12)
            _set_scalar_param(mi, "Clearcoat", 1.0)
            _set_scalar_param(mi, "ClearcoatRoughness", 0.04)
            unreal.EditorAssetLibrary.save_asset(f"{dest}/MI_{vehicle_name}_{safe_name}")
            instances.append(mi)

    _log(f"  Created {len(instances)} paint presets for {vehicle_name}")
    return instances


def create_glass_preset_instances(vehicle_name: str,
                                   destination_path: str | None = None) -> list:
    """Create glass material instances for a vehicle.

    Creates: windshield, side windows, rear window.
    """
    dest = destination_path or f"/Game/Materials/Vehicles/{vehicle_name}"
    parent_path = "/Game/Materials/Vehicles/M_Vehicle_Glass"

    parent = unreal.EditorAssetLibrary.load_asset(parent_path)
    if not isinstance(parent, unreal.Material):
        _log(f"Glass material not found: {parent_path}", "error")
        return []

    _log(f"Creating glass presets for {vehicle_name}...")
    instances = []

    glass_presets = {
        "Windshield": {"color": (0.05, 0.05, 0.05, 0.6), "roughness": 0.02, "ior": 1.52},
        "SideWindow": {"color": (0.03, 0.03, 0.03, 0.7), "roughness": 0.01, "ior": 1.52},
        "RearWindow": {"color": (0.05, 0.05, 0.05, 0.5), "roughness": 0.02, "ior": 1.52},
    }

    for name, props in glass_presets.items():
        mi = _create_material_instance(parent, f"MI_{vehicle_name}_Glass_{name}", dest)
        if mi:
            _set_vector_param(mi, "GlassColor", props["color"])
            _set_scalar_param(mi, "GlassOpacity", props["color"][3])
            _set_scalar_param(mi, "Roughness", props["roughness"])
            _set_scalar_param(mi, "IOR", props["ior"])
            unreal.EditorAssetLibrary.save_asset(f"{dest}/MI_{vehicle_name}_Glass_{name}")
            instances.append(mi)

    _log(f"  Created {len(instances)} glass presets for {vehicle_name}")
    return instances


# ---------------------------------------------------------------------------
# Orchestrators
# ---------------------------------------------------------------------------

def create_all_master_materials():
    """Create all master materials for the project.

    Run this once during project setup. It creates the base materials
    that material instances will reference.
    """
    _log("=== Creating All Master Materials ===")

    # Vehicle materials
    create_vehicle_paint_material()
    create_vehicle_glass_material()
    create_vehicle_wheel_material()
    create_vehicle_tire_material()
    create_headlight_material()
    create_taillight_material()
    create_brake_light_material()

    # Environment materials
    create_road_material()
    create_concrete_material()
    create_metal_material()

    # UI materials
    create_nio_ui_material()
    create_nio_emissive_border()

    _log("=== All Master Materials Created ===")


def create_all_vehicle_materials(vehicle_name: str):
    """Create all materials and presets for a specific vehicle.

    Args:
        vehicle_name: "EP9", "ET7", "ES7", or "ET5".
    """
    _log(f"=== Creating Materials for {vehicle_name} ===")

    # Ensure master materials exist
    create_all_master_materials()

    # Create vehicle-specific instances
    create_paint_preset_instances(vehicle_name)
    create_glass_preset_instances(vehicle_name)

    _log(f"=== {vehicle_name} Materials Complete ===")


def create_all_vehicles_materials():
    """Create materials for all NIO vehicles."""
    for vehicle in ("EP9", "ET7", "ES7", "ET5"):
        create_all_vehicle_materials(vehicle)


# ---------------------------------------------------------------------------
# Texture Assignment Helper
# ---------------------------------------------------------------------------

def assign_textures_to_material(material_path: str, texture_paths: dict):
    """Assign texture assets to a material instance's parameters.

    Args:
        material_path: UE5 path to the material instance.
        texture_paths: Dict mapping parameter name to texture asset path.
            e.g. {"Normal": "/Game/Vehicles/EP9/Textures/T_EP9_N",
                   "BaseColor": "/Game/Vehicles/EP9/Textures/T_EP9_D"}

    Example:
        assign_textures_to_material(
            "/Game/Materials/Vehicles/EP9/MI_EP9_Default",
            {
                "Normal": "/Game/Vehicles/EP9/Textures/T_EP9_Body_N",
                "Metallic": "/Game/Vehicles/EP9/Textures/T_EP9_Body_M",
            }
        )
    """
    mi = unreal.EditorAssetLibrary.load_asset(material_path)
    if not isinstance(mi, unreal.MaterialInstanceConstant):
        _log(f"Not a material instance: {material_path}", "error")
        return

    _log(f"Assigning textures to: {material_path}")
    for param_name, tex_path in texture_paths.items():
        _set_texture_param(mi, param_name, tex_path)
        _log(f"  {param_name} -> {tex_path}")

    unreal.EditorAssetLibrary.save_asset(material_path)


def auto_assign_vehicle_textures(vehicle_name: str):
    """Auto-detect and assign textures for a vehicle based on naming conventions.

    Looks for textures in /Game/Vehicles/{vehicle_name}/Textures/ with naming:
        T_{vehicle}_Body_D.*    -> BaseColor
        T_{vehicle}_Body_N.*    -> Normal
        T_{vehicle}_Body_M.*    -> Metallic
        T_{vehicle}_Glass_D.*   -> Glass BaseColor
    """
    tex_dir = f"/Game/Vehicles/{vehicle_name}/Textures"
    mi_path = f"/Game/Materials/Vehicles/{vehicle_name}/MI_{vehicle_name}_Default"

    if not unreal.EditorAssetLibrary.does_directory_exist(tex_dir):
        _log(f"Texture directory not found: {tex_dir}", "warn")
        return

    _log(f"Auto-assigning textures for {vehicle_name}...")

    assets = unreal.EditorAssetLibrary.list_assets(tex_dir, recursive=False, include_folder=False)
    texture_map = {}

    for asset_path in assets:
        name = asset_path.split("/")[-1]
        upper = name.upper()
        vehicle_upper = vehicle_name.upper()

        if f"{vehicle_upper}_BODY_D" in upper or "BODY_D" in upper:
            texture_map["BaseColor"] = asset_path
        elif f"{vehicle_upper}_BODY_N" in upper or "BODY_N" in upper:
            texture_map["Normal"] = asset_path
        elif f"{vehicle_upper}_BODY_M" in upper or "BODY_M" in upper:
            texture_map["Metallic"] = asset_path
        elif f"{vehicle_upper}_BODY_R" in upper or "BODY_R" in upper:
            texture_map["Roughness"] = asset_path
        elif "GLASS_D" in upper:
            glass_mi = f"/Game/Materials/Vehicles/{vehicle_name}/MI_{vehicle_name}_Glass_Windshield"
            if unreal.EditorAssetLibrary.does_asset_exist(glass_mi):
                assign_textures_to_material(glass_mi, {"BaseColor": asset_path})

    if texture_map:
        if unreal.EditorAssetLibrary.does_asset_exist(mi_path):
            assign_textures_to_material(mi_path, texture_map)
        else:
            _log(f"Material instance not found: {mi_path}", "warn")
    else:
        _log(f"No matching textures found in {tex_dir}", "warn")


# ---------------------------------------------------------------------------
# Standalone execution
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Setup UE5 materials")
    parser.add_argument("-vehicle", help="Vehicle name (EP9, ET7, ES7, ET5)")
    parser.add_argument("-all", action="store_true", help="Create materials for all vehicles")
    parser.add_argument("-masters", action="store_true", help="Create master materials only")
    parser.add_argument("-assign", action="store_true", help="Auto-assign textures")
    args = parser.parse_args()

    if args.masters:
        create_all_master_materials()
    elif args.all:
        create_all_vehicles_materials()
    elif args.vehicle:
        if args.assign:
            auto_assign_vehicle_textures(args.vehicle)
        else:
            create_all_vehicle_materials(args.vehicle)
    else:
        parser.print_help()
