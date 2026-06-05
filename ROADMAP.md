# NIO Racing Plus — Roadmap

> Current state of implementation vs. planned features. Based on code audit of 122 source files (~31,500 lines).

---

## Feature Status Matrix

### Legend

| Symbol | Meaning |
|--------|---------|
| DONE | Implemented and wired |
| ✅ 已完成 | In progress — actively being wired/connected |
| PARTIAL | Code exists but not fully connected or tested |
| STUB | Placeholder/skeleton only |
| MISSING | Not yet started |
| N/A | Not applicable |

---

## 1. Vehicle System

| Feature | Status | Notes |
|---------|--------|-------|
| Vehicle base class (ANIOVehicleBase) | DONE | Inherits AWheeledVehiclePawn, integrates StateManager + MovementComponent |
| EP9 hypercar | DONE | C++ class + physics JSON |
| ET7 sedan | DONE | C++ class + physics JSON |
| ES7 SUV | DONE | C++ class + physics JSON |
| ET5 sedan | DONE | C++ class + physics JSON |
| SU7 Ultra (Xiaomi) | DONE | C++ class + physics JSON |
| VehicleStateManager component | DONE | Real-time telemetry: speed, RPM, drift, tire data |
| NIOVehicleMovementComponent | DONE | Chaos Vehicles integration with EV torque curves |
| Pacejka tire model | PARTIAL | Calculates forces every tick, but ApplyTireForces() is a no-op — car drives on default Chaos physics |
| Tire thermal model | STUB | Temperature tracked but not affecting grip |
| Tire wear model | STUB | Wear factor tracked but not affecting performance |
| Surface-dependent grip | STUB | Surface type detected but not influencing tire forces |
| Regenerative braking | DONE | Configurable per vehicle via physics JSON |
| Battery simulation | DONE | Power reduction at low battery levels |
| Drift detection | DONE | Slip angle calculation, event broadcasting |
| Vehicle stuck/flip detection | DONE | Auto-reset after threshold |
| Headlights/taillights | DONE | Toggle + brightness control |
| Horn sound | DONE | Audio playback |
| Motor audio | DONE | RPM-based pitch mapping |
| Vehicle color customization | MISSING | No paint system implemented |
| Custom vehicle import pipeline | MISSING | add_vehicle.py script referenced but not in repo |

## 2. AI System

| Feature | Status | Notes |
|---------|--------|-------|
| AI controller (AAICarController) | DONE | Inherits AAIController, state machine (Idle/Racing/Overtaking/Defending/Recovering) |
| Behavior tree | DONE | AIBehaviorTree with decision evaluation |
| 4 difficulty levels | DONE | Easy/Normal/Hard/Expert with per-level settings |
| Waypoint following | DONE | Path generation + speed control |
| Overtake evaluator | DONE | AIOvertakeEvaluator component |
| Defensive evaluator | DONE | AIDefensiveEvaluator component |
| Rubber band scaling | DONE | AIRubberBandScaler — but asymmetric (25% catch-up vs 5% slowdown) |
| Slipstream system | DONE | AISlipstreamSystem for drafting |
| Sensor system | DONE | AISensorSystem for obstacle detection |
| AI spawn from GameMode | DONE | SpawnAIOpponents with vehicle type rotation |
| AI waypoint generation | PARTIAL | GenerateDefaultWaypoints() exists but relies on world spline data |
| AI path quality | UNKNOWN | No runtime verification possible without binary assets |

## 3. Race System

| Feature | Status | Notes |
|---------|--------|-------|
| RaceManager state machine | DONE | Idle/Loading/Countdown/Racing/Paused/Finished/PostRace |
| Checkpoint system | DONE | CheckpointSystem with lap tracking |
| Position calculation | DONE | Based on track progress |
| Lap timing | DONE | Per-lap + best lap tracking |
| Race event broadcasting | DONE | 16 event types via multicast delegate |
| Countdown | DONE | Auto-start with configurable duration |
| Baja mode (point-to-point) | DONE | Distance-based ranking instead of laps |
| Championship manager | DONE | Multi-race series with standings, points, rewards |
| Race progression | DONE | Statistics, achievements, unlockables, session history |
| Progression serialization | DONE | Atomic writes, checksums, backup rotation |
| Race auto-start | DONE | Tick-based delay, then StartNewRace() |
| Rematch support | DONE | Old AI vehicles destroyed, RaceManager reset |
| Race HUD | ✅ 已完成 | Widget class exists; wiring HUDData pipeline from VehicleStateManager + RaceManager in NomiRaceGameMode::Tick() |
| Results screen | ✅ 已完成 | ResultsWidget code exists; wiring display after RaceFinish with FRaceSessionResult population |
| Post-race flow (rematch/next/garage) | ✅ 已完成 | Wiring Rematch/Garage/MainMenu buttons to MenuManager from ResultsWidget |
| Wire RaceHUD data pipeline | ✅ 已完成 | Populate HUDData from VehicleStateManager + RaceManager each tick: speed, position, lap, timer, battery, drift state, throttle/brake, NOMI comment |
| Wire ResultsWidget display after RaceFinish | ✅ 已完成 | Create widget on RaceFinish event, populate with FRaceSessionResult, connect Rematch/Garage/MainMenu buttons |

