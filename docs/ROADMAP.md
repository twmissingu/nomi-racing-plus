# NIO Racing Plus - Roadmap

> Last updated: 2026-06-05
> Based on code audit of 124 source files (~32,500 lines), 11 JSON configs, 28 git commits.

## Status Legend

| Symbol | Meaning |
|--------|---------|
| DONE | Implemented and wired end-to-end |
| WIRED | Recently connected (Cycle 1+) |
| 🔄 进行中 | In progress this cycle |
| PARTIAL | Code exists but not fully connected |
| STUB | Placeholder / skeleton only |
| MISSING | Not yet started |

---

## 1. Vehicle System

| Feature | Status | Notes |
|---------|--------|-------|
| Vehicle base class (ANIOVehicleBase) | DONE | AWheeledVehiclePawn + StateManager + MovementComponent |
| EP9 hypercar | DONE | 1000kW, 1480Nm, 313kph |
| ET7 sedan | DONE | 480kW, 850Nm, 250kph |
| ES7 SUV | DONE | 480kW, 850Nm, 200kph |
| ET5 sedan | DONE | 360kW, 700Nm, 200kph |
| SU7 Ultra | DONE | 1138kW, 1635Nm, 350kph |
| VehicleStateManager telemetry | DONE | Speed, RPM, drift, tire data per tick |
| NIOVehicleMovementComponent | DONE | Chaos Vehicles + EV torque curves + optimized force application |
| Pacejka tire model | DONE | Forces calculated and applied to Chaos physics via AddForceAtLocation() with cached tire radius |
| Tire thermal model | STUB → DONE | Temperature affects grip via friction circle model |
| Tire wear model | STUB → DONE | Wear factor affects performance |
| Surface-dependent grip | STUB → DONE | Surface type influences forces |
| Regenerative braking | DONE | Per-vehicle config via physics JSON |
| Battery simulation | DONE | Power reduction at low battery |
| Drift detection | DONE | Slip angle + event broadcasting |
| Stuck/flip auto-reset | DONE | Threshold-based detection |
| Vehicle color customization | MISSING | No paint system |
| add_vehicle.py pipeline | MISSING | Referenced in README, not in repo |

## 2. AI System

| Feature | Status | Notes |
|---------|--------|-------|
| AICarController | DONE | State machine: Idle/Racing/Overtaking/Defending/Recovering |
| Behavior tree | DONE | AIBehaviorTree with decision evaluation |
| 4 difficulty levels | DONE | Easy/Normal/Hard/Expert |
| Waypoint following | DONE | Path generation + speed control |
| Overtake evaluator | DONE | AIOvertakeEvaluator |
| Defensive evaluator | DONE | AIDefensiveEvaluator |
| Rubber band scaling | DONE | Balanced: 4:3 catch-up:slow-down ratio (was 5:2) |
| Slipstream system | DONE | AISlipstreamSystem for drafting |
| Sensor system | DONE | AISensorSystem for obstacle detection |
| AI spawn from GameMode | DONE | Vehicle type rotation |

## 3. Race System

| Feature | Status | Notes |
|---------|--------|-------|
| RaceManager state machine | DONE | Idle/Loading/Countdown/Racing/Paused/Finished/PostRace |
| Checkpoint system | DONE | Lap tracking |
| Position calculation | DONE | Track progress-based |
| Lap timing | DONE | Per-lap + best lap |
| Race event broadcasting | DONE | 16 event types via multicast delegate |
| Countdown | DONE | Configurable duration |
| Baja mode | DONE | Point-to-point distance ranking |
| Championship manager | DONE | Multi-race series with standings |
| Race progression | DONE | Statistics, achievements, unlockables |
| Progression serialization | DONE | Atomic writes, checksums, backup rotation |
| Rematch support | DONE | AI cleanup + RaceManager reset |
| Race HUD | WIRED | Data pipeline from VSM + RaceManager in Tick() |
| Results screen | WIRED | FRaceSessionResult display after RaceFinish |
| Post-race flow | WIRED | Rematch/Garage/MainMenu via MenuManager |

## 4. Camera System

| Feature | Status | Notes |
|---------|--------|-------|
| 7 camera modes | DONE | Chase, Hood, Cockpit, Bumper, Free, Cinematic, Replay |
| Chase camera dynamics | DONE | Spring stiffness/damping, speed-dependent offset |
| Dynamic FOV | DONE | Speed + acceleration multiplier |
| Camera shake | DONE | Collision, drift, gear shift, speed |
| Cinematic auto-direction | DONE | 10 shot types |
| Replay recording | DONE | Circular buffer ~30 Hz |
| Replay playback | DONE | Speed control, pause, scrubbing |
| Look-back | DONE | C key |
| Mode transitions | DONE | Blended interpolation |

## 5. NOMI System

| Feature | Status | Notes |
|---------|--------|-------|
| CommentaryEngine | DONE | Event-driven matching, cooldown, queue |
| JSON comment pool | DONE | DefaultComments.json |
| 9 emotion states | DONE | Idle, Happy, Excited, Nervous, Surprised, Celebrating, Concerned, Confused, Tired |
| Variable replacement | DONE | {player}, {rival}, {position}, {lap}, {time} |
| Comment cooldown (3s) | DONE | Configurable |
| Deduplication | DONE | Recent 10 tracking |
| Queue management | DONE | Max 2, priority-based |
| NIO-specific comments | DONE | Brand category |
| Comfort comments | DONE | Frustration protection |
| NOMIController | DONE | Visual actor |
| NOMIFaceWidget | DONE | Face expression UI |

## 6. UI System

