# NIO Racing Plus - Architecture

> Current system architecture as of 2026-06-05.
> 124 source files, ~32,500 lines of C++.

---

## Directory Layout

```
NomiRacingPlus/Source/NomiRacingPlus/
├── Core/                    Game lifecycle, camera, audio, input
│   ├── NomiGameInstance     Global state, settings, save/load
│   ├── NomiRaceGameMode     Race orchestration, vehicle spawning, HUD pipeline
│   ├── NomiPlayerController Input handling, pause
│   ├── CameraSystem         7 camera modes + replay (split across 4 .cpp files)
│   ├── AudioManager         Sound management
│   ├── NomiRacingParticleSystem  Particle effects
│   └── PostProcessManager   Visual effects
│
├── Vehicles/                Vehicle physics and state
│   ├── NIOVehicleBase       Abstract base (AWheeledVehiclePawn)
│   ├── NIO_EP9/ET7/ES7/ET5 Concrete NIO vehicles
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
│   └── ProgressionSerializer  Save/load for progression
│
├── AI/                      AI opponent behavior
│   ├── AICarController      Main AI controller (state machine)
│   ├── AIBehaviorTree       Decision evaluation
│   ├── AISensorSystem       Obstacle detection
│   ├── AIOvertakeEvaluator  Overtake decisions
│   ├── AIDefensiveEvaluator Defensive decisions
│   ├── AIRubberBandScaler   Difficulty scaling
│   └── AISlipstreamSystem   Drafting mechanics
│
├── NOMI/                    AI companion
│   ├── CommentaryEngine     Comment matching engine
│   ├── NOMIController       Visual representation actor
│   └── NOMIFaceWidget       Face expression UI
│
├── UI/                      User interface
│   ├── MenuManager          Menu state machine with StateStack
│   ├── MainMenuWidget       Mode selection (GT/NIO/Baja)
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
└── Tests/                   26 test files
```

---

## Layer Dependencies

```
NomiGameInstance (global state, settings, save)
       |
NomiRaceGameMode (orchestration hub)
       |
  +---------+-----------+----------------+
  |         |           |                |
RaceManager  CameraSystem  NOMIController  MenuManager
  |                          |                |
CheckpointSystem        CommentaryEngine   Widgets (9)
  |
ChampionshipManager
  |
RaceProgression
  |
ProgressionSerializer


NIOVehicleBase <--- VehicleStateManager (telemetry component)
     |                    |
  5 concrete         NIOVehicleMovementComponent
  vehicles                |
                    TirePhysicsModel (Pacejka)


AICarController <--- AIBehaviorTree (decision logic)
     |                    |
  Overtake/          AISensorSystem
  Defensive/         (obstacle detection)
  RubberBand
  Evaluators
```

---

## Key Data Structures

### FNIOVehicleState

Central telemetry updated every tick. Consumed by HUD, NOMI, RaceManager.

```
FNIOVehicleState
├── SpeedKmh, ThrottleInput, BrakeInput, SteeringInput
├── RPM, Gear (fixed 1 for EV)
├── bIsDrifting, SlipAngle
├── bIsGrounded, Position, Rotation, Velocity
├── BatteryPercent, bIsMotorActive
└── Tire: AvgTemperature, AvgWear, bAnySlipping, MaxSlipRatio, Surface
```

### FRacerData

Per-racer state managed by RaceManager.

```
FRacerData
├── RacerID, DisplayName, bIsPlayer
├── CurrentLap, CurrentCheckpoint, TotalCheckpointsPassed
├── Position (1-based), LapTimes[], BestLapTime, TotalRaceTime
├── bFinished, TrackProgress (0-1)
└── VehiclePawn (APawn*)
```

### FCommentContext

Context for NOMI comment matching.

```
FCommentContext
├── Event (ERaceEvent), PlayerName, RivalName
├── Position, CurrentLap, LapTime
├── DriftDuration, Speed, CornerName
└── bIsNIOVehicle, NIOVehicleType
```

### FNomiGameSettings

Persisted settings with atomic save/load.

```
FNomiGameSettings
├── Graphics: Quality, Nanite, Lumen, MotionBlur, Preset
├── Audio: Master/SFX/Music Volume
├── Gameplay: NOMIFreq, Vehicle, Track, Difficulty, Laps, AI, GameMode
└── Progress: BestLapTimes, SeasonPoints
```

---

## Event Flow

### Race Events

