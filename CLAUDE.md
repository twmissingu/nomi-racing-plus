# CLAUDE.md - AI Agent Instructions

This file provides instructions for AI agents working with the NIO Racing Plus project.

## Project Overview

NIO Racing Plus is a UE5 racing game featuring NIO electric vehicles with realistic physics, NOMI AI companion, and intelligent AI opponents.

## Quick Reference

### Project Structure
```
NomiRacingPlus/
├── Source/NomiRacingPlus/    # C++ source code
│   ├── AI/                   # AI opponent system
│   ├── Camera/               # Camera system (7 modes)
│   ├── Core/                 # Core game systems
│   ├── Effects/              # Particle effects
│   ├── NOMI/                 # NOMI companion system
│   ├── Race/                 # Race management
│   ├── Tests/                # Test suite
│   ├── UI/                   # User interface
│   └── Vehicles/             # Vehicle physics
├── Config/                   # Engine configuration
├── Content/                  # Game assets (JSON)
└── Scripts/                  # Build scripts
```

### Key Files

| File | Purpose |
|------|---------|
| `NomiRacingPlus.uproject` | UE5 project file |
| `Vehicles/NIOVehicleMovementComponent.cpp` | Electric vehicle physics |
| `Vehicles/TirePhysicsModel.cpp` | Pacejka tire model |
| `AI/AIBehaviorTree.cpp` | AI decision making |
| `NOMI/CommentaryEngine.cpp` | NOMI comment system |
| `Race/RaceManager.cpp` | Race state machine |
| `Core/CameraSystem.cpp` | Camera management |

### Common Tasks

#### Adding a New Vehicle
1. Create header/cpp in `Vehicles/`
2. Inherit from `ANIOVehicleBase`
3. Configure in `VehicleStateManager::BeginPlay()`
4. Add to `ENIOVehicleType` enum
5. Add physics JSON in `Content/Vehicles/`

#### Adding AI Behavior
1. Modify `AIBehaviorTree::EvaluateBehavior()`
2. Add sensor data in `AISensorSystem`
3. Configure difficulty in `AIProfiles.json`

#### Adding NOMI Comments
1. Add comments to `Content/NOMI/Comments/DefaultComments.json`
2. Map event type in `CommentaryEngine::LoadCommentPool()`

### Build Commands

```bash
# Windows (Visual Studio)
# Right-click .uproject → Generate VS project files
# Open .sln → Build (Ctrl+Shift+B)

# macOS (Xcode)
# Generate Xcode project via UE5
# Open .xcodeproj → Build (Cmd+B)

# Run tests (inside UE5 Editor)
# Window → Test Automation → Run Tests
```

### Code Style

- Follow UE5 coding conventions
- Use `UCLASS()`, `USTRUCT()`, `UPROPERTY()`, `UFUNCTION()` macros
- Prefix interfaces with `I`
- Prefix enums with `E`
- Use `F` prefix for non-UCLASS structs

### Testing

```bash
# Tests are in Source/NomiRacingPlus/Tests/
# Run via UE5 Editor: Window → Test Automation

# Test categories:
# - Unit: Individual component tests
# - Integration: System interaction tests
# - Performance: Benchmark tests
```

### Configuration Files

| File | Format | Purpose |
|------|--------|---------|
| `Content/Vehicles/VehicleConfig.json` | JSON | Vehicle physics params |
| `Content/AI/AIProfiles.json` | JSON | AI difficulty settings |
| `Content/NOMI/Comments/DefaultComments.json` | JSON | NOMI comment pool |
| `Config/DefaultEngine.ini` | INI | Engine settings |
| `Config/DefaultInput.ini` | INI | Input bindings |

### Performance Considerations

- Cache `FindComponentByClass()` results in `BeginPlay()`
- Always null-check `GetWorld()` before use
- Use `CachedWorld` member when available
- Avoid `RemoveAt(0)` on large arrays (use circular buffer)
- Profile with `PerformanceProfiler` component

### Debugging

- Use `UE_LOG(LogNomiXxx, ...)` for logging
- Log categories: `LogNomiRacing`, `LogNomiVehicle`, `LogNomiNOMI`, `LogNomiRace`, `LogNomiCamera`, `LogNomiPerf`
- Enable verbose logging: `-LogCmds="LogNomiXxx Verbose"`

### Git Workflow

```bash
# Branch naming
feature/description
fix/description
refactor/description

# Commit format
type: description

# Types: feat, fix, refactor, docs, test, chore, perf, ci
```

### Common Pitfalls

1. **Forgetting null checks** - Always check `GetWorld()`, `GetOwner()`, component pointers
2. **Calling FindComponentByClass every frame** - Cache in `BeginPlay()`
3. **Not handling edge cases** - Division by zero, empty arrays, invalid indices
4. **Memory leaks** - Destroy old `UAudioComponent` before creating new ones
5. **Event spam** - Use change detection to avoid broadcasting every tick

### Dependencies

- Unreal Engine 5.5
- Chaos Vehicles Plugin
- Enhanced Input Plugin
- Niagara Plugin (for particles)

### Cross-Platform Notes

| Feature | Windows | macOS |
|---------|---------|-------|
| Nanite | ✅ Enabled | ❌ Disabled (Metal) |
| Lumen | Hardware RT | Software fallback |
| Shadows | Virtual | Reduced quality |

---

For more details, see the [README.md](README.md) or [Docs/](Docs/) directory.
