# NIO Racing Plus — Architecture

> Current system architecture as of 2026-06-05. Based on code audit of 122 source files.

---

## Module Map

```
NomiRacingPlus/Source/NomiRacingPlus/
├── Core/                    Game lifecycle, camera, audio, input
│   ├── NomiGameInstance     Global state, settings, save/load
│   ├── NomiRaceGameMode     Race orchestration, vehicle spawning
│   ├── NomiPlayerController Input handling, pause
│   ├── CameraSystem         7 camera modes + replay
│   ├── AudioManager         Sound management
│   ├── NomiRacingParticleSystem  Particle effects
│   └── PostProcessManager   Visual effects
│
├── Vehicles/                Vehicle physics and state
│   ├── NIOVehicleBase       Abstract base (AWheeledVehiclePawn)
│   ├── NIO_EP9/ET7/ES7/ET5 Concrete vehicle classes
│   ├── Xiaomi_SU7Ultra      Non-NIO vehicle
│   ├── VehicleStateManager  Real-time telemetry component
│   ├── NIOVehicleMovementComponent  Chaos Vehicles integration
│   ├── TirePhysicsModel     Pacejka Magic Formula
│   └── NIOTirePresets       Tire configuration presets
│
├── Race/                    Race logic and progression
│   ├── RaceManager          State machine, timing, positions
│   ├── CheckpointSystem     Lap tracking
│   ├── ChampionshipManager  Multi-race series
│   ├── RaceProgression      Achievements, stats, unlockables
│   ├── ProgressionSerializer  Save/load for progression
│   └── ChampionshipTypes/RaceProgressionTypes  Data structures
│
├── AI/                      AI opponent behavior
│   ├── AICarController      Main AI controller (AAIController)
│   ├── AIBehaviorTree       Decision evaluation
│   ├── AISensorSystem       Obstacle detection
│   ├── AIOvertakeEvaluator  Overtake decisions
│   ├── AIDefensiveEvaluator Defensive decisions
│   ├── AIRubberBandScaler   Difficulty scaling
│   └── AISlipstreamSystem   Drafting mechanics
│
├── NOMI/                    AI companion system
│   ├── CommentaryEngine     Comment matching engine
│   ├── NOMIController       Visual representation actor
│   └── NOMIFaceWidget       Face expression UI
│
├── UI/                      User interface
│   ├── MenuManager          Menu state machine
│   ├── MainMenuWidget       Mode selection
│   ├── GarageWidget         Vehicle selection
│   ├── TrackSelectWidget    Track selection
│   ├── RaceSettingsWidget   Race configuration
│   ├── RaceHUD              In-race display
│   ├── PauseMenuWidget      Pause overlay
│   ├── ResultsWidget        Post-race results
│   ├── LoadingScreenWidget  Loading screen
│   ├── ReplayWidget         Replay playback UI
│   ├── NIOColorTheme        Brand color system
│   └── AccessibilityManager Accessibility features
│
├── Editor/                  Development tools
│   ├── PerformanceProfiler  FPS/memory/GPU tracking
│   └── AssetValidator       Asset validation
│
└── Tests/                   Test suite (22 files)
    ├── VehicleSystemTest    Vehicle type/config tests
    ├── TirePhysicsTest      Pacejka formula tests
    ├── AITest               AI behavior tests
    ├── RaceSystemTest       RaceManager tests
    ├── NOMISystemTest       Commentary tests
    ├── CameraSystemTest     Camera mode tests
    ├── ProgressionTest      Achievement/stat tests
    ├── VehicleRaceIntegrationTest  Vehicle-Race wiring
    ├── AIRaceManagerIntegrationTest  AI-Race wiring
    ├── NOMIEventIntegrationTest  NOMI-Event wiring
    ├── PerformanceBenchmarkTest  Performance tests
    └── TestUtilities        Shared test helpers
```

---

## Dependency Graph

```
                    NomiGameInstance
                    (global state, settings)
                           │
                    NomiRaceGameMode
                    (orchestration hub)
                    ┌──────┼──────┐
                    │      │      │
              RaceManager  │  NOMIController
              (state machine) │  (visual)
                    │      │      │
            ┌───────┤  CommentaryEngine
            │       │  (comment matching)
      CheckpointSystem       │
      (lap tracking)    NOMIFaceWidget
            │           (expressions)
      ChampionshipManager
      (multi-race series)
            │
      RaceProgression
      (achievements, stats)
            │
      ProgressionSerializer
      (save/load)


      NIOVehicleBase ◄──── VehicleStateManager
      (abstract pawn)      (telemetry component)
            │                     │
      ┌─────┼─────┐         NIOVehicleMovementComponent
      │     │     │         (Chaos Vehicles integration)
    EP9   ET7   ES7               │
    ET5  SU7Ultra            TirePhysicsModel
                             (Pacejka formula)


      AICarController ◄──── AIBehaviorTree
      (AI driving)          (decision logic)
            │                     │
      ┌─────┼─────┐         AISensorSystem
      │     │     │         (obstacle detection)
  Overtake Defensive RubberBand
  Evaluator Evaluator Scaler
            │
      AISlipstreamSystem
      (drafting)


      CameraSystem
      (7 modes + replay)
            │
      ┌─────┼─────┐
      │     │     │
    Chase  Cinematic Replay
    Camera  Camera   System


      MenuManager
      (state machine)
            │
      ┌─────┼─────┬─────────┬──────────┐
      │     │     │         │          │
   MainMenu Garage TrackSelect RaceSettings Results
   Widget   Widget Widget     Widget       Widget
```

