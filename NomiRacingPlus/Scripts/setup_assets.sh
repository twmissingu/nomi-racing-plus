#!/bin/bash
# NIO Racing Plus - Asset Setup Script
# Downloads and processes assets from various sources

set -e  # Exit on error

# Configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSETS_DIR="${PROJECT_DIR}/Assets"
CONTENT_DIR="${PROJECT_DIR}/Content"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Create asset directories
create_directories() {
    log_step "Creating asset directories..."

    mkdir -p "${ASSETS_DIR}/NIO/EP9"
    mkdir -p "${ASSETS_DIR}/NIO/ET7"
    mkdir -p "${ASSETS_DIR}/NIO/ES7"
    mkdir -p "${ASSETS_DIR}/CARLA/Towns"
    mkdir -p "${ASSETS_DIR}/CARLA/Vehicles"
    mkdir -p "${ASSETS_DIR}/Textures/PBR"
    mkdir -p "${ASSETS_DIR}/Audio/SFX"
    mkdir -p "${ASSETS_DIR}/Audio/Music"
    mkdir -p "${ASSETS_DIR}/HDR"

    log_info "Asset directories created"
}

# Download NIO vehicle models from Sketchfab
download_nio_models() {
    log_step "Downloading NIO vehicle models..."

    # NIO EP9
    log_info "Downloading NIO EP9..."
    # Note: Requires Sketchfab account and API key
    # Using SketchfabDownloader: https://github.com/SaltyFishOTL/SketchfabDownloader

    cat > "${ASSETS_DIR}/NIO/download_info.txt" << 'EOF'
NIO Vehicle Models - Download Instructions

1. NIO EP9 (1,400,252 faces)
   URL: https://sketchfab.com/3d-models/nio-ep9-2017-b9bfaa1ea4824bef85ea755f8c10c6d2
   License: CC BY 4.0
   Format: glTF Binary (.glb)

2. NIO ET7 (1,043,414 faces)
   URL: https://sketchfab.com/3d-models/nio-et7-2021-b428077c63a743c6bf82059e2ec3b4fb
   License: CC BY 4.0
   Format: glTF Binary (.glb)

3. NIO ES7 (588,958 faces)
   URL: https://sketchfab.com/3d-models/nio-es7-2023-4d9c574b84514b21ac783aca550793fe
   License: CC BY 4.0
   Format: glTF Binary (.glb)

Download Instructions:
1. Visit each URL
2. Click "Download 3D Model"
3. Select "glTF Binary (.glb)" format
4. Place files in respective directories

Alternative: Use SketchfabDownloader
pip install sketchfab-downloader
sketchfab-downloader -i MODEL_ID -o OUTPUT_DIR
EOF

    log_info "Download info created at: ${ASSETS_DIR}/NIO/download_info.txt"
    log_warn "Manual download required - see download_info.txt for instructions"
}

# Download free assets from ambientCG
download_ambientcg_assets() {
    log_step "Downloading PBR textures from ambientCG..."

    # Create download list
    cat > "${ASSETS_DIR}/Textures/download_list.txt" << 'EOF'
ambientCG PBR Textures - Download List

Road Textures:
- Asphalt 01: https://ambientcg.com/view?id=Asphalt01
- Asphalt 02: https://ambientcg.com/view?id=Asphalt02
- Concrete 01: https://ambientcg.com/view?id=Concrete01

Metal Textures:
- Metal 01: https://ambientcg.com/view?id=Metal01
- Metal 02: https://ambientcg.com/view?id=Metal02

Ground Textures:
- Ground 01: https://ambientcg.com/view?id=Ground01
- Sand 01: https://ambientcg.com/view?id=Sand01

License: CC0 (Public Domain)
Format: PNG (2K resolution recommended)
EOF

    log_info "Texture download list created"
}

# Download HDR skyboxes from Poly Haven
download_hdr_skyboxes() {
    log_step "Downloading HDR skyboxes from Poly Haven..."

    cat > "${ASSETS_DIR}/HDR/download_list.txt" << 'EOF'
Poly Haven HDR Skyboxes - Download List

Day Skies:
- Clear Sky: https://polyhaven.com/hdris/clear sky
- Partly Cloudy: https://polyhaven.com/hdris/partly cloudy

Sunset Skies:
- Sunset 01: https://polyhaven.com/hdris/sunset 01
- Golden Hour: https://polyhaven.com/hdris/golden hour

Night Skies:
- Night 01: https://polyhaven.com/hdris/night 01

License: CC0 (Public Domain)
Format: HDR (4K resolution recommended)
EOF

    log_info "HDR download list created"
}

