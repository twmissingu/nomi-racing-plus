# NIO Racing Plus — Vision

> The guiding vision for the project. All decisions should align with this document.

---

## Mission Statement

Build a **production-grade, open-source electric vehicle racing game** that showcases NIO's engineering excellence through realistic physics, intelligent AI companions, and an accessible yet deep driving experience — all using free and open-source assets.

---

## Core Values

### 1. Physics Authenticity

This is not an arcade racer. The driving experience must reflect real EV behavior:

- **0 RPM peak torque** — electric motors deliver instant power
- **Regenerative braking** — lifting the accelerator should feel like engine braking
- **Tire physics** — Pacejka Magic Formula model with thermal behavior, wear, and surface-dependent grip
- **Vehicle differentiation** — each car (EP9 hypercar, ET7 sedan, ES7 SUV, ET5 sedan, SU7 Ultra) must feel distinct

The Pacejka tire model currently calculates forces every tick but `ApplyTireForces()` is a no-op. **Wiring the tire model into Chaos Vehicles is the single most important physics task.**

### 2. NOMI as a Companion, Not a Gadget

NOMI is not a voice assistant reading numbers. NOMI is a co-pilot with personality:

- **Emotional awareness** — reacts to race events with appropriate emotions (excitement for overtakes, comfort after crashes)
- **Context sensitivity** — knows the vehicle type, track, position, and race situation
- **Frustration protection** — proactively provides comfort comments when the player is struggling (last place, repeated crashes, getting overtaken repeatedly)
- **NIO brand pride** — highlights NIO-specific features (quad-motor EP9, battery, regen braking)

### 3. Accessible Depth

- **Easy to pick up** — keyboard/gamepad controls work immediately
- **Deep to master** — tire management, racing lines, drift technique matter at higher difficulties
- **4 AI difficulty levels** — from rubber-band-assisted Easy to precision Expert
- **Cross-platform** — Windows (full quality) and macOS (software fallback) both playable

### 4. One Line to Add Any Car

The `add_vehicle.py` pipeline is the project's killer feature. Adding a new vehicle from Sketchfab should be a single command that handles model download, paint application, physics configuration, sound generation, and game integration. This pipeline must remain the simplest path from "I want this car" to "I can race this car."

---

## Target Audience

| Segment | Description | Priority |
|---------|-------------|----------|
| **Primary** | NIO enthusiasts and EV fans who want to experience NIO vehicles in a racing context | HIGH |
| **Secondary** | UE5 developers learning vehicle physics, AI, and game architecture | MEDIUM |
| **Tertiary** | Casual racing game players looking for a free, quality racing experience | LOW |

---

## Success Criteria

### Minimum Viable Game (MVG)

A player can:

1. Launch the game and navigate the main menu
2. Select a vehicle from the garage (5 NIO/Xiaomi vehicles)
3. Choose a track and race settings
4. Race against AI opponents with real physics
5. See their speed, position, and lap count on a HUD
6. Hear NOMI react to race events
7. View race results with position, time, and stats
8. Play again (rematch) or return to the garage

### Quality Bar

- **60 FPS** on mid-range Windows hardware (RTX 3060 / i5-12400 / 16GB) at 1080p Medium
- **45 FPS** on Apple Silicon (M1 Pro / 16GB) at 1800p Medium
- **No game-breaking bugs** — race always completes, save never corrupts, UI never freezes
- **Consistent physics** — tire model actually affects driving behavior

---

## What This Project Is NOT

| Not This | Why |
|----------|-----|
| A multiplayer game | Single-player + AI opponents only (for now) |
| A mobile game | UE5 desktop-only |
| A commercial product | Non-commercial, personal learning/showcase project |
| A CARLA fork | Uses CARLA assets only, not its codebase |
| A sim racing simulator | More accessible than iRacing, more realistic than Mario Kart |

---

## Technical Philosophy

- **C++ over Blueprints** — all game logic in C++ for performance and maintainability
- **JSON configuration** — vehicle physics, AI profiles, NOMI comments all in JSON for easy editing
- **Component architecture** — VehicleStateManager, CameraSystem, CommentaryEngine as reusable components
- **Event-driven communication** — race events flow through delegates, not direct coupling
- **Atomic saves** — write to .tmp then rename; CRC32 checksums; 3-backup rotation
- **Graceful degradation** — macOS falls back to software Lumen, manual LOD when Nanite unavailable

---

## Brand Identity

- **Primary color**: NIO Blue (#00A1E0)
- **Secondary color**: Cyan (#00D4FF)
- **Background**: Deep blue-black (#0A0E1A)
- **Design language**: Clean, minimal, tech-forward — inspired by NIO's own UI design
- **Typography**: Rajdhani / Orbitron / Exo 2 (geometric, tech feel)

---

## Long-Term Aspirations (Post-MVP)

These are direction-setting goals, not commitments:

- Additional tracks (Mountain Pass, Desert Rally)
- Weather system affecting grip and visibility
- Replay system with cinematic camera angles
- Online leaderboard (anonymous, no accounts)
- VR support
- Community vehicle sharing via the `add_vehicle.py` pipeline

---

*Last updated: 2026-06-05*
