# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-06-02

### Added

#### Vehicle System
- NIO EP9 hypercar (1000kW, 1480Nm, 0-100 in 2.7s)
- NIO ET7 sedan (480kW, 850Nm, 0-100 in 3.8s)
- NIO ES7 SUV (480kW, 850Nm, 0-100 in 3.9s)
- NIO ET5 sedan (360kW, 700Nm, 0-100 in 4.0s)
- Chaos Vehicles physics integration
- Pacejka Magic Formula tire model
- Electric vehicle torque curves (0 RPM peak torque)
- Regenerative braking system
- Battery simulation with power reduction at low levels

#### NOMI AI Companion
- Commentary engine with 500+ pre-generated Chinese comments
- Event-based comment triggering (overtake, drift, collision, etc.)
- Emotional expression system (Happy, Excited, Concerned, etc.)
- Comment cooldown and deduplication
- Queue management for sequential comments

#### AI Opponents
- Behavior tree with overtaking, defending, and slipstream strategies
- 4 difficulty levels (Easy, Normal, Hard, Expert)
- Rubber band difficulty scaling
- Sensor system for obstacle detection
- Path prediction and avoidance

#### Race System
- Checkpoint-based lap tracking
- Championship management with standings
- Race progression and achievements
- Multiple race modes (Street GT, NIO Cup, Baja)

#### Camera System
- 7 camera modes (Chase, Hood, Cockpit, Bumper, Free, Cinematic, Replay)
- Dynamic FOV based on speed
- Camera shake (collision, drift, gear shift)
- Replay recording and playback

#### Particle System
- Tire smoke (acceleration slip)
- Drift smoke (lateral sliding)
- Collision sparks
- Dust effects
- Tire marks
- Rain effects

#### UI System
- Race HUD with speed, position, lap info
- NIO brand color theme
- NOMI emotion display

#### Development Tools
- Performance profiler with FPS, memory, GPU tracking
- Asset validator for mesh, texture, material validation
- Build scripts for Windows and macOS
- CI/CD pipeline with GitHub Actions

#### Testing
- Unit tests for vehicle, tire, AI, NOMI, race, camera, progression systems
- Integration tests for vehicle-race, AI-race, NOMI-event
- Performance benchmark tests
- Test utilities and helpers

### Fixed
- PeakMotorTorque permanent mutation when battery < 10%
- Division by zero in RaceManager progress calculation
- FirstPlace/LastPlace events broadcasting every tick
- Unsafe enum cast in NomiGameInstance settings
- AudioManager memory leak in PlayMusic/PlayNOMIVoice
- FindComponentByClass called every frame (cached references)
- TireModel formula inconsistency in thermal model
- Multiple GetWorld() null pointer issues

### Security
- No hardcoded secrets or API keys
- GitHub Secrets used for CI/CD credentials
- .gitignore properly configured for UE5 projects

---

## [Unreleased]

### Planned
- Additional NIO vehicle models
- More track environments
- Online multiplayer
- Mobile platform support
- VR support