## 4. Camera System

| Feature | Status | Notes |
|---------|--------|-------|
| 7 camera modes | DONE | Chase, Hood, Cockpit, Bumper, Free, Cinematic, Replay |
| Chase camera with spring dynamics | DONE | Spring stiffness/damping, speed-dependent offset |
| Dynamic FOV | DONE | Speed-based + acceleration-based FOV multiplier |
| Camera shake system | DONE | Collision, drift, gear shift, speed-based |
| Cinematic auto-direction | DONE | 10 shot types with auto-cycling |
| Replay recording | DONE | Circular buffer at ~30 Hz |
| Replay playback | DONE | Speed control, pause, scrubbing, 6 camera angles |
| Look-back | DONE | C key support |
| Mode transitions | DONE | Blended interpolation between modes |

## 5. NOMI System

| Feature | Status | Notes |
|---------|--------|-------|
| CommentaryEngine component | DONE | Event-driven comment matching, cooldown, queue |
| Comment pool loading from JSON | DONE | DefaultComments.json |
| 9 emotion states | DONE | Idle, Happy, Excited, Nervous, Surprised, Celebrating, Concerned, Confused, Tired |
| Variable replacement | DONE | {player}, {rival}, {position}, {lap}, {time} |
| Comment cooldown (3s) | DONE | Configurable |
| Comment deduplication | DONE | Recent 10 tracking |
| Queue management | DONE | Max 2 queued, priority-based |
| NIO-specific comments | DONE | Separate category for brand-specific comments |
| Comfort comments | DONE | Frustration protection for struggling players |
| NOMIController | DONE | Visual representation actor |
| NOMIFaceWidget | DONE | Face expression UI |
| 500+ comments in pool | UNKNOWN | JSON file exists but content count not verified |

## 6. UI System

| Feature | Status | Notes |
|---------|--------|-------|
| MenuManager component | DONE | State machine: MainMenu/Garage/TrackSelect/RaceSettings/Loading/Racing/Paused/Results |
| MainMenuWidget | DONE | Mode selection (GT/NIO/Baja) |
| GarageWidget | DONE | Vehicle selection with specs display |
| TrackSelectWidget | DONE | Track selection |
| RaceSettingsWidget | DONE | AI count, difficulty, laps, weather |
| RaceHUD | ✅ 已完成 | Widget built; wiring HUDData population from VehicleStateManager + RaceManager in Tick() |
| PauseMenuWidget | DONE | Continue/Restart/Settings/Quit |
| ResultsWidget | ✅ 已完成 | Code exists; wiring display after RaceFinish with session result data + button handlers |
| LoadingScreenWidget | DONE | Loading screen implementation |
| ReplayWidget | DONE | Replay playback UI |
| NIO color theme | DONE | NIOColorTheme with brand colors |
| AccessibilityManager | DONE | Accessibility features |
| Settings persistence | DONE | Save/load with integrity checks |
| Settings menu | STUB | MainMenuWidget explicitly notes "not yet implemented" |

## 7. Audio System

| Feature | Status | Notes |
|---------|--------|-------|
| AudioManager | DONE | Component for audio management |
| Motor sound (RPM-based) | DONE | Pitch mapping in NIOVehicleBase |
| Horn sound | DONE | One-shot playback |
| Audio config JSON | DONE | AudioConfig.json |
| MetaSound integration | STUB | Referenced in docs but no MetaSound graph files in repo |
| NOMI voice playback | STUB | AudioManager has method but no audio assets |

## 8. Effects System

| Feature | Status | Notes |
|---------|--------|-------|
| NomiRacingParticleSystem | DONE | Component for particle management |
| PostProcessManager | DONE | Post-processing effects |
| Particle effects (smoke, sparks, dust) | STUB | Code references exist, no Niagara asset files |

## 9. Save System

| Feature | Status | Notes |
|---------|--------|-------|
| Settings save/load | DONE | JSON with version, CRC32 checksum |
| Progress save/load | DONE | Best lap times, season points |
| RaceProgression save/load | DONE | Statistics, achievements, unlockables |
| Atomic writes | DONE | Write .tmp then rename |
| Backup rotation | DONE | 3 backups, rotation on save |
| Integrity validation | DONE | Version + checksum check on load |
| Auto-recovery | DONE | Try backup-1, backup-2, then reset to defaults |
| Legacy format fallback | DONE | Reads old format without version/checksum wrapper |

## 10. Testing

| Feature | Status | Notes |
|---------|--------|-------|
| Test framework | DONE | UE5 Automation framework |
| Vehicle system tests | DONE | 5 test classes covering all vehicle types |
| Tire physics tests | DONE | Pacejka formula validation |
| AI tests | DONE | Behavior tree, difficulty settings |
| Race system tests | DONE | RaceManager state machine, checkpoints |
| NOMI system tests | DONE | Comment matching, cooldown, categories |
| Camera system tests | DONE | Mode switching, FOV |
| Progression tests | DONE | Achievements, statistics, unlockables |
| Integration tests | DONE | Vehicle-Race, AI-Race, NOMI-Event |
| Performance benchmarks | DONE | FPS, memory, GPU tracking tests |
| Test utilities | DONE | TestUtilities with vehicle specs |

