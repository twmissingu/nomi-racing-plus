# NIO Racing Plus - Complete Setup Guide

> Step-by-step guide to set up and run NIO Racing Plus

---

## Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| Unreal Engine | 5.7 | Download from Epic Games Launcher |
| Visual Studio | 2022 | Windows only, for C++ compilation |
| Xcode | 15+ | macOS only |
| Python | 3.10+ | For asset processing scripts |
| Blender | 3.6+ | For vehicle model processing |
| Git | 2.30+ | For version control |

---

## Quick Start (5 Minutes)

### Step 1: Clone Repository

```bash
git clone https://github.com/twmissingu/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus
```

### Step 2: Download Assets

```bash
# Make script executable
chmod +x Scripts/download_assets.sh

# Download free assets (textures, HDR, audio)
./Scripts/download_assets.sh
```

### Step 3: Open in UE5

1. Double-click `NomiRacingPlus.uproject`
2. Wait for shader compilation (first time only)
3. Open Output Log: `Window → Output Log`

### Step 4: Run Full Setup

In the Output Log, type:

```python
exec(open('Scripts/Editor/run_full_setup.py').read())
```

This will:
- Import all assets
- Create materials
- Build vehicle Blueprints
- Create all tracks
- Configure audio
- Set up AI
- Balance the game
- Run tests

### Step 5: Play!

Press the **Play** button in the editor to test the game.

---

## Detailed Setup

### Downloading Vehicle Models

NIO vehicle models must be downloaded manually from Sketchfab:

| Vehicle | URL | License |
|---------|-----|---------|
| EP9 | https://sketchfab.com/3d-models/nio-ep9-2017-b9bfaa1ea4824bef85ea755f8c10c6d2 | CC BY 4.0 |
| ET7 | https://sketchfab.com/3d-models/nio-et7-2021-b428077c63a743c6bf82059e2ec3b4fb | CC BY 4.0 |
| ES7 | https://sketchfab.com/3d-models/nio-es7-2023-4d9c574b84514b21ac783aca550793fe | CC BY 4.0 |

**Download Steps:**
1. Visit each URL
2. Click "Download 3D Model"
3. Select "glTF Binary (.glb)" format
4. Save to `Assets/NIO/[VehicleName]/`

### Processing Vehicle Models with Blender

```bash
# Process single vehicle
blender --background --python Scripts/process_vehicle.py -- Assets/NIO/EP9/source.glb Content/Vehicles/EP9/

# Process all vehicles
./Scripts/process_all_vehicles.sh
```

### Importing to UE5

Run in Output Log:

```python
# Import all assets
exec(open('Scripts/Editor/batch_import.py').read())

# Generate LODs
exec(open('Scripts/Editor/lod_generation.py').read())

# Create materials
exec(open('Scripts/Editor/material_setup.py').read())
```

---

## Script Reference

### Editor Scripts

| Script | Purpose | Command |
|--------|---------|---------|
| `run_full_setup.py` | Complete game setup | `exec(open('Scripts/Editor/run_full_setup.py').read())` |
| `batch_import.py` | Import all assets | `exec(open('Scripts/Editor/batch_import.py').read())` |
| `lod_generation.py` | Generate LOD levels | `exec(open('Scripts/Editor/lod_generation.py').read())` |
| `material_setup.py` | Create materials | `exec(open('Scripts/Editor/material_setup.py').read())` |
| `track_builder.py` | Create racing tracks | `exec(open('Scripts/Editor/track_builder.py').read())` |
| `vehicle_blueprint_builder.py` | Build vehicle BPs | `exec(open('Scripts/Editor/vehicle_blueprint_builder.py').read())` |
| `game_balance.py` | Balance tuning | `exec(open('Scripts/Editor/game_balance.py').read())` |
| `test_scenarios.py` | Run tests | `exec(open('Scripts/Editor/test_scenarios.py').read())` |

### Shell Scripts