```
VehicleStateManager.Tick()
  └── Detects: drift, high speed, stuck, flipped
       |
RaceManager.Tick()
  └── Updates positions, timers, laps
  └── Detects: overtake, lap complete, fastest lap
       |
OnRaceEvent multicast delegate
  ├─> NomiRaceGameMode.OnRaceEvent()
  │     ├─ RaceStart: enable input, create HUD, attach NOMI
  │     ├─ RaceFinish: record session, show results
  │     └─ ForwardEventToNOMI()
  │           └─ CommentaryEngine.RequestComment(Context)
  │               ├─ Match from pool
  │               ├─ Cooldown check (3s)
  │               ├─ Dedup check (recent 10)
  │               └─ Queue for display
  │
  └─> NomiRaceGameMode.Tick() [HUD pipeline]
        └─ Populate FHUDData from VSM + RaceManager + CommentaryEngine
        └─ Update RaceHUD widget each frame
```

### Menu Flow

```
MainMenu -> Garage -> TrackSelect -> RaceSettings -> Loading -> Racing
                                                            |
                                                       PauseMenu (ESC)
                                                            |
                                                       Results (finish)
                                                            |
                       <--- back navigation via StateStack ---
```

### Save Flow

```
SaveSettings()
  ├─ Build JSON, serialize with sorted keys
  ├─ CRC32 checksum
  ├─ Wrap in {version, checksum, data}
  ├─ RotateBackups() (3 backups)
  └─ AtomicWriteSave() (.tmp then rename)

LoadSettings()
  ├─ ValidateSaveIntegrity() (version + checksum)
  │   └─ If invalid: RecoverFromBackup() chain
  ├─ Deserialize JSON
  └─ Legacy format fallback
```

---

## Design Decisions

| Decision | Rationale |
|----------|-----------|
| Components over inheritance | VehicleStateManager, CameraSystem, etc. as UActorComponent for mix/match and testability |
| Event-driven communication | RaceManager multicast delegate. No direct coupling between race logic and UI/NOMI. |
| JSON configuration | Vehicle physics, AI profiles, NOMI comments in JSON. Edit without recompile. |
| Atomic saves | Write .tmp then rename. CRC32 checksums. 3-backup rotation. Prevents corruption. |
| C++ over Blueprints | All game logic in C++. Blueprints only for widget layout, mesh assignment, level design. |
| Chaos Vehicles + custom layer | NIOVehicleMovementComponent wraps UChaosWheeledVehicleMovementComponent with EV torque. |

---

## Log Categories

| Category | Purpose |
|----------|---------|
| LogNomiRacing | General game flow |
| LogNomiRace | Race state, timing, positions |
| LogNomiVehicle | Vehicle physics, tire model |
| LogNomiNOMI | Commentary engine |
| LogNomiAI | AI behavior, decisions |
| LogNomiCamera | Camera modes, transitions |
| LogNomiPerf | Performance profiling |

---

## Content Files

| File | Purpose |
|------|---------|
| Vehicles/VehicleConfig.json | All vehicle physics params |
| Vehicles/EP9_Physics.json | EP9-specific physics |
| Vehicles/ET7_Physics.json | ET7-specific physics |
| Vehicles/ES7_Physics.json | ES7-specific physics |
| Vehicles/ET5_Physics.json | ET5-specific physics |
| Vehicles/SU7Ultra_Physics.json | SU7 Ultra physics |
| AI/AIProfiles.json | AI difficulty settings |
| NOMI/Comments/DefaultComments.json | NOMI comment pool |
| Maps/TrackConfig.json | Track configuration |
| Audio/AudioConfig.json | Audio settings |
| UI/UITheme.json | UI color theme |

---

## Cross-Platform

| Feature | Windows | macOS |
|---------|---------|-------|
| Nanite | Enabled (DX12/Vulkan) | Disabled (Metal) |
| Lumen | Hardware RT | Software fallback |
| Shadows | Virtual shadow maps | Reduced quality |
| Build | VS 2022 | Xcode 14+ |
| Target FPS | 60 @ 1080p Medium | 45 @ 1800p Medium |

Runtime detection via `PLATFORM_MAC`. Graphics presets auto-disable Nanite/Lumen on macOS.

---

## Constants (NomiRacingPlus.h)

| Constant | Value | Purpose |
|----------|-------|---------|
| NIOBlueHex | #00A1E0 | Brand primary |
| NIOCyanHex | #00D4FF | Brand secondary |
| NIOBackgroundHex | #0A0E1A | UI background |
| DefaultGravity | 980.0 cm/s^2 | Physics |
| AirDensity | 1.225 kg/m^3 | Aerodynamics |
| MaxLaps | 20 | Race limit |
| MaxAIOpponents | 7 | AI limit |
| CountdownDuration | 5.0s | Race start |
| NOMICommentCooldown | 3.0s | Comment frequency |
| NOMIMaxQueueSize | 2 | Queue depth |
| NOMIRecentCommentsMax | 10 | Dedup window |