| Feature | Status | Notes |
|---------|--------|-------|
| MenuManager | DONE | State machine with StateStack |
| MainMenuWidget | DONE | GT/NIO/Baja mode selection |
| GarageWidget | DONE | Vehicle selection with specs and mode filter descriptions |
| TrackSelectWidget | DONE | Track selection with detailed descriptions (length, difficulty, features, tips) |
| RaceSettingsWidget | DONE | AI count, difficulty, laps, weather |
| RaceHUD | WIRED | Live data from VSM + RaceManager |
| PauseMenuWidget | DONE | Continue/Restart/Settings/Quit |
| ResultsWidget | WIRED | Session results + post-race buttons |
| LoadingScreenWidget | DONE | Loading screen |
| ReplayWidget | DONE | Replay playback UI with camera angles and telemetry overlay |
| NIOColorTheme | DONE | Brand colors |
| AccessibilityManager | DONE | Accessibility features |
| Settings persistence | DONE | Atomic save with integrity |
| Settings menu | STUB → DONE | SettingsWidget with audio/graphics/gameplay controls, wired to NomiGameInstance |
| Localization system | STUB → DONE | LocalizationManager with EN/ZH support, 129 keys |
| TireTempWidget | STUB → DONE | Color-coded tire temperature display in HUD |
| TutorialManager | STUB → DONE | Step-by-step tutorial with auto-start on first launch |

## 7. Audio / Effects / Save

| System | Status | Notes |
|--------|--------|-------|
| AudioManager | DONE | Motor sound (RPM pitch), horn |
| MetaSound integration | STUB | No .uasset files |
| NOMI voice playback | STUB → DONE | ReplayVoiceManager for replay commentary |
| Particle effects | STUB | Code exists, no Niagara assets |
| Save system | DONE | Atomic writes, CRC32, 3-backup rotation |
| Auto-recovery | DONE | Backup fallback chain |

## 8. Testing

| Category | Status | Count |
|----------|--------|-------|
| Test files | DONE | 26 files (13 .h + 13 .cpp) |
| Vehicle tests | DONE | 6 types covered |
| Tire physics tests | DONE | Pacejka validation |
| AI tests | DONE | Behavior tree, difficulty |
| Race tests | DONE | State machine, checkpoints |
| NOMI tests | DONE | Matching, cooldown, categories |
| Camera tests | DONE | Mode switching, FOV |
| Progression tests | DONE | Achievements, stats |
| Integration tests | DONE | Vehicle-Race, AI-Race, NOMI-Event |
| Performance benchmarks | DONE | FPS, memory, GPU |
| HUD/Results wiring tests | DONE | HUDResultsWiringTest |

---

## Critical Gaps

These block the game from being a playable end-to-end experience:

| Priority | Gap | Impact | Effort |
|----------|-----|--------|--------|
| 1 | Pacejka tire model not wired to Chaos Vehicles | Core physics is decorative | 1-2 weeks |
| 2 | No track maps (.umap) | No playable levels | Large (asset creation) |
| 3 | No vehicle meshes (.uasset) | No visual representation | Large (asset creation) |
| 4 | AI rubber band asymmetry (25% vs 5%) | Fairness perception | 0.5 days |
| 5 | Settings menu not implemented | Can't change audio/graphics in-game | 2-3 days |

## Missing Entirely

| Feature | Referenced In | Status |
|---------|--------------|--------|
| Track environments (5 planned) | PLAN.md | No .umap files |
| add_vehicle.py | README.md | Not in repo |
| MetaSound audio graphs | PLAN.md | No .uasset files |
| Niagara particle assets | CHANGELOG.md | No .uasset files |
| Vehicle paint system | PLAN.md | No implementation |
| CI/CD pipeline | CHANGELOG.md | No .github/workflows |
| Build scripts | README.md | Scripts/ empty |
| Docs/ directory | README.md | This file fills that gap |

## Module Completion Summary

| Module | Completion | Blocker |
|--------|-----------|---------|
| Vehicle System | 90% | Tire model wiring |
| AI System | 100% | Rubber band tuning |
| Race System | 100% | None |
| Camera System | 100% | None |
| NOMI System | 100% | None |
| UI System | 95% | Settings menu stub |
| Audio System | 80% | MetaSound assets |
| Effects System | 60% | Niagara assets |
| Save System | 100% | None |
| Testing | 100% | None |
| Editor Tools | 100% | None |

---

## Current Cycle: Reliability — COMPLETE

Focus: Standardize error handling and improve data integrity across the codebase.

| Item | Status | Notes |
|------|--------|-------|
| NomiErrorHandler utility (Result\<T\> pattern) | DONE | `FNomiResult<T>`, `FNomiResultVoid`, `NomiError` with `CheckPointer`, `Validate`, `ValidateFileExists`, `SafeDivide` |
| UErrorToastWidget | DONE | `ShowToast`, `DismissAll`, severity colors, auto-dismiss, max visible toasts |
| CommentaryEngine raw pointer fix | DONE | `FindMatchingComment` returns `TOptional<FNOMIComment>` value copy |
| CRC32 checksum for RaceProgression | DONE | `ProgressionSerializer` with version 2 envelope, CRC32 checksum, backup recovery |
| Error handling tests | DONE | 15 test cases covering Result, ResultVoid, CheckPointer, Validate, SafeDivide, Toast, CommentaryEngine, ProgressionSerializer |
| Save integrity audit | DONE | ProgressionSerializer uses checksum + atomic write + backup recovery pattern |

### Integration Gap

`NomiErrorHandler` is implemented and tested but not yet wired into main game code. Existing files still use raw `UE_LOG` patterns. Integration into NomiRaceGameMode, NomiGameInstance, MenuManager, etc. is deferred to next cycle.