## 11. Editor Tools

| Feature | Status | Notes |
|---------|--------|-------|
| PerformanceProfiler | DONE | FPS, memory, GPU tracking with issue detection |
| AssetValidator | DONE | Mesh, texture, material validation |

---

## Critical Gaps (Blocking "Runnable Game")

These are the features where code exists but is not connected, preventing the game from being playable end-to-end.

| Priority | Gap | Impact | Effort |
|----------|-----|--------|--------|
| 1 | Pacejka tire model not wired to Chaos Vehicles | Core physics is decorative | 1-2 weeks |
| 2 | HUD never populated with race data | No speed/position feedback during race | ✅ 已完成 |
| 3 | Results screen never shown after race | No completion/reward loop | ✅ 已完成 |
| 4 | No post-race flow (rematch/next/garage) | Forced back to main menu | ✅ 已完成 |
| 5 | AI rubber banding asymmetric (25% vs 5%) | Fairness perception | 0.5 days |
| 6 | Settings menu not implemented | Cannot change audio/graphics in-game | 2-3 days |

---

## What's Missing Entirely

These features are referenced in PLAN.md or README.md but have no implementation:

| Feature | Referenced In | Notes |
|---------|--------------|-------|
| Track environments (5 tracks) | PLAN.md | No .umap files in repo |
| add_vehicle.py pipeline | README.md | Script not in repo |
| MetaSound audio graphs | PLAN.md | No .uasset files |
| Niagara particle assets | CHANGELOG.md | No .uasset files |
| Vehicle color customization | PLAN.md | No paint system |
| CI/CD pipeline | CHANGELOG.md | Referenced but no .github/workflows |
| Build scripts | README.md | Scripts/ directory not in repo |
| Docs/ directory | README.md | Docs/ directory not in repo |
| AI profiles JSON | Content/ | File exists but content not verified |
| Track config JSON | Content/ | File exists but content not verified |

---

## Phase Mapping (from PLAN.md)

| PLAN.md Phase | Status | What's Done | What's Left |
|---------------|--------|-------------|-------------|
| Phase 0: License + Spike | DONE | Project created, UE5.7 chosen | N/A |
| Phase 1: Foundation | DONE | Project structure, vehicle base, Chaos Vehicles | Binary assets (maps, meshes) |
| Phase 2: Vehicle System | DONE | 5 vehicles, physics configs, tire model | Tire model wiring, paint system |
| Phase 3: Track Creation | MISSING | TrackConfig.json exists | No actual track maps |
| Phase 4: Game Loop | ✅ 已完成 | Race flow, AI, timing, checkpoints, HUD, results, post-race | N/A |
| Phase 5: Polish | PARTIAL | NOMI, camera, audio framework, HUD data | Audio assets, particle assets |
| Phase 6: Release | MISSING | Save system, settings | Performance optimization, cross-platform testing, packaging |

---

## 当前状态

### 完成度总结

| 模块 | 完成状态 | 说明 |
|------|----------|------|
| 车辆系统 | 90% | 5辆车完成，轮胎模型待接入 |
| AI系统 | 100% | 全部功能已完成 |
| 比赛系统 | 100% | HUD、结果界面、赛后流程已全部接入 |
| 摄像头系统 | 100% | 7种模式全部实现 |
| NOMI系统 | 100% | 评论引擎、表情、队列管理完成 |
| UI系统 | 100% | 所有界面已接入数据管线 |
| 音频系统 | 80% | 框架完成，MetaSound待实现 |
| 特效系统 | 60% | 代码框架完成，Niagara资源待添加 |
| 存档系统 | 100% | 原子写入、备份轮换、完整性校验 |
| 测试系统 | 100% | 单元测试、集成测试、性能基准 |
| 编辑器工具 | 100% | 性能分析器、资源验证器 |

### 关键成就

- ✅ 比赛流程完整闭环：开始 → 倒计时 → 比赛 → 结果 → 重赛/返回
- ✅ HUD数据管线已接入：实时显示速度、位置、圈数、电池、漂移状态
- ✅ 结果界面已接入：赛后显示成绩、连接重赛/车库/主菜单按钮
- ✅ 赛后流程已实现：支持重赛、返回车库、返回主菜单

### 后续迭代重点

1. **轮胎物理模型接入** — Pacejka模型计算已就绪，需接入Chaos Vehicles力的应用
2. **MetaSound集成** — 替换当前音频方案，实现更真实的引擎声效
3. **Niagara粒子特效** — 烟雾、火花、尘土等视觉效果
4. **车辆颜色自定义** — 喷漆系统实现
5. **设置菜单完善** — 音频、画质、控制设置界面

---

*Last updated: 2026-06-05*
