# NIO Racing Plus

> 基于 CARLA + UE5 的写实 3D 赛车游戏
> 预算：$0（全部使用免费开源资源，非商业用途）

## 项目概述

NIO Racing Plus 是一款以蔚来品牌为主题的写实 3D 赛车游戏，使用 Unreal Engine 5.5 开发。

### 核心特性

- **写实画质**：CARLA 级别的城市环境、车辆模型、光照渲染
- **真实物理**：UE5 Chaos Vehicles 物理引擎，支持漂移、轮胎滑移、悬挂反馈
- **蔚来品牌**：NIO EP9/ET7/ES7/ET5 四款车型
- **NOMI 副驾**：球形机器人 + 预生成评论池 + 本地匹配引擎
- **跨平台**：Windows + macOS（Apple Silicon）

## 快速开始

### 环境要求

- Unreal Engine 5.5+
- Windows 10/11 或 macOS 12+
- Visual Studio 2022 (Windows) 或 Xcode 14+ (macOS)

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
/Users/Shared/Epic\ Games/UE_5.5/Engine/Build/BatchFiles/Mac/GenerateXcodeProject.sh
# 2. 打开 NomiRacingPlus.xcodeproj
# 3. 设置 scheme 为 NomiRacingPlusEditor
# 4. 编译 (Cmd+B)
# 5. 双击 NomiRacingPlus.uproject 打开编辑器
```

## 技术栈

| 组件 | 技术 |
|------|------|
| 引擎 | Unreal Engine 5.5 |
| 物理 | Chaos Vehicles |
| 渲染 | Nanite (Windows) / 手动 LOD (macOS) |
| 光照 | Lumen (软件回退) |
| 音频 | MetaSound |
| 输入 | Enhanced Input |

## 项目结构

```
NomiRacingPlus/
├── Config/                 # 配置文件
│   ├── DefaultEngine.ini   # 引擎设置
│   ├── DefaultInput.ini    # 输入绑定
│   └── DefaultGameplayTags.ini
├── Content/
│   ├── NOMI/               # NOMI 系统资源
│   │   └── Comments/       # 评论池 (500+ 条)
│   ├── Vehicles/           # 车辆配置
│   └── AI/                 # AI 行为配置
├── Source/NomiRacingPlus/
│   ├── AI/                 # AI 对手系统
│   │   ├── AIBehaviorTree  # 行为树
│   │   ├── AICarController # AI 控制器
│   │   └── AISensorSystem  # 传感器系统
│   ├── Camera/             # 相机系统
│   │   └── CameraSystem    # 7 种相机模式
│   ├── Core/               # 核心系统
│   │   ├── AudioManager    # 音频管理
│   │   ├── NomiGameInstance# 游戏实例
│   │   └── NomiRaceGameMode# 游戏模式
│   ├── Effects/            # 粒子效果
│   │   └── ParticleSystem  # 轮胎烟雾、漂移烟雾等
│   ├── NOMI/               # NOMI 伴侣系统
│   │   ├── CommentaryEngine# 评论引擎
│   │   └── NOMIController  # NOMI 控制器
│   ├── Race/               # 比赛系统
│   │   ├── RaceManager     # 比赛管理
│   │   ├── CheckpointSystem# 检查点系统
│   │   └── ChampionshipManager # 锦标赛管理
│   ├── UI/                 # 用户界面
│   │   ├── RaceHUD         # 游戏内 HUD
│   │   └── NIOColorTheme  # NIO 品牌色彩
│   ├── Vehicles/           # 车辆系统
│   │   ├── NIOVehicleMovementComponent # 电动车物理
│   │   ├── TirePhysicsModel # Pacejka 轮胎模型
│   │   └── VehicleStateManager # 状态管理
│   └── Tests/              # 测试用例
└── Scripts/                # 构建和设置脚本
```

## 核心系统

### 1. 车辆系统

- `UVehicleStateManager` - 车辆状态管理（速度、电池、轮胎温度）
- `UNIOVehicleMovementComponent` - NIO 电动车物理（0 RPM 峰值扭矩、动能回收）
- `UTirePhysicsModel` - Pacejka Magic Formula 轮胎模型
- `ANIOVehicleBase` - 车辆基类
- `ANIO_EP9` / `ANIO_ET7` / `ANIO_ES7` / `ANIO_ET5` - 具体车型

### 2. 比赛系统

- `ARaceManager` - 比赛状态机（Idle → Countdown → Racing → Finished）
- `ACheckpoint` / `ACheckpointTrack` - 检查点系统
- `AChampionshipManager` - 锦标赛管理
- 计时、名次、圈速计算

### 3. NOMI 系统

- `UCommentaryEngine` - 评论匹配引擎（500+ 条预生成评论）
- `ANOMIController` - NOMI 视觉控制（表情、动画）
- 事件触发、冷却、去重、队列机制

### 4. AI 系统

- `AAICarController` - AI 控制器
- `UAIBehaviorTree` - 行为树（超车、防守、滑流）
- `UAISensorSystem` - 传感器系统
- `UAIRubberBandScaler` - 橡皮筋难度调节
- 4 档难度（Easy/Normal/Hard/Expert）

### 5. 相机系统

- 7 种相机模式（追踪、引擎盖、驾驶舱、保险杠、自由、电影、回放）
- 动态 FOV 调节
- 碰撞/漂移/换挡震动
- 回放系统

### 6. 粒子系统

- 轮胎烟雾（加速打滑）
- 漂移烟雾（横向滑移）
- 碰撞火花
- 尘土效果
- 轮胎痕迹
- 雨天效果

## 车辆参数

| 车型 | 功率 | 扭矩 | 0-100 km/h | 极速 | 车身类型 |
|------|------|------|------------|------|---------|
| EP9 | 1000 kW | 1480 Nm | 2.7s | 313 km/h | 超跑 |
| ET7 | 480 kW | 850 Nm | 3.8s | 250 km/h | 轿车 |
| ES7 | 480 kW | 850 Nm | 3.9s | 200 km/h | SUV |
| ET5 | 360 kW | 700 Nm | 4.0s | 200 km/h | 轿车 |

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
| 手刹 | A 键 |
| 视角 | Y 键 |

## 性能优化

### Windows
- 启用 Nanite 静态网格
- 使用 Lumen 全局光照
- 启用虚拟阴影贴图

### macOS
- Nanite 禁用（Metal 限制）
- 软件 Lumen 回退
- 降低阴影质量

### 推荐设置

| 设置 | 低 | 中 | 高 |
|------|----|----|| 
| 分辨率 | 720p | 1080p | 1440p |
| 阴影 | 低 | 中 | 高 |
| 纹理 | 低 | 中 | 高 |
| 特效 | 低 | 中 | 高 |

## 开发计划

- **Phase 0**：许可证验证与 Spike（1-1.5 周）
- **Phase 1**：基础搭建（4-5 周）
- **Phase 2**：车辆系统（4-5 周）
- **Phase 3**：赛道制作（6-8 周）
- **Phase 4**：游戏循环（5-6 周）
- **Phase 5**：体验打磨（4-5 周）
- **Phase 6**：发布准备（3-4 周）

## 许可证

- 代码：MIT
- NIO 模型：CC BY 4.0（Sketchfab）
- CARLA 资产：MIT
- 其他资产：见 LICENSES.md

## 参考资源

- [CARLA](https://github.com/carla-simulator/carla) - 城市环境资产
- [EngineSimulatorPlugin](https://github.com/nicholas477/EngineSimulatorPlugin) - 引擎物理参考
- [Unreal-NebulousVehicle](https://github.com/MrRobinOfficial/Unreal-NebulousVehicle) - 车辆基础类
- [Sketchfab](https://sketchfab.com) - NIO 车辆模型
- [Poly Haven](https://polyhaven.com) - HDR 天空盒
- [ambientCG](https://ambientcg.com) - PBR 材质

---

**版本**: 1.0.0
**最后更新**: 2026-06-01
**UE5 版本**: 5.5
