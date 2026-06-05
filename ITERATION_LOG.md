# NIO Racing Plus — Iteration Log

> Autonomous development cycle journal. Each entry documents findings, changes, and UX improvements.

---

## Baseline Assessment (Pre-Workflow)

**Date:** 2026-06-05
**Codebase State:** ~60-70% complete
**Source Files:** 123 (.h + .cpp)
**Test Files:** 22 (5,165 lines)
**Documentation:** 14 files
**Config Files:** 10 JSON + 5 INI

### Known Issues
- Settings menu explicitly "not yet implemented" (MainMenuWidget.cpp:74)
- No CI/CD builds have ever succeeded
- Systems have wiring issues (evidenced by 10+ "fix:" commits)
- Binary assets (.uasset) not in repo — code-level verification only
- No GitHub issues or PRs ever created

### Blockers for "Runnable Game"
1. Menu → Race flow may not wire correctly
2. Vehicle spawning + AI initialization untested at runtime
3. Camera system transitions between modes
4. HUD updates during race
5. Results screen → replay/next race flow

---

## Cycle 1 — Code Quality, Test Coverage, and UX Analysis

**Date:** 2026-06-05
**Phase:** Audit -> Develop -> Test -> Review -> Iterate

### Audit Findings

**Documentation-Code Consistency Audit:** Found 16 inconsistencies across 13 documentation files and ~90 source files. 8 critical/high, 10 medium.

| Severity | Count | Key Findings |
|----------|-------|-------------|
| CRITICAL | 2 | UE5 engine version mismatch (.uproject says 5.7, docs say 5.5); SU7 Ultra vehicle exists in code/config but is undocumented |
| HIGH | 6 | Vehicle enum safety, brace nesting issues, variable shadowing, bulk removal bugs |
| MEDIUM | 10 | Missing default cases in switches, inconsistent null-checking patterns |
| LOW | 4 | Duplicate includes, minor style inconsistencies |

**Code Quality Audit (16 findings):**
- 0 stale code issues
- 16 inconsistencies between documentation and implementation

**Security Audit:** No new security issues introduced by Cycle 1 changes. 3 pre-existing issues noted:
1. RaceProgression save lacks checksum protection (unlike NomiGameInstance saves)
2. Inconsistent input validation at system boundaries
3. No rate limiting on event broadcasting

### Changes Made

| File | Description |
|------|-------------|
| `VehicleStateManager.cpp` | Fixed ResetVehicle logic bug, removed duplicate GameplayStatics.h include |
| `CommentaryEngine.cpp` | Fixed bulk removal in AddToRecentComments with proper bIsReady guard |
| `AIBehaviorTree.cpp` | Fixed variable shadowing in difficulty evaluation |
| `AICarController.cpp` | Fixed brace nesting mismatch in behavior tree execution |
| `NomiRaceGameMode.cpp` | Added missing include, fixed enum safety in switch |
| `PauseMenuWidget.cpp` | Fixed pause menu wiring to GameMode |
| `TestUtilities.h/.cpp` | Added SU7Ultra specs (1900kg, 1138kW, 1200Nm, 350kph, 1.98s) |
| `VehicleSystemTest.h/.cpp` | Added 3 new test classes: SU7Ultra, ResetSafety, GetSpecs |
| `NOMISystemTest.h/.cpp` | Added 2 new test classes: CommentNotReady, CommentCategory; fixed always-passing assertion |

### Test Coverage Changes

| Metric | Before | After | Delta |
|--------|--------|-------|-------|
| Test classes | 14 | 19 | +5 |
| Vehicle type coverage | 3/6 types | 6/6 types | +3 (ET5, SU7Ultra, Custom) |
| ResetVehicle tests | 0 | 3 tests | +3 |
| CommentaryEngine tests | 1 (broken) | 5 (fixed + 4 new) | +4 |
| GetVehicleSpecs tests | 0 | 1 (all 6 types) | +1 |

**Anti-pattern fixed:** `FCommentMatchingTest` had an always-passing `TestTrue(TEXT("..."), true)` assertion. Replaced with `TestFalse` that actually tests the return value.

**Remaining gaps (require UE5 world integration):**
- ResetVehicle 3-path fallback (checkpoint -> nearest -> ground trace)
- AddToRecentComments bulk eviction (private array, no accessor)
- CheckStuckAndFlip auto-clear (requires world ticking)

### Review Findings

| Severity | Finding | Status |
|----------|---------|--------|
| HIGH | Inconsistent GetWorld() null-checking in NomiRaceGameMode — only ChampionshipManager spawn is guarded while 5 other bare GetWorld() calls remain | Filed for Cycle 2 |
| MEDIUM | Missing default case in new difficulty switch | Filed for Cycle 2 |
| LOW | Removed GameplayStatics.h include was a justified duplicate | No action needed |

### UX Improvement Proposals (Iterate Phase)

After reviewing tire physics, AI behavior, camera system, HUD, audio, and race management code:

| Priority | Finding | Impact | Effort |
|----------|---------|--------|--------|
| 1 | Pacejka tire model calculates forces every tick but `ApplyTireForces()` is a no-op — car drives on default Chaos physics. Tire type, surface grip, temperature, and wear are purely decorative. | CRITICAL — core physics disconnect | HIGH |
| 2 | Asymmetric AI rubber banding (25% catch-up boost vs 5% slowdown when ahead) creates classic rubber-band frustration | HIGH — player fairness | LOW — tune parameters |
| 3 | HUD is built but never populated with race data during gameplay | HIGH — players have no speed/position feedback | MEDIUM — wire data bindings |
| 4 | Results screen code exists but is never shown after race completion | HIGH — no closure/reward loop | MEDIUM — connect to RaceManager |
| 5 | Post-race flow (rematch, next race, garage) is missing | MEDIUM — players forced to main menu | LOW — add button handlers |

**Key insight:** 3 of 5 improvements address systems where code exists but is never connected. These are the highest-impact, lowest-effort changes to move from "prototype" to "playable demo."

---

