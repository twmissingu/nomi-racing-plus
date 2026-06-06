# NIO Racing Plus - Project Vision

> Last updated: 2026-06-05

## Mission

Build a **production-grade, open-source electric vehicle racing game** showcasing NIO's engineering through realistic EV physics, an intelligent AI companion (NOMI), and accessible-yet-deep gameplay. All built with free/open-source assets on Unreal Engine 5.7.

## Core Pillars

### 1. Physics Authenticity

Not an arcade racer. The driving model must reflect real EV behavior:

- 0 RPM peak torque (instant electric motor power)
- Regenerative braking on accelerator lift
- Pacejka Magic Formula tire model with thermal/wear behavior
- Combined slip friction circle for realistic force blending
- Surface-dependent grip (tarmac, wet, gravel, sand, grass, ice)
- Distinct handling per vehicle (EP9 hypercar vs ET5 sedan vs ES7 SUV)

**Current state:** Pacejka tire model calculates forces every tick and applies them via `ApplyTireForces()`. Thermal model affects grip based on temperature. Friction circle handles combined slip. Surface detection influences grip. All 6 vehicles have distinct handling characteristics.

### 2. NOMI as Companion

NOMI is not a voice assistant reading telemetry. It is a co-pilot with personality:

- Emotional awareness (excitement for overtakes, comfort after crashes)
- Context sensitivity (vehicle type, track, position, race situation)
- Frustration protection (proactive comfort when player struggles)
- NIO brand pride (highlights NIO-specific engineering features)

**Current state:** CommentaryEngine with 9 emotion states, comment matching, cooldown, and queue management. 500+ comments loaded from JSON. Visual face widget implemented.

### 3. Accessible Depth

- Easy to pick up (keyboard/gamepad work immediately)
- Deep to master (tire management, racing lines, drift technique at higher difficulties)
- 4 AI difficulty levels with rubber-band scaling
- Cross-platform (Windows full quality, macOS software fallback)

### 4. One-Line Vehicle Import

The `add_vehicle.py` pipeline: single command from Sketchfab URL to playable car. Handles model download, paint, physics config, sound generation, and game integration.

**Current state:** Script referenced in README but not present in repository. [MISSING]

## Target Audience

| Segment | Description | Priority |
|---------|-------------|----------|
| NIO enthusiasts / EV fans | Experience NIO vehicles in racing context | Primary |
| UE5 developers | Learn vehicle physics, AI, game architecture | Secondary |
| Casual racing players | Free, quality racing experience | Tertiary |

## What This Project Is NOT

- Not multiplayer (single-player + AI only, for now)
- Not mobile (UE5 desktop-only)
- Not commercial (personal learning/showcase)
- Not a CARLA fork (uses CARLA assets only, not codebase)
- Not a sim racing simulator (more accessible than iRacing, more realistic than Mario Kart)

## Quality Bar

- 60 FPS on mid-range Windows (RTX 3060, 1080p Medium)
- 45 FPS on Apple Silicon (M1 Pro, 1800p Medium)
- No game-breaking bugs (race always completes, save never corrupts, UI never freezes)
- Consistent physics (tire model actually affects driving)

## Brand Identity

- Primary: NIO Blue (#00A1E0)
- Secondary: Cyan (#00D4FF)
- Background: Deep blue-black (#0A0E1A)
- Typography: Rajdhani / Orbitron / Exo 2

## Long-Term Aspirations

Direction-setting goals, not commitments:

- Additional tracks (Mountain Pass, Desert Rally)
- Weather system affecting grip and visibility
- Online leaderboard (anonymous, no accounts)
- VR support
- Community vehicle sharing via add_vehicle.py

## Open Questions

- Multiplayer scope: local split-screen? online? [待确认]
- Monetization or sponsorship model? [待确认] (currently non-commercial)
- Mobile/tablet port feasibility with UE5? [待确认]
- Esports or competitive racing features? [待确认]
- MOD/UGC distribution beyond add_vehicle.py? [待确认]
