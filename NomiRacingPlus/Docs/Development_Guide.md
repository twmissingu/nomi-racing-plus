# NIO Racing Plus -- Developer Guide

> The authoritative reference for project setup, architecture, coding standards, and contribution workflow.

---

## Table of Contents

1. [Project Setup](#1-project-setup)
2. [Architecture Overview](#2-architecture-overview)
3. [Coding Standards](#3-coding-standards)
4. [Testing](#4-testing)
5. [Build System](#5-build-system)
6. [Asset Pipeline](#6-asset-pipeline)
7. [Contribution Workflow](#7-contribution-workflow)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Project Setup

### 1.1 Prerequisites

| Tool | Version | Purpose |
|------|---------|---------|
| Unreal Engine | 5.5 | Game engine |
| Visual Studio | 2022 (Windows) | C++ IDE and compiler |
| Xcode | 14+ (macOS) | C++ IDE and compiler |
| Git + Git LFS | Latest | Version control and large asset tracking |
| Blender | 3.6+ | Vehicle model processing and LOD generation |
| Python | 3.8+ | Build scripts and asset processing |

### 1.2 Clone and Initialize

```bash
git clone --recursive https://github.com/your-repo/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus
git lfs install
git lfs pull
```

### 1.3 Environment Configuration

**macOS:**

Set `UE5_ROOT` if UE5 is not installed at the default location (`/Users/Shared/Epic Games/UE_5.5`):

```bash
export UE5_ROOT="/path/to/your/UE_5.5"
```

**Windows:**

Set `UE5_ROOT` if UE5 is not at `C:\Program Files\Epic Games\UE_5.5`:

```powershell
$env:UE5_ROOT = "C:\path\to\UE_5.5"
```

### 1.4 Generate Project Files

```bash
# macOS
./Scripts/build.sh generate

# Windows
# Open NomiRacingPlus.uproject in UE5 Editor, or use:
# <UE5_ROOT>\Engine\Build\BatchFiles\Build.bat NomiRacingPlusEditor Win64 Development -project=<path>\NomiRacingPlus.uproject
```

### 1.5 Initial Build

```bash
# Development build
./Scripts/build.sh build Development

# Or open NomiRacingPlus.uproject in UE5 Editor and let it compile
```

### 1.6 Verify Setup

1. Open `NomiRacingPlus.uproject` in UE5 5.5.
2. The project should compile without errors.
3. Play in Editor (PIE) to verify the default map loads.

### 1.7 IDE Configuration

**Recommended settings for Visual Studio / Rider:**

- Use spaces for indentation (4 spaces).
- Line length limit: 120 characters.
- Enable UE5 coding assistance plugin.

**Recommended settings for Xcode:**

- Generated via `GenerateProjectFiles.sh` from UE5.
- Set indentation to 4 spaces.

---

## 2. Architecture Overview

### 2.1 Project Structure

```
NomiRacingPlus/
├── Source/NomiRacingPlus/         # C++ source code
│   ├── Core/                      # Game mode, instance, player controller, audio, camera, particles
│   ├── Vehicles/                  # Vehicle base class, NIO movement, state manager, tire physics
│   ├── Race/                      # Race manager, checkpoints, progression, championship
│   ├── NOMI/                      # Commentary engine, NOMI controller
│   ├── AI/                        # AI car controller, behavior tree, sensors, overtaking, slipstream
│   ├── UI/                        # HUD, color theme
│   ├── Editor/                    # Editor tools (asset validator, performance profiler)
│   ├── Tests/                     # Automation tests
│   └── NomiRacingPlus.Build.cs    # Module build rules
├── Content/                       # UE5 content (assets)
│   ├── Vehicles/                  # Vehicle meshes, materials, blueprints
│   ├── Maps/                      # Track levels
│   ├── UI/                        # Widget blueprints, fonts
│   ├── NOMI/                      # Comment pool JSON data
│   ├── Audio/                     # Sound assets and MetaSound graphs
│   ├── AI/                        # AI behavior tree assets
│   └── Materials/                 # Shared materials
├── Config/                        # UE5 configuration (.ini files)
├── Scripts/                       # Build scripts, editor utilities
├── Docs/                          # Documentation
└── .github/workflows/             # CI/CD pipeline
```

### 2.2 System Dependency Graph

```
┌─────────────────────────────────────────────────────────────┐
│                    NomiGameInstance                          │
│         (Global state, settings, persistence)               │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   NomiRaceGameMode                          │
│       (Race flow, player/AI spawning, championship)         │
└──┬──────────────┬──────────────┬──────────────┬─────────────┘
   │              │              │              │
   ▼              ▼              ▼              ▼
┌────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│Vehicle │  │  Race    │  │  NOMI    │  │   AI     │
│System  │  │  System  │  │  System  │  │  System  │
└───┬────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘
    │            │              │              │
    ▼            ▼              ▼              ▼
┌────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│Physics │  │Checkpoint│  │Commentary│  │Behavior  │
│& Input │  │& Timing  │  │& Emotion │  │Tree      │
└───┬────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘
    │            │              │              │
    └────────────┴──────────────┴──────────────┘
                         │
                         ▼
                 ┌──────────────┐
                 │   UI / HUD   │
                 │   System     │
                 └──────────────┘
```

### 2.3 Core Systems

| System | Key Classes | Responsibility |
|--------|-------------|----------------|
| **Vehicle** | `ANIOVehicleBase`, `UVehicleStateManager`, `UNIOVehicleMovementComponent`, `UTirePhysicsModel` | Vehicle physics, electric motor torque, regenerative braking, drift detection, tire model |
| **Race** | `ARaceManager`, `ACheckpoint`, `URaceProgression`, `AChampionshipManager` | Race state machine, checkpoint validation, lap timing, position calculation, season points |
| **NOMI** | `UCommentaryEngine`, `ANOMIController` | 500+ comment pool, event-driven matching, cooldown/dedup/queue, emotion states |
| **AI** | `AAICarController`, `UAIBehaviorTree`, `UAISensorSystem`, `UAIOvertakeEvaluator`, `UAIRubberBandScaler` | Path following, overtaking decisions, difficulty scaling, slipstream detection |
| **UI** | `URaceHUD`, `UNIOThemeSubsystem`, `FNIOColorTheme` | HUD layout, NIO brand colors, responsive breakpoints, accessibility |
| **Core** | `ANomiRaceGameMode`, `UNomiGameInstance`, `ANomiPlayerController`, `AAudioManager`, `ACameraSystem` | Game lifecycle, input routing, audio management, camera modes |

### 2.4 Vehicle Hierarchy

```
AWheeledVehiclePawn
└── ANIOVehicleBase (Abstract)
    ├── ANIO_EP9   (Hypercar: 1000kW, 1480Nm, AWD quad-motor)
    ├── ANIO_ET7   (Sedan: 480kW, 850Nm, AWD dual-motor)
    └── ANIO_ES7   (SUV: 480kW, 850Nm, AWD dual-motor)

Components attached to ANIOVehicleBase:
├── UVehicleStateManager        (State tracking: speed, input, drift, battery)
├── UNIOVehicleMovementComponent (Electric motor torque, regen braking)
├── USpotLightComponent x2      (Headlights)
├── UPointLightComponent x2     (Taillights)
└── UAudioComponent             (Motor/SFX)
```

### 2.5 Race State Machine

```
IDLE ──► LOADING ──► COUNTDOWN ──► RACING ──► FINISHED ──► POST_RACE
  ▲                                                              │
  └──────────────────────── (ResetRace) ◄────────────────────────┘
```

State transitions are managed by `ARaceManager`. The `FOnRaceEvent` delegate broadcasts events (`ERaceEvent`) that the NOMI system, HUD, and audio system subscribe to.

### 2.6 Event System

The project uses UE5 multicast delegates for cross-system communication:

```cpp
// RaceManager broadcasts events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceEvent, ERaceEvent, Event, const FRacerData&, RacerData);

// Systems subscribe:
// - CommentaryEngine: receives events, matches comments
// - RaceHUD: updates display
// - AudioManager: triggers sound effects
// - AICarController: reacts to race context
```

### 2.7 Data Flow Example: Overtake

```
1. RaceManager::RacerPassCheckpoint() detects position change
2. RaceManager broadcasts ERaceEvent::Overtake via OnRaceEvent delegate
3. CommentaryEngine receives event, builds FCommentContext
4. CommentaryEngine::RequestComment() searches comment pool
5. Matched comment enters queue (respects cooldown, dedup, priority)
6. NOMIController plays comment (text display + emotion animation)
7. RaceHUD updates position display (green flash for position gain)
```

---

## 3. Coding Standards

### 3.1 Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes (Actor) | `A` prefix + PascalCase | `ANIOVehicleBase`, `ARaceManager` |
| Classes (Object) | `U` prefix + PascalCase | `UVehicleStateManager`, `UCommentaryEngine` |
| Structs | `F` prefix + PascalCase | `FVehicleState`, `FRaceConfig`, `FNOMIComment` |
| Enums | `E` prefix + PascalCase | `ENIOVehicleType`, `ERaceState`, `ERaceEvent` |
| Interfaces | `I` prefix + PascalCase | `IInteractable` |
| Functions | PascalCase | `GetVehicleState()`, `StartRace()` |
| Variables | PascalCase | `SpeedKmh`, `CurrentLap` |
| Boolean variables | `b` prefix + PascalCase | `bIsPlayer`, `bIsDrifting`, `bFinished` |
| Constants | UPPER_SNAKE_CASE | `MAX_LAPS`, `COUNTDOWN_DURATION` |
| Delegates | `F` prefix + `On` + PascalCase | `FOnRaceEvent` |
| Template parameters | `T` prefix | `TArray`, `TMap` |

### 3.2 UE5 Macro Usage

Every class, struct, enum, property, and function exposed to Blueprint must use the appropriate UE5 macro.

**Class declaration:**

```cpp
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ARaceManager : public AActor
{
    GENERATED_BODY()
    // ...
};
```

**Abstract base class:**

```cpp
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API ANIOVehicleBase : public AWheeledVehiclePawn
{
    GENERATED_BODY()
    // ...
};
```

**Component class:**

```cpp
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UCommentaryEngine : public UActorComponent
{
    GENERATED_BODY()
    // ...
};
```

**Struct declaration:**

```cpp
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FVehicleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float SpeedKmh = 0.0f;
};
```

**Enum declaration:**

```cpp
UENUM(BlueprintType)
enum class ERaceState : uint8
{
    Idle      UMETA(DisplayName = "Idle"),
    Racing    UMETA(DisplayName = "Racing"),
    Finished  UMETA(DisplayName = "Finished")
};
```

**Delegate declaration:**

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceEvent, ERaceEvent, Event, const FRacerData&, RacerData);
```

### 3.3 Property Specifiers

| Specifier | Use Case |
|-----------|----------|
| `EditAnywhere` | Configurable in editor and Blueprint defaults |
| `EditDefaultsOnly` | Configurable only in class defaults |
| `VisibleAnywhere` | Visible in editor but not editable |
| `BlueprintReadWrite` | Readable and writable from Blueprint |
| `BlueprintReadOnly` | Readable from Blueprint, writable only from C++ |
| `BlueprintCallable` | Function callable from Blueprint |
| `BlueprintAssignable` | Delegate assignable from Blueprint |

### 3.4 Include Order

Follow this order, with a blank line between each group:

```cpp
// 1. UE5 Engine headers
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// 2. UE5 Plugin headers
#include "WheeledVehiclePawn.h"

// 3. Project headers
#include "Vehicles/VehicleStateManager.h"
#include "Race/RaceManager.h"

// 4. Generated header (always last)
#include "MyClass.generated.h"
```

### 3.5 File Organization

- One primary class per file. The `.h` and `.cpp` filenames must match the class name (minus the prefix).
- Use `#pragma once` instead of include guards.
- Every file starts with: `// Copyright NomiRacingPlus Project. All Rights Reserved.`
- Keep files under 400 lines. Extract components or subsystems when approaching this limit.
- Place files in the appropriate subsystem directory (`Core/`, `Vehicles/`, `Race/`, `NOMI/`, `AI/`, `UI/`, `Tests/`).

### 3.6 Code Style

| Rule | Value |
|------|-------|
| Indentation | 4 spaces (no tabs) |
| Line length | 120 characters max |
| Braces | Opening brace on same line |
| Pointer/Reference | Attach to type: `FString* OutParam`, `const FVehicleState& State` |
| Comments | Doxygen-style for public APIs (`/** ... */`) |
| Magic numbers | Extract to named constants |

### 3.7 Error Handling

- Use `check()` / `ensure()` for programming errors (invariants).
- Use `UE_LOG()` for runtime errors and warnings.
- Validate pointers before use; return early with a log message on failure.
- Never silently swallow errors.

```cpp
void ARaceManager::RacerPassCheckpoint(APawn* VehiclePawn, int32 CheckpointIndex)
{
    if (!VehiclePawn)
    {
        UE_LOG(LogRace, Warning, TEXT("RacerPassCheckpoint: null VehiclePawn"));
        return;
    }
    // ...
}
```

### 3.8 Memory Management

- Use `UPROPERTY()` for all UObject pointers so the garbage collector tracks them.
- Use `TObjectPtr<>` for UPROPERTY pointers (preferred in UE5).
- Use `TWeakObjectPtr<>` for non-owning references that may become invalid.
- Avoid raw `new`/`delete` for UObjects; use `NewObject<>()` or `CreateDefaultSubobject<>()`.

### 3.9 Tick Optimization

- Disable ticking on components that do not need per-frame updates.
- Use `SetComponentTickEnabled(false)` in constructors where appropriate.
- Prefer event-driven updates over polling in Tick.
- Group related per-frame logic into a single Tick function rather than spreading across multiple components.

---

## 4. Testing

### 4.1 Test Framework

The project uses UE5's built-in Automation Testing framework. Tests are defined in `Source/NomiRacingPlus/Tests/`.

### 4.2 Test File Structure

Each test file has a `.h` file that declares test classes and a `.cpp` file that implements them.

**Header (e.g., `VehicleSystemTest.h`):**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVehicleStateManagerTest,
    "NomiRacingPlus.Vehicle.StateManager",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
```

**Implementation (e.g., `VehicleSystemTest.cpp`):**

```cpp
#include "Tests/VehicleSystemTest.h"
#include "Vehicles/VehicleStateManager.h"

bool FVehicleStateManagerTest::RunTest(const FString& Parameters)
{
    // Arrange
    UVehicleStateManager* StateManager = NewObject<UVehicleStateManager>();
    TestNotNull(TEXT("StateManager should be created"), StateManager);

    if (!StateManager) { return false; }

    // Act
    const FVehicleState& InitialState = StateManager->GetVehicleState();

    // Assert
    TestEqual(TEXT("Initial speed should be 0"), InitialState.SpeedKmh, 0.0f);
    TestFalse(TEXT("Initial drift should be false"), InitialState.bIsDrifting);

    return true;
}
```

### 4.3 Test Naming Convention

Test names follow the pattern: `NomiRacingPlus.<System>.<TestName>`

Examples:
- `NomiRacingPlus.Vehicle.StateManager`
- `NomiRacingPlus.Vehicle.NIOPhysics`
- `NomiRacingPlus.Vehicle.DriftDetection`
- `NomiRacingPlus.Race.StateMachine`
- `NomiRacingPlus.NOMI.CommentMatching`

### 4.4 Existing Test Suites

| Suite | File | Tests | Coverage |
|-------|------|-------|----------|
| Vehicle | `VehicleSystemTest.cpp` | State manager, NIO physics, drift detection, electric torque, tire model | Vehicle state, physics config, drift thresholds, torque curves, Pacejka model |
| Race | `RaceSystemTest.cpp` | State machine, checkpoint validation, position calculation, timer | Race flow, checkpoint logic, ranking |
| NOMI | `NOMISystemTest.cpp` | Init, matching, queue, cooldown, emotion | Commentary engine core logic |
| AI | `AITest.cpp` | Difficulty settings, state transitions, waypoint following | AI controller basics |
| Camera | `CameraSystemTest.cpp` | Camera modes and transitions | Camera system |
| Progression | `ProgressionTest.cpp` | Championship and season tracking | Championship manager |
| Tire | `TirePhysicsTest.cpp` | Pacejka formula, surface grip, thermal model | Tire physics model |

### 4.5 Running Tests

```bash
# Via UE5 Editor
# Window > Test Automation > Select tests > Start Tests

# Via command line
./Scripts/build.sh build Development
# Then use UE5's automation commandlet
```

### 4.6 Writing New Tests

1. Create a test header in `Tests/` with the `IMPLEMENT_SIMPLE_AUTOMATION_TEST` macro.
2. Implement `RunTest()` in the corresponding `.cpp` file.
3. Follow the Arrange-Act-Assert pattern.
4. Use `TestEqual`, `TestTrue`, `TestFalse`, `TestNotNull` for assertions.
5. Return `true` on success, `false` on failure.
6. Name the test: `NomiRacingPlus.<System>.<TestName>`.

---

## 5. Build System

### 5.1 Build Configurations

| Configuration | Optimization | Debug Symbols | Use Case |
|--------------|-------------|---------------|----------|
| Debug | None | Full | Debugger attached, step-through |
| Development | Partial | Yes | Day-to-day iteration |
| Shipping | Full | Stripped | Release builds |

### 5.2 Quick Build Commands

```bash
# Development build
./Scripts/build.sh build Development

# Shipping build
./Scripts/build_macos.sh release              # macOS: build + sign + notarize + DMG
./Scripts/build_windows.bat release           # Windows: build + sign + installer

# Skip signing for local testing
./Scripts/build_macos.sh build --skip-sign
./Scripts/build_windows.bat build --skip-sign

# Clean all build artifacts
./Scripts/build.sh clean
```

### 5.3 Module Dependencies

The project module (`NomiRacingPlus.Build.cs`) depends on:

| Module | Purpose |
|--------|---------|
| `Core`, `CoreUObject` | UE5 foundation |
| `Engine` | Game framework |
| `InputCore`, `EnhancedInput` | Input handling |
| `ChaosVehicles` | Vehicle physics |
| `UMG`, `Slate`, `SlateCore` | UI framework |
| `Json`, `JsonUtilities` | Comment pool JSON parsing |
| `PhysicsCore`, `RenderCore`, `RHI` | Low-level engine |

### 5.4 CI/CD Pipeline

The `.github/workflows/build.yml` pipeline runs on push to `main`/`develop` and on pull requests:

1. **Preflight** -- validate configuration
2. **Windows Build** -- build, sign, package
3. **macOS Build** -- build, sign, notarize, DMG
4. **Tests** -- run UE5 automation tests
5. **Code Quality** -- check for issues
6. **Release** -- create GitHub Release (on tag push)

### 5.5 Platform-Specific Notes

**macOS:**
- Nanite is not fully supported on Metal; the project uses manual LOD levels.
- Lumen falls back to software tracing.
- Code signing requires a Developer ID Application certificate.
- Notarization is required for Gatekeeper compliance.

**Windows:**
- Nanite and Lumen are fully supported with hardware ray tracing.
- Code signing requires a PFX certificate.
- NSIS or Inno Setup creates the installer.

---

## 6. Asset Pipeline

### 6.1 Asset Naming Conventions

| Asset Type | Prefix | Example |
|-----------|--------|---------|
| Static Mesh | `SM_` | `SM_Vehicle_EP9` |
| Skeletal Mesh | `SK_` | `SK_Character_NOMI` |
| Material | `M_` | `M_Vehicle_Body` |
| Material Instance | `MI_` | `MI_Vehicle_EP9_Red` |
| Texture | `T_` | `T_Vehicle_Body_D` (diffuse), `_N` (normal), `_R` (roughness) |
| Blueprint | `BP_` | `BP_Vehicle_EP9` |
| Widget | `W_` | `W_RaceHUD` |
| Sound | `S_` | `S_Motor_EP9` |
| Map | `MAP_` | `MAP_NIOCityCircuit` |

### 6.2 Vehicle Model Processing

```
Sketchfab download (.glb/.fbx)
    │
    ▼
Blender import + topology check + UV fix + material slots
    │
    ├─ LOD generation (Decimate modifier)
    │   ├─ LOD0: original poly count
    │   ├─ LOD1: ~100K-150K faces
    │   └─ LOD2: ~20K-30K faces
    │
    ▼
Export FBX (with LOD levels and material references)
    │
    ▼
UE5 import → Vehicle Blueprint → Chaos Vehicles config → PBR materials → collision
```

### 6.3 LOD Targets

| LOD | Distance | EP9 | ET7 | ES7 |
|-----|----------|-----|-----|-----|
| LOD0 | 0-10m | 600K | 500K | 400K |
| LOD1 | 10-50m | 150K | 120K | 100K |
| LOD2 | 50m+ | 30K | 25K | 20K |

### 6.4 Asset Sources and Licenses

| Source | Content | License |
|--------|---------|---------|
| Sketchfab | NIO vehicle models | CC BY 4.0 |
| CARLA | City maps, road assets | MIT |
| Poly Haven | HDR skyboxes, PBR textures | CC0 |
| ambientCG | PBR materials | CC0 |
| Quixel Megascans (Fab) | Rocks, vegetation | Free for UE |
| Freesound.org | Audio samples | CC0 / CC-BY |
| Kenney | UI audio | CC0 |

### 6.5 Asset Archive

All external assets are archived locally in `assets-archive/` (managed by Git LFS) with a `LICENSES.md` recording source, license, and download date for each asset. See `PLAN.md` section 2.3 for the full archive structure.

---

## 7. Contribution Workflow

### 7.1 Branch Strategy

```
main (stable releases)
├── develop (integration branch)
│   ├── feature/vehicle-physics
│   ├── feature/nomi-commentary
│   ├── feature/ai-behavior-tree
│   ├── fix/checkpoint-validation
│   └── refactor/tire-model
└── release/v1.0
```

### 7.2 Development Process

**Step 1: Create a feature branch**

```bash
git checkout develop
git pull origin develop
git checkout -b feature/my-feature
```

**Step 2: Implement changes**

- Follow coding standards (section 3).
- Write tests for new functionality (section 4).
- Update documentation if public APIs change.

**Step 3: Verify locally**

```bash
# Build
./Scripts/build.sh build Development

# Run tests (via UE5 Editor automation or command line)
# Verify in PIE (Play In Editor)
```

**Step 4: Commit**

```bash
git add <files>
git commit -m "feat: Add EP9 torque curve configuration"
```

**Step 5: Push and create PR**

```bash
git push -u origin feature/my-feature
# Create Pull Request on GitHub targeting 'develop'
```

### 7.3 Commit Message Format

```
<type>: <description>

<optional body>
```

Types:

| Type | Use |
|------|-----|
| `feat` | New feature |
| `fix` | Bug fix |
| `refactor` | Code restructuring without behavior change |
| `docs` | Documentation only |
| `test` | Adding or updating tests |
| `chore` | Build scripts, CI, dependencies |
| `perf` | Performance improvement |
| `ci` | CI/CD pipeline changes |

Examples:

```
feat: Add EP9 vehicle physics configuration
fix: Correct tire slip angle calculation at low speed
refactor: Extract race state machine into separate class
docs: Update API reference for CommentaryEngine
test: Add drift detection edge case tests
chore: Update build script for macOS notarization
```

### 7.4 Pull Request Guidelines

1. Target the `develop` branch.
2. Provide a clear description of what changed and why.
3. Reference any related issues.
4. Ensure the build passes on your local machine before pushing.
5. Include screenshots or GIFs for visual changes (HUD, menus, vehicle appearance).

### 7.5 Code Review Checklist

Before requesting review, verify:

- [ ] Code follows naming conventions (section 3.1)
- [ ] UE5 macros are used correctly (section 3.2)
- [ ] No hardcoded magic numbers (extract to constants)
- [ ] No raw UObject pointers without `UPROPERTY()`
- [ ] Proper error handling (null checks, logging)
- [ ] No `console.log` / debug print statements left in
- [ ] Tests written for new functionality
- [ ] Files under 400 lines
- [ ] Functions under 50 lines
- [ ] No nesting deeper than 4 levels
- [ ] Comments on complex logic
- [ ] No unrelated changes bundled in

### 7.6 Common Commit Patterns

**Adding a new vehicle:**

```
feat: Add NIO ET5 vehicle class and physics config
test: Add ET5 torque curve and state manager tests
```

**Fixing a race bug:**

```
fix: Prevent lap count increment on wrong checkpoint order
test: Add checkpoint sequence validation test
```

**Updating NOMI comments:**

```
feat: Add 50 new comfort comments for frustration scenarios
```

---

## 8. Troubleshooting

### 8.1 Build Failures

**"Module not found" error:**
Regenerate project files and rebuild:
```bash
./Scripts/build.sh generate
./Scripts/build.sh build Development
```

**"ChaosVehicles" module not found:**
Ensure the `ChaosVehiclesPlugin` is enabled in `NomiRacingPlus.uproject` and UE5 5.5 is installed.

**macOS Metal shader compilation errors:**
Check `Config/DefaultEngine.ini` for `r.Metal.ForceNonAirArgumentBuffers=1`. This setting is required for macOS compatibility.

### 8.2 Runtime Issues

**Vehicle not responding to input:**
Verify Enhanced Input actions are configured in `Config/DefaultInput.ini` and the Player Controller is binding them.

**NOMI comments not appearing:**
Check that the comment pool JSON is loaded (`CommentaryEngine::LoadCommentPool`). Verify cooldown has elapsed (default 3 seconds).

**AI vehicles not moving:**
Ensure waypoints are set (`AAICarController::SetWaypoints`) and `StartRacing()` is called.

### 8.3 Performance Issues

**Low FPS on city tracks:**
- Check LOD settings on static meshes.
- Enable occlusion culling.
- Reduce draw calls by merging static meshes where possible.
- Use `stat fps` and `stat unit` in-editor to identify bottlenecks.

**macOS-specific performance:**
- Nanite is disabled; ensure manual LOD levels are set on all high-poly meshes.
- Lumen uses software tracing; consider baked lighting for performance-critical tracks.

### 8.4 Asset Import Issues

**"Unsupported format" error:**
Convert to FBX or GLB using Blender before importing.

**Materials look wrong after import:**
Re-assign material slots in the UE5 static mesh editor. Verify PBR texture maps (diffuse, normal, roughness) are correctly linked.

### 8.5 Debug Tools

| Tool | Command / Location | Purpose |
|------|-------------------|---------|
| Output Log | Window > Output Log | Runtime logging (`UE_LOG`) |
| Unreal Insights | `-trace=cpu,frame` command line | Performance profiling |
| Automation Testing | Window > Test Automation | Run unit tests |
| stat fps | Console command | Frame rate display |
| stat unit | Console command | Per-frame timing breakdown |
| Visual Studio / Xcode Debugger | Attach to UE5 process | Step-through debugging |

### 8.6 Getting Help

1. Check this guide and the [API Reference](API_Reference.md).
2. Review the [Build System](Build_System.md) documentation.
3. Review the [Asset Pipeline Guide](Asset_Pipeline_Guide.md).
4. Check existing issues on the repository.
5. Create a new issue with reproduction steps.
