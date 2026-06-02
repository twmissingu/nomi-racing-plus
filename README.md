[![English](https://img.shields.io/badge/English-blue.svg)](README.md)
[![中文](https://img.shields.io/badge/中文-red.svg)](README_zh.md)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![UE5](https://img.shields.io/badge/Unreal%20Engine-5.5-black.svg)](https://www.unrealengine.com)

---

# NIO Racing Plus

**Race with NIO's finest electric hypercars — realistic physics, AI companions, and endless fun.**

A production-grade UE5 racing game featuring NIO vehicles (EP9, ET7, ES7, ET5) with Chaos Vehicles physics, NOMI AI companion, and advanced AI opponents. Built entirely with free and open-source assets.

## 🌟 Why This Project?

### 🚗 **ONE LINE TO ADD ANY CAR** — The Game's Killer Feature!

```bash
# Import ANY vehicle from Sketchfab with ONE command!
python3 Scripts/add_vehicle.py "https://sketchfab.com/3d-models/porsche-911-gt3-abc123" --color red
```

**That's it!** The system automatically:
- Downloads the 3D model
- Applies your chosen paint color
- Generates physics configuration
- Creates motor sound effects
- Integrates with the game

**100+ cars from Sketchfab, instantly playable in your game!**

[📖 Custom Vehicle Guide](Docs/Custom_Vehicle_Guide.md)

---

🏎️ **Realistic Electric Vehicle Physics** — Not just another arcade racer. Experience 0 RPM peak torque, regenerative braking, and battery simulation that mimics real EV behavior.

🤖 **NOMI AI Companion** — Your in-car AI companion reacts to race events with 500+ pre-generated comments, emotional expressions, and context-aware responses.

🧠 **Smart AI Opponents** — 4 difficulty levels with behavior trees, overtaking strategies, slipstream drafting, and rubber band difficulty scaling.

🎬 **Professional Camera System** — 7 camera modes including cinematic shots, replay system, and dynamic FOV based on speed.

## Features

- ✨ **4 NIO Vehicles** — EP9 (1000kW hypercar), ET7 (sedan), ES7 (SUV), ET5 (sedan)
- 🚀 **Chaos Vehicles Physics** — Pacejka tire model, drift detection, tire thermal/wear
- 🎯 **Race System** — Checkpoints, championships, achievements, progression
- 🎥 **Camera System** — Chase, Hood, Cockpit, Bumper, Free, Cinematic, Replay
- 💨 **Particle Effects** — Tire smoke, drift smoke, collision sparks, dust, rain
- 🔊 **Audio System** — MetaSound integration, NOMI voice, ambient sounds
- 📊 **Performance Profiler** — FPS, memory, GPU tracking with issue detection
- 🧪 **Comprehensive Tests** — Unit, integration, and performance tests

## Quick Start

### Prerequisites

- **Unreal Engine 5.5+** — [Download](https://www.unrealengine.com/download)
- **Windows 10/11** or **macOS 12+**
- **Visual Studio 2022** (Windows) or **Xcode 14+** (macOS)

### Installation

```bash
# Clone the repository
git clone https://github.com/twmissingu/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus
```

### Build & Run

**Windows:**
```bash
# 1. Right-click NomiRacingPlus.uproject → Generate Visual Studio project files
# 2. Open NomiRacingPlus.sln
# 3. Set configuration to Development Editor | Win64
# 4. Build → Build Solution (Ctrl+Shift+B)
# 5. Double-click NomiRacingPlus.uproject to open editor
```

**macOS:**
```bash
# 1. Generate Xcode project
"/Users/Shared/Epic Games/UE_5.5/Engine/Build/BatchFiles/Mac/GenerateXcodeProject.sh"
# 2. Open NomiRacingPlus.xcodeproj
# 3. Set scheme to NomiRacingPlusEditor
# 4. Build (Cmd+B)
# 5. Double-click NomiRacingPlus.uproject to open editor
```

## For AI Agents

This project is designed for seamless AI agent interaction:

```bash
# 1. Clone and enter project
git clone https://github.com/twmissingu/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus

# 2. Project structure
Source/NomiRacingPlus/    # C++ source code
Config/                   # Engine configuration
Content/                  # Game assets (JSON configs)
Scripts/                  # Build and setup scripts

# 3. Key files
NomiRacingPlus.uproject   # UE5 project file
Source/NomiRacingPlus/Vehicles/NIOVehicleMovementComponent.cpp  # Core physics
Source/NomiRacingPlus/AI/AIBehaviorTree.cpp                     # AI system
Source/NomiRacingPlus/NOMI/CommentaryEngine.cpp                 # NOMI system

# 4. Run tests (inside UE5 Editor)
# Window → Test Automation → Run Tests
```

## Vehicle Specifications

| Model | Power | Torque | 0-100 km/h | Top Speed | Type |
|-------|-------|--------|------------|-----------|------|
| EP9 | 1000 kW | 1480 Nm | 2.7s | 313 km/h | Hypercar |
| ET7 | 480 kW | 850 Nm | 3.8s | 250 km/h | Sedan |
| ES7 | 480 kW | 850 Nm | 3.9s | 200 km/h | SUV |
| ET5 | 360 kW | 700 Nm | 4.0s | 200 km/h | Sedan |

## Controls

### Keyboard

| Action | Key |
|--------|-----|
| Throttle | W |
| Brake | S |
| Steer Left | A |
| Steer Right | D |
| Handbrake | Space |
| Look Back | C |
| Change Camera | V |
| Pause | ESC |

### Gamepad

| Action | Button |
|--------|--------|
| Throttle | RT |
| Brake | LT |
| Steer | Left Stick |
| Handbrake | A |
| Camera | Y |

## Tech Stack

| Component | Technology |
|-----------|------------|
| Engine | Unreal Engine 5.5 |
| Physics | Chaos Vehicles |
| Rendering | Nanite (Windows) / Manual LOD (macOS) |
| Lighting | Lumen (Software fallback) |
| Audio | MetaSound |
| Input | Enhanced Input |

## Performance Tips

### Windows
- Enable Nanite for static meshes
- Use Lumen for global illumination
- Enable virtual shadow maps

### macOS
- Nanite disabled (Metal limitation)
- Software Lumen fallback
- Reduced shadow quality

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Third-Party Assets
- **NIO Vehicle Models**: CC BY 4.0 [Sketchfab](https://sketchfab.com)
- **CARLA Assets**: MIT [GitHub](https://github.com/carla-simulator/carla)
- **Poly Haven**: CC0 [polyhaven.com](https://polyhaven.com)
- **ambientCG**: CC0 [ambientcg.com](https://ambientcg.com)

## Acknowledgments

- [CARLA](https://github.com/carla-simulator/carla) — City environment assets
- [EngineSimulatorPlugin](https://github.com/nicholas477/EngineSimulatorPlugin) — Engine physics reference
- [Unreal-NebulousVehicle](https://github.com/MrRobinOfficial/Unreal-NebulousVehicle) — Vehicle base class
- [Sketchfab](https://sketchfab.com) — NIO vehicle models
- [Poly Haven](https://polyhaven.com) — HDR skyboxes
- [ambientCG](https://ambientcg.com) — PBR materials

---

**Version**: 1.0.0 | **Last Updated**: 2026-06-02 | **UE5 Version**: 5.5
