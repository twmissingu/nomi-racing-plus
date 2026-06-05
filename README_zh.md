[![English](https://img.shields.io/badge/English-blue.svg)](README.md)
[![中文](https://img.shields.io/badge/中文-red.svg)](README_zh.md)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![UE5](https://img.shields.io/badge/Unreal%20Engine-5.7-black.svg)](https://www.unrealengine.com)

---

# NIO 赛车 Plus

**驾驶蔚来电动超车驰骋赛道——真实物理、AI 伙伴、无尽乐趣。**

一款基于 UE5 的专业级赛车游戏，搭载蔚来车型（EP9、ET7、ES7、ET5），配备 Chaos Vehicles 物理引擎、NOMI AI 伙伴系统和智能 AI 对手。全部使用免费开源资源构建。

## 为什么选择这个项目？

### 🚗 **一行命令添加任意车辆**——游戏的核心亮点！

```bash
# 使用一条命令从 Sketchfab 导入任意车辆！
python3 Scripts/add_vehicle.py "https://sketchfab.com/3d-models/porsche-911-gt3-abc123" --color red
```

**就这么简单！** 系统会自动：
- 下载 3D 模型
- 应用你选择的车漆颜色
- 生成物理配置
- 创建发动机音效
- 集成到游戏中

**100+ 款 Sketchfab 车辆，即刻可玩！**

[📖 自定义车辆指南](Docs/Custom_Vehicle_Guide.md)

---

🏎️ **真实电动车物理**——这不是普通的街机赛车。体验 0 RPM 峰值扭矩、动能回收制动和模拟真实电动车行为的电池系统。

🤖 **NOMI AI 伙伴**——你的车载 AI 伙伴会对比赛事件做出反应，拥有 500+ 条预生成评论、情感表达和上下文感知响应。

🧠 **智能 AI 对手**——4 个难度等级，配备行为树、超车策略、滑流 drafting 和橡皮筋难度调节。

🎬 **专业相机系统**——7 种相机模式，包括电影镜头、回放系统和基于速度的动态 FOV。

## 功能特性

- ✨ **5 款车型**——EP9（超跑）、ET7（轿车）、ES7（SUV）、ET5（轿车）、SU7 Ultra（小米超级轿车）
- 🚀 **Chaos Vehicles 物理**——Pacejka 轮胎模型、漂移检测、轮胎温度/磨损
- 🎯 **比赛系统**——检查点、锦标赛、成就、进度系统
- 🎥 **相机系统**——追踪、引擎盖、驾驶舱、保险杠、自由、电影、回放
- 💨 **粒子效果**——轮胎烟雾、漂移烟雾、碰撞火花、尘土、雨水
- 🔊 **音频系统**——MetaSound 集成、NOMI 语音、环境音效
- 📊 **性能分析器**——FPS、内存、GPU 追踪与问题检测
- 🧪 **全面测试**——单元测试、集成测试和性能测试

## 快速开始

### 环境要求

- **Unreal Engine 5.7+**——[下载](https://www.unrealengine.com/download)
- **Windows 10/11** 或 **macOS 12+**
- **Visual Studio 2022**（Windows）或 **Xcode 14+**（macOS）

### 安装

```bash
# 克隆仓库
git clone https://github.com/twmissingu/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus
```

### 编译运行

**Windows:**
```bash
# 1. 右键 NomiRacingPlus.uproject → 生成 Visual Studio 项目文件
# 2. 打开 NomiRacingPlus.sln
# 3. 设置配置为 Development Editor | Win64
# 4. 生成 → 生成解决方案 (Ctrl+Shift+B)
# 5. 双击 NomiRacingPlus.uproject 打开编辑器
```

**macOS:**
```bash
# 1. 生成 Xcode 项目
"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/GenerateXcodeProject.sh"
# 2. 打开 NomiRacingPlus.xcodeproj
# 3. 设置 scheme 为 NomiRacingPlusEditor
# 4. 编译 (Cmd+B)
# 5. 双击 NomiRacingPlus.uproject 打开编辑器
```

## AI Agent 指南

本项目专为 AI agent 无缝交互设计：

```bash
# 1. 克隆并进入项目
git clone https://github.com/twmissingu/nomi-racing-plus.git
cd nomi-racing-plus/NomiRacingPlus

# 2. 项目结构
Source/NomiRacingPlus/    # C++ 源代码
Config/                   # 引擎配置
Content/                  # 游戏资源（JSON 配置）
Scripts/                  # 构建和设置脚本

# 3. 关键文件
NomiRacingPlus.uproject   # UE5 项目文件
Source/NomiRacingPlus/Vehicles/NIOVehicleMovementComponent.cpp  # 核心物理
Source/NomiRacingPlus/AI/AIBehaviorTree.cpp                     # AI 系统
Source/NomiRacingPlus/NOMI/CommentaryEngine.cpp                 # NOMI 系统

# 4. 运行测试（在 UE5 编辑器内）
# 窗口 → 测试自动化 → 运行测试
```

## 车辆参数

| 车型 | 功率 | 扭矩 | 0-100 km/h | 极速 | 类型 |
|------|------|------|------------|------|------|
| EP9 | 1000 kW | 1480 Nm | 2.7s | 313 km/h | 超跑 |
| ET7 | 480 kW | 850 Nm | 3.8s | 250 km/h | 轿车 |
| ES7 | 480 kW | 850 Nm | 3.9s | 200 km/h | SUV |
| ET5 | 360 kW | 700 Nm | 4.0s | 200 km/h | 轿车 |
| SU7 Ultra | 1138 kW | 1200 Nm | 1.98s | 350 km/h | 超级轿车 |

## 控制方式

### 键盘

| 操作 | 按键 |
|------|------|
| 加速 | W |
| 刹车 | S |
| 左转 | A |
| 右转 | D |
| 手刹 | 空格 |
| 回头看 | C |
| 切换视角 | V |
| 暂停 | ESC |

### 手柄

| 操作 | 按键 |
|------|------|
| 加速 | RT |
| 刹车 | LT |
| 转向 | 左摇杆 |
| 手刹 | A |
| 视角 | Y |

## 技术栈

| 组件 | 技术 |
|------|------|
| 引擎 | Unreal Engine 5.7 |
| 物理 | Chaos Vehicles |
| 渲染 | Nanite (Windows) / 手动 LOD (macOS) |
| 光照 | Lumen (软件回退) |
| 音频 | MetaSound |
| 输入 | Enhanced Input |

## 性能优化

### Windows
- 启用 Nanite 静态网格
- 使用 Lumen 全局光照
- 启用虚拟阴影贴图

### macOS
- Nanite 禁用（Metal 限制）
- 软件 Lumen 回退
- 降低阴影质量

## 贡献指南

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'feat: add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

### 第三方资源许可
- **蔚来车辆模型**: CC BY 4.0 [Sketchfab](https://sketchfab.com)
- **CARLA 资源**: MIT [GitHub](https://github.com/carla-simulator/carla)
- **Poly Haven**: CC0 [polyhaven.com](https://polyhaven.com)
- **ambientCG**: CC0 [ambientcg.com](https://ambientcg.com)

## 致谢

- [CARLA](https://github.com/carla-simulator/carla) — 城市环境资源
- [EngineSimulatorPlugin](https://github.com/nicholas477/EngineSimulatorPlugin) — 引擎物理参考
- [Unreal-NebulousVehicle](https://github.com/MrRobinOfficial/Unreal-NebulousVehicle) — 车辆基类
- [Sketchfab](https://sketchfab.com) — 蔚来车辆模型
- [Poly Haven](https://polyhaven.com) — HDR 天空盒
- [ambientCG](https://ambientcg.com) — PBR 材质

---

**版本**: 1.0.0 | **最后更新**: 2026-06-05 | **UE5 版本**: 5.7