# Download CARLA assets
download_carla_assets() {
    log_step "Preparing CARLA asset extraction..."

    cat > "${ASSETS_DIR}/CARLA/extraction_guide.txt" << 'EOF'
CARLA Asset Extraction Guide

CARLA Repository: https://github.com/carla-simulator/carla
Branch: ue5-dev (UE 5.5 compatible)

Extraction Steps:
1. Clone CARLA repository:
   git clone --branch ue5-dev https://github.com/carla-simulator/carla.git

2. Build CARLA or download pre-built package:
   - Pre-built: https://github.com/carla-simulator/carla/releases

3. Extract assets:
   Town Maps: carla/Unreal/CarlaUnreal/Content/Carla/Maps/
   Vehicle Models: carla/Unreal/CarlaUnreal/Content/Carla/Vehicles/

4. Copy to this project:
   cp -r carla/Unreal/CarlaUnreal/Content/Carla/Maps/* ${ASSETS_DIR}/CARLA/Towns/
   cp -r carla/Unreal/CarlaUnreal/Content/Carla/Vehicles/* ${ASSETS_DIR}/CARLA/Vehicles/

License Verification:
- Code: MIT License
- Assets: Verify individual asset licenses in CARLA documentation
- Non-commercial use: Should be compatible with MIT license

Target Towns:
- Town03: Best for city circuit
- Town06: Best for Shanghai Pudong style
EOF

    log_info "CARLA extraction guide created"
}

# Create Blender processing script
create_blender_script() {
    log_step "Creating Blender processing script..."

    cat > "${ASSETS_DIR}/NIO/process_vehicle.py" << 'EOF'
# Blender Python Script - Process NIO Vehicle Models
# Run with: blender --background --python process_vehicle.py -- INPUT_FILE OUTPUT_DIR

import bpy
import sys
import os
from pathlib import Path

def clean_scene():
    """Remove all objects from scene"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)

def import_model(filepath):
    """Import GLB/FBX model"""
    ext = Path(filepath).suffix.lower()
    if ext == '.glb' or ext == '.gltf':
        bpy.ops.import_scene.gltf(filepath=filepath)
    elif ext == '.fbx':
        bpy.ops.import_scene.fbx(filepath=filepath)
    else:
        print(f"Unsupported format: {ext}")
        return False
    return True

def check_topology():
    """Check model topology"""
    for obj in bpy.context.selected_objects:
        if obj.type == 'MESH':
            print(f"Object: {obj.name}")
            print(f"  Vertices: {len(obj.data.vertices)}")
            print(f"  Polygons: {len(obj.data.polygons)}")
            print(f"  Materials: {len(obj.data.materials)}")
    return True

def setup_materials():
    """Setup material slots for UE5"""
    for obj in bpy.context.selected_objects:
        if obj.type == 'MESH':
            # Rename materials for UE5
            for i, mat in enumerate(obj.data.materials):
                if mat is None:
                    continue
                # Create standardized material names
                if 'body' in mat.name.lower() or 'car' in mat.name.lower():
                    mat.name = f"M_Vehicle_Body_{i}"
                elif 'glass' in mat.name.lower() or 'window' in mat.name.lower():
                    mat.name = f"M_Vehicle_Glass_{i}"
                elif 'tire' in mat.name.lower() or 'wheel' in mat.name.lower():
                    mat.name = f"M_Vehicle_Wheel_{i}"
                elif 'light' in mat.name.lower():
                    mat.name = f"M_Vehicle_Light_{i}"
    return True

def generate_lod(target_faces, suffix):
    """Generate LOD using decimate modifier"""
    for obj in bpy.context.selected_objects:
        if obj.type == 'MESH':
            # Add decimate modifier
            mod = obj.modifiers.new(name=f"LOD_{suffix}", type='DECIMATE')
            current_faces = len(obj.data.polygons)
            ratio = min(1.0, target_faces / current_faces)
            mod.ratio = ratio
            mod.use_collapse_triangulate = True

            # Apply modifier
            bpy.context.view_layer.objects.active = obj
            bpy.ops.object.modifier_apply(modifier=f"LOD_{suffix}")

            print(f"LOD {suffix}: {len(obj.data.polygons)} faces (target: {target_faces})")
    return True

def export_fbx(output_path):
    """Export as FBX for UE5"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        apply_scale_options='FBX_SCALE_ALL',
        bake_space_transform=True,
        object_types={'MESH'},
        use_mesh_modifiers=True,
        mesh_smooth_type='FACE',
        use_tspace=True,
        embed_textures=False,
        path_mode='COPY'
    )
    return True

def main():
    # Get command line arguments
    argv = sys.argv
    if '--' in argv:
        argv = argv[argv.index('--') + 1:]
    else:
        print("Usage: blender --background --python process_vehicle.py -- INPUT_FILE OUTPUT_DIR")
        return

    if len(argv) < 2:
        print("Error: Missing arguments")
        print("Usage: blender --background --python process_vehicle.py -- INPUT_FILE OUTPUT_DIR")
        return

    input_file = argv[0]
    output_dir = argv[1]

    print(f"Processing: {input_file}")
    print(f"Output: {output_dir}")

    # Create output directory
    os.makedirs(output_dir, exist_ok=True)

    # Clean scene
    clean_scene()

    # Import model
    if not import_model(input_file):
        print("Failed to import model")
        return

    # Check topology
    check_topology()

    # Setup materials
    setup_materials()

    # Generate LODs
    # LOD0: Original (for Nanite) or 400K faces
    # LOD1: 100K-150K faces
    # LOD2: 20K-30K faces

    # Save LOD0 (original)
    lod0_path = os.path.join(output_dir, "LOD0.fbx")
    export_fbx(lod0_path)
    print(f"LOD0 exported: {lod0_path}")

    # Generate LOD1 (150K faces)
    clean_scene()
    import_model(input_file)
    generate_lod(150000, "1")
    lod1_path = os.path.join(output_dir, "LOD1.fbx")
    export_fbx(lod1_path)
    print(f"LOD1 exported: {lod1_path}")

    # Generate LOD2 (30K faces)
    clean_scene()
    import_model(input_file)
    generate_lod(30000, "2")
    lod2_path = os.path.join(output_dir, "LOD2.fbx")
    export_fbx(lod2_path)
    print(f"LOD2 exported: {lod2_path}")

    print("Processing complete!")

if __name__ == "__main__":
    main()
EOF

    log_info "Blender processing script created at: ${ASSETS_DIR}/NIO/process_vehicle.py"
}

# Create asset manifest
create_asset_manifest() {
    log_step "Creating asset manifest..."

    cat > "${ASSETS_DIR}/manifest.json" << 'EOF'
{
    "version": "1.0.0",
    "project": "NIO Racing Plus",
    "assets": {
        "vehicles": {
            "EP9": {
                "source": "Sketchfab",
                "url": "https://sketchfab.com/3d-models/nio-ep9-2017-b9bfaa1ea4824bef85ea755f8c10c6d2",
                "license": "CC BY 4.0",
                "faces": 1400252,
                "lod0_target": 600000,
                "lod1_target": 150000,
                "lod2_target": 30000
            },
            "ET7": {
                "source": "Sketchfab",
                "url": "https://sketchfab.com/3d-models/nio-et7-2021-b428077c63a743c6bf82059e2ec3b4fb",
                "license": "CC BY 4.0",
                "faces": 1043414,
                "lod0_target": 500000,
                "lod1_target": 120000,
                "lod2_target": 25000
            },
            "ES7": {
                "source": "Sketchfab",
                "url": "https://sketchfab.com/3d-models/nio-es7-2023-4d9c574b84514b21ac783aca550793fe",
                "license": "CC BY 4.0",
                "faces": 588958,
                "lod0_target": 400000,
                "lod1_target": 100000,
                "lod2_target": 20000
            }
        },
        "textures": {
            "source": "ambientCG",
            "license": "CC0",
            "resolution": "2K"
        },
        "hdr": {
            "source": "Poly Haven",
            "license": "CC0",
            "resolution": "4K"
        },
        "carla": {
            "source": "CARLA Simulator",
            "license": "MIT",
            "branch": "ue5-dev",
            "towns": ["Town03", "Town06"]
        }
    }
}
EOF

    log_info "Asset manifest created"
}

# Main
main() {
    log_info "NIO Racing Plus - Asset Setup"
    echo ""

    create_directories
    download_nio_models
    download_ambientcg_assets
    download_hdr_skyboxes
    download_carla_assets
    create_blender_script
    create_asset_manifest

    echo ""
    log_info "Asset setup complete!"
    log_info "Next steps:"
    log_info "1. Download NIO models from Sketchfab (see download_info.txt)"
    log_info "2. Download textures from ambientCG (see download_list.txt)"
    log_info "3. Download HDR skyboxes from Poly Haven (see download_list.txt)"
    log_info "4. Extract CARLA assets (see extraction_guide.txt)"
    log_info "5. Run Blender script to process vehicle models"
}

main
