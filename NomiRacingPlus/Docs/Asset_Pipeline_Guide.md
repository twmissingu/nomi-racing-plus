# NIO Racing Plus - Asset Pipeline Guide

> Complete guide to importing and managing game assets

---

## Table of Contents

1. [Overview](#overview)
2. [Vehicle Assets](#vehicle-assets)
3. [Track Assets](#track-assets)
4. [Texture Assets](#texture-assets)
5. [Audio Assets](#audio-assets)
6. [LOD Generation](#lod-generation)
7. [Material Setup](#material-setup)
8. [Optimization](#optimization)

---

## Overview

### Asset Pipeline Flow

```
Source Assets → Processing → Import → Configuration → Testing → Release
     │              │           │            │            │         │
     ▼              ▼           ▼            ▼            ▼         ▼
  Download    Blender/UE5    UE5 Editor   Blueprint    Playtest   Package
```

### Asset Categories

| Category | Source | Format | Size |
|----------|--------|--------|------|
| **Vehicles** | Sketchfab | GLB/FBX | 500K-1.5M polygons |
| **Tracks** | CARLA/Custom | UAsset | Varies |
| **Textures** | ambientCG | PNG/TGA | 2K-4K |
| **Audio** | Freesound | WAV/OGG | Various |
| **HDR** | Poly Haven | HDR | 4K |

### File Organization

```
Content/
├── Vehicles/
│   ├── EP9/
│   │   ├── Meshes/
│   │   ├── Materials/
│   │   └── Textures/
│   ├── ET7/
│   └── ES7/
├── Maps/
│   ├── NIOCityCircuit/
│   ├── ShanghaiPudong/
│   └── SpeedwayOval/
├── Audio/
│   ├── Motor/
│   ├── SFX/
│   └── Music/
└── UI/
    ├── HUD/
    └── Menus/
```

---

## Vehicle Assets

### Download from Sketchfab

1. **Visit Sketchfab Model Page**
   - EP9: https://sketchfab.com/3d-models/nio-ep9-2017-b9bfaa1ea4824bef85ea755f8c10c6d2
   - ET7: https://sketchfab.com/3d-models/nio-et7-2021-b428077c63a743c6bf82059e2ec3b4fb
   - ES7: https://sketchfab.com/3d-models/nio-es7-2023-4d9c574b84514b21ac783aca550793fe

2. **Download Format**
   - Select "glTF Binary (.glb)" format
   - Download to `Assets/NIO/[VehicleName]/`

3. **Verify License**
   - All models: CC BY 4.0
   - Credit required in game

### Process with Blender

```bash
# Process single vehicle
blender --background --python Scripts/process_vehicle.py -- Assets/NIO/EP9/source.glb Content/Vehicles/EP9/

# Process all vehicles
./Scripts/process_all_vehicles.sh
```

### Blender Processing Steps

1. **Import GLB/FBX**
   - File → Import → glTF 2.0 (.glb/.gltf)
   - Select source file

2. **Check Topology**
   - Verify polygon count
   - Check for non-manifold geometry
   - Fix UV mapping if needed

3. **Setup Materials**
   - Create material slots: Body, Glass, Wheel, Light, Interior
   - Assign materials to faces

4. **Generate LODs**
   - LOD0: Original (for Nanite) or 400K-600K faces
   - LOD1: 100K-150K faces
   - LOD2: 20K-30K faces

5. **Export FBX**
   - File → Export → FBX
   - Settings: Apply Scalings = FBX All, Forward = -Z, Up = Y

### Import to UE5

1. **Open Content Browser**
   - Navigate to `Content/Vehicles/[VehicleName]/`

2. **Import FBX**
   - Drag LOD0.fbx to Content Browser
   - Import Settings:
     - Skeletal Mesh: No
     - Import LODs: Yes
     - Generate Lightmap UVs: Yes
     - Combine Meshes: No

3. **Configure LOD**
   - Open Static Mesh Editor
   - LOD Settings → Auto Compute LOD Distances
   - Set LOD0: 0-10m, LOD1: 10-50m, LOD2: 50m+

4. **Create Vehicle Blueprint**
   - Right-click → Blueprint Class → WheeledVehiclePawn
   - Assign Static Mesh
   - Configure ChaosVehicleMovementComponent

---

## Track Assets

### CARLA Asset Extraction

1. **Clone CARLA Repository**
   ```bash
   git clone --branch ue5-dev https://github.com/carla-simulator/carla.git
   ```

2. **Locate Assets**
   - Maps: `carla/Unreal/CarlaUnreal/Content/Carla/Maps/`
   - Vehicles: `carla/Unreal/CarlaUnreal/Content/Carla/Vehicles/`

3. **Copy to Project**
   ```bash
   cp -r carla/Unreal/CarlaUnreal/Content/Carla/Maps/Town03 Content/Maps/NIOCityCircuit/
   ```

### Track Customization

1. **Remove Unnecessary Elements**
   - Delete autonomous driving sensors
   - Remove test markers
   - Keep static environment

2. **Add Racing Elements**
   - Start/Finish line
   - Checkpoint triggers
   - AI waypoints
   - Track barriers

3. **Add NIO Branding**
   - NIO billboards
   - Swap station
   - NIO-themed decorations

---

## Texture Assets

### Download from ambientCG

1. **Visit ambientCG**
   - URL: https://ambientcg.com
   - License: CC0 (Public Domain)

2. **Download Textures**
   - Resolution: 2K (2048x2048)
   - Format: PNG
   - Maps: Albedo, Normal, Roughness, Metallic

3. **Required Textures**
   - Road/Asphalt
   - Concrete
   - Metal
   - Glass
   - Rubber (tires)

### Import to UE5

1. **Import Textures**
   - Drag to Content Browser
   - Settings:
     - Compression: Default (DXT5)
     - Mip Gen Settings: Sharpen 0

2. **Create Material**
   - Right-click → Material
   - Connect texture maps:
     - Albedo → Base Color
     - Normal → Normal
     - Roughness → Roughness
     - Metallic → Metallic

### Material Instances

```
M_Vehicle_Body (Master Material)
├── MI_Vehicle_Body_Red (Material Instance)
├── MI_Vehicle_Body_Blue (Material Instance)
└── MI_Vehicle_Body_Silver (Material Instance)
```

---

## Audio Assets

### Download Sources

| Source | URL | License |
|--------|-----|---------|
| **Freesound** | https://freesound.org | CC0/CC-BY |
| **Sonniss** | https://sonniss.com/gameaudiogdc | Free commercial |
| **Kenney** | https://kenney.nl | CC0 |

### Required Audio

| Category | Files | Notes |
|----------|-------|-------|
| **Motor** | EP9, ET7, ES7 loops | Electric whine |
| **Wind** | Wind loop | Speed-based |
| **Tire** | Screech, scrub | Slip-based |
| **Collision** | 3-4 impacts | Various forces |
| **UI** | Click, hover, success | Interface feedback |
| **NOMI** | Voice clips | Optional |

### Import to UE5

1. **Import Audio**
   - Drag WAV/OGG to Content Browser
   - Settings:
     - Sound Class: SFX/Motor/Music
     - Attenuation: Configure 3D settings

2. **Create Sound Cues**
   - Right-click → Sound Cue
   - Connect nodes:
     - Mixer (volume control)
     - Randomizer (pitch/volume variation)
     - Attenuation (3D audio)

---

## LOD Generation

### LOD Requirements

| LOD Level | Polygon Count | Distance | Use Case |
|-----------|--------------|----------|----------|
| **LOD0** | 400K-600K | 0-10m | Nanite (Windows) |
| **LOD1** | 100K-150K | 10-50m | Standard rendering |
| **LOD2** | 20K-30K | 50m+ | Distant objects |

### Auto LOD in UE5

1. **Open Static Mesh Editor**
2. **LOD Settings**
   - Auto Compute LOD Distances: Yes
   - LOD0 Screen Size: 1.0
   - LOD1 Screen Size: 0.5
   - LOD2 Screen Size: 0.25

### Manual LOD with Blender

```python
# Generate LOD1 (100K faces)
bpy.ops.object.modifier_add(type='DECIMATE')
bpy.context.object.modifiers["Decimate"].ratio = 100000 / current_faces
bpy.ops.object.modifier_apply(modifier="Decimate")
```

---

## Material Setup

### Vehicle Materials

| Material | Maps | Settings |
|----------|------|----------|
| **Body** | Albedo, Normal, Metallic, Roughness | Clear coat, Metallic |
| **Glass** | Opacity, Normal | Translucent, Refraction |
| **Wheel** | Albedo, Normal, Roughness | Metallic |
| **Tire** | Albedo, Normal, Roughness | Non-metallic |
| **Light** | Emissive, Normal | Emissive, Bloom |

### Master Material Setup

```
M_Vehicle_Master
├─ Texture Sample (Albedo)
├─ Texture Sample (Normal)
├─ Texture Sample (Metallic/Roughness/AO)
├─ Scalar Parameter (Metallic)
├─ Scalar Parameter (Roughness)
├─ Vector Parameter (Color Tint)
└─ Material Output
```

### Material Instances

Create instances for color variations:
- Red, Blue, Silver, Black, White
- Adjust Color Tint parameter

---

## Optimization

### Performance Budgets

| Category | Budget | Notes |
|----------|--------|-------|
| **Triangles** | 5M per frame | All objects |
| **Draw Calls** | 2000 per frame | Batch where possible |
| **Texture Memory** | 2GB | Use streaming |
| **Audio Memory** | 512MB | Stream long clips |

### Optimization Techniques

1. **LOD Usage**
   - Enable auto LOD
   - Set appropriate distances
   - Use Nanite on Windows

2. **Texture Streaming**
   - Enable texture streaming
   - Set appropriate mip levels
   - Use virtual textures

3. **Material Optimization**
   - Use material instances
   - Minimize instruction count
   - Use material LODs

4. **Audio Optimization**
   - Stream long clips
   - Compress audio
   - Use sound cues for variation

### Validation

Run asset validation before release:
```bash
./Scripts/validate_assets.sh
```

Checks:
- Polygon counts within limits
- Texture sizes appropriate
- Materials optimized
- Naming conventions followed

---

## Checklist

### Vehicle Import Checklist

- [ ] Source model downloaded (GLB/FBX)
- [ ] Blender processing complete
- [ ] LODs generated (0, 1, 2)
- [ ] Materials assigned
- [ ] FBX exported
- [ ] UE5 import successful
- [ ] LOD distances configured
- [ ] Vehicle Blueprint created
- [ ] ChaosVehicle configured
- [ ] Physics parameters set
- [ ] Test drive successful

### Track Import Checklist

- [ ] CARLA assets extracted
- [ ] Unnecessary elements removed
- [ ] Racing elements added
- [ ] Checkpoints configured
- [ ] AI waypoints placed
- [ ] Lighting configured
- [ ] Performance acceptable
- [ ] Test race complete

### Audio Import Checklist

- [ ] Audio files downloaded
- [ ] Formats converted (WAV/OGG)
- [ ] Imported to UE5
- [ ] Sound classes assigned
- [ ] Sound cues created
- [ ] 3D attenuation configured
- [ ] Volume levels balanced
- [ ] Test in-game

---

*This guide ensures consistent, high-quality assets for NIO Racing Plus.*