| Script | Purpose | Command |
|--------|---------|---------|
| `download_assets.sh` | Download free assets | `./Scripts/download_assets.sh` |
| `setup_assets.sh` | Create asset structure | `./Scripts/setup_assets.sh` |
| `build.sh` | Build project | `./Scripts/build.sh` |
| `build_macos.sh` | Build for macOS | `./Scripts/build_macos.sh` |
| `build_windows.bat` | Build for Windows | `Scripts\build_windows.bat` |

---

## Balance Tuning

### Vehicle Balance

```python
# View vehicle stats
VehicleBalancer('EP9').print_stats()
VehicleBalancer('ET7').print_stats()
VehicleBalancer('ES7').print_stats()

# Adjust parameters
VehicleBalancer('EP9').adjust_parameter('motor_torque', 1500)
VehicleBalancer('ET7').adjust_parameter('vehicle_mass', 2200)

# Apply presets
apply_arcade_preset()      # More forgiving physics
apply_simulation_preset()  # Realistic physics
```

### AI Balance

```python
# View AI profiles
AIBalancer().print_profiles()

# Adjust difficulty
AIBalancer().adjust_difficulty('medium', {
    'reaction_time': 0.2,
    'accuracy': 0.85,
    'aggression': 0.5
})
```

### Race Balance

```python
# View race modes
RaceBalancer().print_race_modes()

# Adjust scoring
RaceBalancer().adjust_scoring('Street GT', {
    '1st': 25,
    '2nd': 18,
    '3rd': 15
})
```

---

## Testing

### Run All Tests

```python
exec(open('Scripts/Editor/test_scenarios.py').read())
run_all_tests()
```

### Run Specific Tests

```python
# Smoke test (essential only)
run_quick_smoke_test()

# Performance test
run_performance_test()

# AI test
run_ai_test()

# Print checklist
print_test_checklist()
```

### Manual Test Checklist

See `test_scenarios.py` for the complete checklist, or run:

```python
print_test_checklist()
```

---

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| **Shader compilation slow** | Wait for completion, reduce shader quality |
| **Assets not found** | Run `download_assets.sh` first |
| **Build fails** | Check UE5 version (5.7 required) |
| **Physics jittery** | Increase physics substeps in DefaultEngine.ini |
| **Low FPS** | Reduce graphics settings, enable LODs |
| **Audio not playing** | Check AudioConfig.json paths |

### Debug Commands

```python
# Check asset status
unreal.EditorAssetLibrary.list_assets('/Game/Vehicles')

# Validate assets
exec(open('Scripts/Editor/asset_validator.py').read())

# Check Blueprint status
exec(open('Scripts/Editor/blueprint_compile.py').read())
```

---

## Next Steps

After setup is complete:

1. **Test Drive**: Press Play and test each vehicle
2. **Race Test**: Complete a full race on each track
3. **Balance**: Adjust parameters using `game_balance.py`
4. **Optimize**: Run performance tests
5. **Release**: Create a GitHub Release

---

## File Structure After Setup

```
NomiRacingPlus/
├── Assets/                    # Downloaded source assets
│   ├── NIO/                   # Vehicle models
│   ├── Textures/              # PBR textures
│   ├── HDR/                   # Skybox HDRs
│   └── Audio/                 # Sound files
├── Content/                   # UE5 content (auto-populated)
│   ├── Vehicles/              # Vehicle Blueprints
│   ├── Maps/                  # Track levels
│   ├── Materials/             # Materials
│   ├── Audio/                 # Sound assets
│   └── UI/                    # UI widgets
├── Scripts/
│   ├── Editor/                # UE5 Python scripts
│   └── *.sh                   # Shell scripts
└── Docs/                      # Documentation
```

---

## Support

- **Issues**: https://github.com/twmissingu/nomi-racing-plus/issues
- **Discussions**: https://github.com/twmissingu/nomi-racing-plus/discussions
- **Email**: support@nio-racing-plus.dev

---

*Happy Racing! 🏎️*