---

## Key Data Structures

### FNIOVehicleState (VehicleStateManager.h)

Central telemetry structure updated every tick. Consumed by HUD, NOMI, and RaceManager.

```
FNIOVehicleState
├── SpeedKmh, ThrottleInput, BrakeInput, SteeringInput
├── RPM, Gear (fixed at 1 for EV)
├── bIsDrifting, SlipAngle
├── bIsGrounded, Position, Rotation, Velocity
├── BatteryPercent, bIsMotorActive
└── Tire telemetry: AverageTireTemperature, AverageTireWear,
    bAnyTireSlipping, MaxSlipRatio, MaxSlipAngleDeg, CurrentSurface
```

### FRacerData (RaceManager.h)

Per-racer state managed by RaceManager.

```
FRacerData
├── RacerID, DisplayName, bIsPlayer
├── CurrentLap, CurrentCheckpoint, TotalCheckpointsPassed
├── Position (1-based)
├── LapTimes[], BestLapTime, TotalRaceTime
├── bFinished, TrackProgress (0-1)
└── VehiclePawn (APawn reference)
```

### FRaceConfig (RaceManager.h)

Race configuration passed from MenuManager through GameMode to RaceManager.

```
FRaceConfig
├── NumLaps, MaxAIOpponents
├── TrackName, RaceMode ("StreetGT"/"NIO"/"Baja")
├── CountdownDuration, bAllowCollisions, CollisionPenalty
└── bIsPointToPoint (Baja mode)
```

### FCommentContext (CommentaryEngine.h)

Context passed to CommentaryEngine when requesting a comment.

```
FCommentContext
├── Event (ERaceEvent)
├── PlayerName, RivalName
├── Position, CurrentLap, LapTime
├── DriftDuration, Speed
├── CornerName
└── bIsNIOVehicle, NIOVehicleType
```

### FNomiGameSettings (NomiGameInstance.h)

Persisted settings with atomic save/load.

```
FNomiGameSettings
├── Graphics: GraphicsQuality, bEnableNanite, bEnableLumen, bEnableMotionBlur, CurrentPreset
├── Audio: MasterVolume, SFXVolume, MusicVolume
├── Gameplay: NOMIFrequency, SelectedVehicle, SelectedTrack, Difficulty, NumLaps, NumAIOpponents, GameMode
└── Progress: BestLapTimes (TMap), SeasonPoints
```

---

## Event Flow

### Race Event Pipeline

```
VehicleStateManager.Tick()
    │
    ├── Detects: drift start/end, high speed, stuck, flipped
    │
    ▼
RaceManager.Tick()
    │
    ├── Updates positions, timers, lap counts
    ├── Detects: overtake, overtaken, lap complete, fastest lap
    │
    ▼
RaceManager.OnRaceEvent delegate (multicast)
    │
    ├──► NomiRaceGameMode.OnRaceEvent()
    │       ├── Handle RaceStart: enable input, create HUD, attach NOMI
    │       ├── Handle RaceFinish: record session, update progression
    │       ├── Handle LapComplete: log
    │       └── ForwardEventToNOMI()
    │               │
    │               ▼
    │           CommentaryEngine.RequestComment(Context)
    │               ├── Find matching comment from pool
    │               ├── Check cooldown (3s)
    │               ├── Check dedup (recent 10)
    │               └── Queue comment for display
    │
    └──► [HUD binding — NOT YET WIRED]
```

### Menu Flow

```
MenuManager.Initialize(PC)
    │
    ▼
MainMenu ──► Garage ──► TrackSelect ──► RaceSettings ──► Loading ──► Racing
    │                                                            │
    │                                                      PauseMenu (ESC)
    │                                                            │
    │                                                      Results (finish)
    │                                                            │
    └────────────────────────────────────────────────────────────┘
                          (back navigation via StateStack)
```

### Save/Load Flow

```
NomiGameInstance.SaveSettings()
    │
    ├── Build JSON object with all settings
    ├── Serialize with sorted keys (deterministic)
    ├── Calculate CRC32 checksum
    ├── Wrap in {version, checksum, data}
    ├── RotateBackups() — shift save -> backup-1 -> backup-2
    └── AtomicWriteSave() — write .tmp, then rename

NomiGameInstance.LoadSettings()
    │
    ├── Check file exists
    ├── ValidateSaveIntegrity() — version + checksum
    │   ├── If invalid: RecoverFromBackup() — try backup-1, backup-2
    │   └── If no valid backup: reset to defaults
    ├── Deserialize JSON
    └── Extract data (wrapped format or legacy fallback)
```

---

## Design Decisions

### 1. Component Architecture over Inheritance

VehicleStateManager, CameraSystem, CommentaryEngine, MenuManager are all UActorComponent subclasses attached to actors rather than inherited. This allows:

- Mixing and matching components across different actor types
- Testing components in isolation
- Adding features without modifying base classes

### 2. Event-Driven Communication

RaceManager broadcasts events via FOnRaceEvent multicast delegate. Subscribers (GameMode, CommentaryEngine, HUD) bind independently. No direct coupling between race logic and UI/NOMI.

### 3. JSON Configuration

Vehicle physics, AI profiles, NOMI comments, and UI themes are all JSON files in Content/. This allows:

- Editing without recompiling
- Easy version control of game balance
- Potential modding support

### 4. Atomic Save System

All save operations follow: build JSON -> compute checksum -> write .tmp -> rename to final. This prevents corruption from crashes mid-write. 3-backup rotation provides recovery from corrupted saves.

### 5. C++ Over Blueprints

All game logic is in C++. Blueprints are used only for:

- Widget layout (UMG)
- Vehicle mesh assignment (CDO)
- Level design (maps)

This provides better performance, easier debugging, and version control friendliness.

### 6. Chaos Vehicles with Custom Physics Layer

NIOVehicleMovementComponent wraps UChaosWheeledVehicleMovementComponent with EV-specific torque curves. TirePhysicsModel provides Pacejka calculations on top. The intent is to feed Pacejka forces back into Chaos, but this connection is not yet implemented.

---

## Log Categories

| Category | Purpose |
|----------|---------|
| LogNomiRacing | General game flow |
| LogNomiRace | Race state, timing, positions |
| LogNomiVehicle | Vehicle physics, tire model |
| LogNomiNOMI | Commentary engine, comments |
| LogNomiAI | AI behavior, decisions |
| LogNomiCamera | Camera modes, transitions |
| LogNomiPerf | Performance profiling |

---

## Constants (NomiRacingPlus.h)

| Constant | Value | Purpose |
|----------|-------|---------|
| NIOBlueHex | #00A1E0 | Brand primary color |
| NIOCyanHex | #00D4FF | Brand secondary color |
| NIOBackgroundHex | #0A0E1A | UI background |
| DefaultGravity | 980.0 cm/s^2 | Physics gravity |
| AirDensity | 1.225 kg/m^3 | Aerodynamics |
| MaxLaps | 20 | Race limit |
| MaxAIOpponents | 7 | AI limit |
| CountdownDuration | 5.0s | Race start delay |
| NOMICommentCooldown | 3.0s | Comment frequency limit |
| NOMIMaxQueueSize | 2 | Comment queue depth |
| NOMIRecentCommentsMax | 10 | Dedup window |

---

## Content Files

| File | Purpose | Format |
|------|---------|--------|
| Vehicles/VehicleConfig.json | All 5 vehicle physics parameters | JSON |
| Vehicles/EP9_Physics.json | EP9-specific physics | JSON |
| Vehicles/ET7_Physics.json | ET7-specific physics | JSON |
| Vehicles/ES7_Physics.json | ES7-specific physics | JSON |
| Vehicles/ET5_Physics.json | ET5-specific physics | JSON |
| Vehicles/SU7Ultra_Physics.json | SU7 Ultra physics | JSON |
| AI/AIProfiles.json | AI difficulty settings | JSON |
| NOMI/Comments/DefaultComments.json | NOMI comment pool | JSON |
| Maps/TrackConfig.json | Track configuration | JSON |
| Audio/AudioConfig.json | Audio settings | JSON |
| UI/UITheme.json | UI color theme | JSON |

---

## Cross-Platform Strategy

| Feature | Windows | macOS |
|---------|---------|-------|
| Nanite | Enabled (DX12/Vulkan) | Disabled (Metal limitation) |
| Lumen | Hardware RT | Software fallback |
| Shadows | Virtual shadow maps | Reduced quality |
| Build | Visual Studio 2022 | Xcode 14+ |
| Target FPS | 60 FPS @ 1080p Medium | 45 FPS @ 1800p Medium |

Platform detection at runtime via `PLATFORM_MAC` preprocessor. Graphics presets automatically disable Nanite/Lumen on macOS.

---

*Last updated: 2026-06-05*
