# NIO Racing Plus — 详细开发计划

> 基于 CARLA + UE5 的写实 3D 赛车游戏升级方案
> 预算：$0（全部使用免费开源资源，非商业用途）
> 预估工期：24-32 周（6-8 个月，单人开发）

---

## 术语表

| 术语 | 定义 |
|------|------|
| CARLA | 开源自动驾驶仿真器，本项目仅提取其城镇地图和车辆模型资产 |
| Chaos Vehicles | UE5 内置车辆物理引擎 |
| MetaSound | UE5 音频图系统，用于实时音效处理 |
| LOD | Level of Detail，根据距离切换模型精度 |
| Nanite | UE5 虚拟化几何系统，支持电影级高模（不支持 macOS Metal） |
| Lumen | UE5 全局光照系统 |
| 加速踏板/制动踏板 | 统一术语，不使用"油门/刹车" |
| NOMI | 蔚来车载 AI 伴侣，球形机器人 |

---

## 一、项目概述

### 1.1 目标

将现有 Godot 版 nomi-racing 升级为 CARLA 级画质的写实 3D 赛车游戏，核心特征：

- **写实画质**：CARLA 级别的城市环境、车辆模型、光照渲染
- **真实物理**：UE5 Chaos Vehicles 物理引擎，支持漂移、轮胎滑移、悬挂反馈
- **蔚来品牌**：NIO EP9/ET7/ES7 三款车型（确定交付），ET5 可选（最后阶段）
- **NOMI 副驾**：球形机器人 + 预生成评论池 + 本地匹配引擎
- **跨平台**：Windows + macOS（Apple Silicon）
- **非商业**：个人学习/展示项目，非商业发布

### 1.2 技术架构

| 维度 | 决策 |
|------|------|
| 引擎 | Unreal Engine 5.7（Vulkan/Metal） |
| 基础 | **待验证**：Fork CARLA 或仅提取资产（Phase 1 Spike 决定） |
| 物理 | Chaos Vehicles（替代 CARLA 默认物理） |
| 车辆 | CARLA 原有车辆 + NIO 3 辆确定（EP9/ET7/ES7）+ ET5 可选 |
| 赛道 | 5 条（CARLA 城镇×2 + 山地 + 椭圆 + 沙漠） |
| 模式 | Street GT / NIO / Baja 三种 |
| NOMI | 预生成评论池（500+ 条）+ 本地匹配引擎 |
| 平台 | Windows + macOS（M1 Pro+ 推荐） |
| 多人 | 初期仅单人 + AI 对手 |
| UI | 科技风 + NIO Blue（#00A1E0）主题 |
| 音频 | Freesound.org 采样 + MetaSound 实时处理 |

### 1.3 游戏模式定义

| 模式 | 玩法规则 | 可用车辆 | 赛道 |
|------|---------|---------|------|
| **Street GT** | 城市街道赛，N 圈制，允许碰撞，无碰撞惩罚 | 全部车辆 | 城市赛道 |
| **NIO** | NIO 品牌专属赛，N 圈制，仅限 NIO 车型，电动扭矩优势 | NIO EP9/ET7/ES7(+ET5) | 城市赛道 |
| **Baja** | 沙漠越野赛，点对点，无圈数，地形影响抓地力 | 越野车型 | 沙漠赛道 |

---

## 二、CARLA 基础 — 资产获取与技术验证

### 2.1 CARLA 仓库信息

- **仓库**：https://github.com/carla-simulator/carla
- **分支**：`ue5-dev`（UE 5.7 版本）
- **许可证**：MIT（代码），资产许可证需单独确认
- **语言**：C++ + Python API

### 2.2 Phase 0：许可证验证与 Spike（Phase 1 最先执行）

> **这是项目启动的第一个任务，阻断后续所有工作。**

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 0.1 资产许可证审计 | 下载 CARLA 发布包，确认城镇地图和车辆模型的许可证条款。确认是否可用于非商业衍生项目。 | 明确许可证允许或不允许 | 1 天 |
| 0.2 技术路径 Spike | 依次尝试两条路径：先 (A) Fork CARLA + 裁剪编译，若编译失败或裁剪不可行则切换到 (B) 新建 UE5 + 仅导入资产 | 两条路径的可行性和工作量对比 | 5-7 天 |

**Spike 决策矩阵**（Phase 0 结束时填写）：

| 维度 | 权重 | Fork 路径评分 (1-5) | 仅提取路径评分 (1-5) |
|------|------|-------------------|-------------------|
| 编译成功率（macOS + Windows） | 30% | ? | ? |
| 首次可运行耗时 | 25% | ? | ? |
| 资产完整性（地图/车辆/交通灯） | 20% | ? | ? |
| 后续维护成本 | 15% | ? | ? |
| 代码可控性 | 10% | ? | ? |

加权总分高者胜出。如选择 Fork 路径，锁定 CARLA 版本为一次性快照（记录 commit hash），后续仅 cherry-pick 关键修复，不跟踪上游。
| 0.3 路径决策 | 基于 Spike 结果选择最终技术路径 | 选定路径，更新计划 | 1 天 |

**如果 CARLA 资产不可用（许可证不允许或技术提取失败）**：
- 降级方案：使用 UE5 City Sample（Epic 商店免费）+ Quixel Megascans 自建城市环境
- 降级影响：工期增加 2-3 周（城市环境需要从零搭建）

### 2.3 资产归档策略

所有外部资产获取后必须本地归档，防止平台变更导致资产丢失：

```
assets-archive/                    # Git LFS 管理
├── carla/                         # CARLA 发布包资产
│   ├── maps/                      # 城镇地图 .uasset
│   └── vehicles/                  # 车辆模型 .uasset
├── sketchfab/                     # Sketchfab 下载的模型
│   ├── EP9/
│   │   ├── source.glb             # 原始下载
│   │   └── EP9.fbx                # Blender 导出
│   ├── ET7/
│   └── ES7/
├── textures/                      # ambientCG / Poly Haven 贴图
├── audio/                         # Freesound / Sonniss 音效
└── LICENSES.md                    # 每个资产的许可证记录
```

**规则**：
- 首次下载后立即归档到本地 Git LFS 仓库
- 记录每个资产的来源 URL、许可证类型、下载日期
- Sketchfab 模型保留原始 GLB + Blender 导出 FBX 双重备份

### 2.4 Fork CARLA 路径（如果 Spike 选择此路径）

需要保留的 CARLA 组件：

| 组件 | 说明 | 保留/移除 |
|------|------|----------|
| UE5 项目结构 | Config/Plugins/Source 基础框架 | ✅ 保留 |
| 城镇地图 | Town01-Town10 城市环境 | ✅ 保留 2 个（最佳画质的） |
| 车辆模型 | Audi TT、Dodge Charger 等 | ✅ 保留（GT 模式用） |
| 交通信号系统 | 红绿灯、路标 | ✅ 保留（城市赛道氛围） |
| 行人系统 | NPC 行人 | ✅ 保留（城市赛道氛围） |
| 传感器系统 | 摄像头/LiDAR/雷达 | ❌ 移除（自动驾驶专用） |
| Python API | 自动驾驶控制接口 | ❌ 移除 |
| ROS Bridge | ROS 通信桥接 | ❌ 移除 |
| Scenario Runner | 自动驾驶场景 | ❌ 移除 |
| Leaderboard | 自动驾驶排行榜 | ❌ 移除 |

裁剪工作清单：
1. 删除 `Unreal/CarlaUnreal/` 下的自动驾驶传感器蓝图
2. 移除 Python API 插件（`PythonAPI/`）
3. 移除 ROS Bridge 插件
4. 保留车辆蓝图，但移除自动驾驶控制器
5. 保留城镇地图，但移除交通模拟逻辑（保留静态道路和建筑）
6. 清理 `Plugins/` 目录，只保留核心渲染和车辆插件

**上游同步策略**：
- 锁定 CARLA 版本为一次性快照（记录 commit hash）
- Phase 0-1 期间可跟踪上游关键修复
- Phase 2 起不再合并上游，仅 cherry-pick 关键 Bug 修复
- 资产版本与代码版本锁定，不单独更新

### 2.5 仅提取资产路径（如果 Spike 选择此路径）

1. 下载 CARLA 发布包
2. 新建纯 UE5 5.7 项目
3. 仅导入 CARLA 的城镇地图 .uasset 和车辆模型 .uasset
4. 在新项目中从零搭建游戏逻辑（不继承 CARLA 代码架构）
5. 优点：无裁剪负担，架构干净
6. 缺点：需要手动适配资产依赖

---

## 三、NIO 车辆模型方案

### 3.1 模型来源：Sketchfab（免费 CC Attribution）

**确定交付（3 款）**：

| 车型 | Sketchfab 模型 | 面数 | LOD0 | LOD1 | LOD2 | 许可证 | URL |
|------|---------------|------|------|------|------|--------|-----|
| **EP9** | NIO EP9 2017 | 1,400,252 面 | 60 万 | 15 万 | 3 万 | CC BY | https://sketchfab.com/3d-models/nio-ep9-2017-b9bfaa1ea4824bef85ea755f8c10c6d2 |
| **ET7** | Nio ET7 2021 | 1,043,414 面 | 50 万 | 12 万 | 2.5 万 | CC BY | https://sketchfab.com/3d-models/nio-et7-2021-b428077c63a743c6bf82059e2ec3b4fb |
| **ES7** | Nio ES7 2023 | 588,958 面 | 40 万 | 10 万 | 2 万 | CC BY | https://sketchfab.com/3d-models/nio-es7-2023-4d9c574b84514b21ac783aca550793fe |

**可选交付（最后阶段）**：

| 车型 | 状态 | 方案 |
|------|------|------|
| **ET5** | ⚠️ 可选 | 从 ET7 改造（Blender 手工调整，非简单缩放）或搜索其他平台 |

> **注意**：2022 NIO ET7（561K 面）为 CC BY-NC-SA 许可，不可用于本项目。本项目为非商业用途，CC BY 许可的模型可在 Credits 中标注原作者使用。

### 3.2 ET5 模型方案（最后阶段，可选）

ET5 在 Sketchfab 上没有现成模型。如需交付，可选方案：

| 方案 | 说明 | 可行性 |
|------|------|--------|
| **A: 从 ET7 改造** | 在 Blender 中手工调整 ET7 模型（修改车身比例、前脸、尾部设计语言） | ⭐⭐ 需 1 周 Blender 工作 |
| **B: 寻找其他平台** | CGTrader、TurboSquid、Free3D 搜索 | ⭐ 不确定 |
| **C: 砍掉 ET5** | 仅交付 3 款 NIO 车型 | ⭐⭐⭐ 最稳妥 |

**建议**：优先尝试方案 A，如果质量不达标则执行方案 C。ET5 不阻断核心开发。

### 3.3 模型处理流程（含 LOD 生成）

```
Sketchfab 下载 (.glb/.fbx)
    │
    ▼
本地归档（assets-archive/sketchfab/）
    │
    ▼
Blender 导入
    │
    ├─ 检查拓扑结构
    ├─ 修复 UV 映射（如需要）
    ├─ 设置材质槽（车漆/玻璃/轮胎/轮毂/车灯）
    │
    ├─ ★ LOD 生成（必须）
    │   ├─ LOD0：原始面数（60 万面用于 Nanite，40 万面用于非 Nanite）
    │   ├─ LOD1：Blender Decimate → 目标 10-15 万面
    │   └─ LOD2：Blender Decimate → 目标 2-3 万面
    │
    ▼
导出 FBX（含 LOD 层级和材质引用）
    │
    ▼
UE5 导入
    │
    ├─ 启用 Nanite（Windows，EP9/ET7 高模）或手动 LOD（macOS）
    ├─ 创建 Vehicle Blueprint
    ├─ 配置 Chaos Vehicles 物理参数
    ├─ 应用 PBR 材质（车漆/玻璃/金属）
    ├─ 设置车灯（大灯/尾灯/刹车灯）
    └─ 绑定碰撞体
```

**面数预算**：
| LOD 级别 | 距离 | EP9 目标 | ET7 目标 | ES7 目标 |
|---------|------|---------|---------|---------|
| LOD0 | 0-10m | 60 万面 | 50 万面 | 40 万面 |
| LOD1 | 10-50m | 15 万面 | 12 万面 | 10 万面 |
| LOD2 | 50m+ | 3 万面 | 2.5 万面 | 2 万面 |

**macOS 注意**：Nanite 在 Metal 上支持有限，macOS 必须依赖手动 LOD 层级。

### 3.4 NIO 车辆物理参数（基于真实数据）

```json
{
  "EP9": {
    "mass_kg": 1735,
    "power_kw": 1000,
    "torque_nm": 1480,
    "drive_type": "AWD_quad_motor",
    "top_speed_kph": 313,
    "acceleration_0_100": 2.7,
    "downforce_max_kg": 2000,
    "wheel_base_mm": 2750,
    "body_type": "hypercar"
  },
  "ET7": {
    "mass_kg": 2379,
    "power_kw": 480,
    "torque_nm": 850,
    "drive_type": "AWD_dual_motor",
    "top_speed_kph": 250,
    "acceleration_0_100": 3.8,
    "wheel_base_mm": 3060,
    "body_type": "sedan"
  },
  "ET5": {
    "mass_kg": 2070,
    "power_kw": 360,
    "torque_nm": 700,
    "drive_type": "AWD_dual_motor",
    "top_speed_kph": 200,
    "acceleration_0_100": 4.0,
    "wheel_base_mm": 2888,
    "body_type": "sedan"
  },
  "ES7": {
    "mass_kg": 2400,
    "power_kw": 480,
    "torque_nm": 850,
    "drive_type": "AWD_dual_motor",
    "top_speed_kph": 200,
    "acceleration_0_100": 3.9,
    "wheel_base_mm": 2960,
    "body_type": "suv"
  }
}
```

### 3.5 NIO 真实特性实现

| 特性 | 实现方式 |
|------|---------|
| **电动扭矩曲线** | Chaos Vehicles 自定义扭矩曲线：0 RPM 即峰值扭矩，高速衰减 |
| **无声加速** | MetaSound：电机高频嗡鸣 + 风噪 + 轮胎噪音（无引擎声） |
| **换电站动画** | UE5 Sequencer：车辆停入 → 电池包滑出 → 新电池滑入 |
| **NOMI 机器人** | Skeletal Mesh + Material Animation（详见第六节） |
| **NIO 蓝色主题** | UI 材质 #00A1E0 + 发光效果 |

---

## 四、赛道与环境

### 4.1 赛道规划

| 序号 | 赛道名称 | 类型 | 环境来源 | 赛道特征 |
|------|---------|------|---------|---------|
| 1 | **NIO City Circuit** | 城市赛道 | CARLA Town03 改造 | 夜景霓虹，蔚来换电站，城市天际线 |
| 2 | **Shanghai Pudong** | 城市赛道 | CARLA Town06 改造 | 陆家嘴风格，东方明珠背景，隧道段 |
| 3 | **Mountain Pass** | 山地赛道 | Quixel Megascans 岩石 + UE5 地形 | 发夹弯，海拔变化，悬崖景观 |
| 4 | **Speedway Oval** | 椭圆赛道 | UE5 程序化生成 | 高速椭圆，倾斜弯道，看台观众 |
| 5 | **Desert Rally** | 沙漠越野 | Quixel Megascans 沙地 + 岩石 | 点对点，沙丘，峡谷，日落 |

### 4.2 环境资产来源

| 资产类型 | 来源 | 许可证 | URL |
|---------|------|--------|-----|
| 城市建筑 | CARLA 自带 | MIT | 内置于 CARLA |
| 道路标线 | CARLA 自带 | MIT | 内置于 CARLA |
| HDR 天空盒 | Poly Haven | CC0 | https://polyhaven.com/hdris |
| PBR 路面材质 | ambientCG | CC0 | https://ambientcg.com |
| 岩石/山体 | Quixel Megascans (Fab) | 免费用于 UE | https://fab.com |
| 沙地材质 | ambientCG | CC0 | https://ambientcg.com |
| 植被 | Quixel Megascans (Fab) | 免费用于 UE | https://fab.com |
| 街道家具 | CARLA 自带 + Fab 免费资产 | MIT / UE EULA | - |

### 4.3 光照方案

| 赛道 | 光照预设 | 特殊效果 |
|------|---------|---------|
| NIO City Circuit | 夜晚 | 霓虹灯、车灯反射、湿滑路面 |
| Shanghai Pudong | 黄昏 | 城市天际线剪影、暖色调天空 |
| Mountain Pass | 正午 | 强烈阳光、山体阴影、薄雾 |
| Speedway Oval | 白天 | 明亮、高速动态模糊 |
| Desert Rally | 日落 | 长阴影、沙尘粒子、热浪扭曲 |

### 4.4 赛道制作流程

```
1. 选定 CARLA 城镇地图（或创建新地图）
2. 在 UE5 编辑器中加载地图
3. 移除不需要的自动驾驶元素（传感器支架、测试标记等）
4. 添加赛车元素：
   ├─ 起跑线/终点线（发光白线 + 计时拱门）
   ├─ 检查点（Area3D 触发器）
   ├─ AI 路径点（Spline + Curve）
   ├─ 赛道护栏（碰撞体 + 视觉网格）
   └─ 路边广告牌（NIO 品牌）
5. 添加环境装饰：
   ├─ HDR 天空盒
   ├─ 方向光 + 环境光
   ├─ 体积雾（如需要）
   └─ 粒子系统（尘土、落叶等）
6. 性能优化：
   ├─ 静态网格 LOD
   ├─ 遮挡剔除
   └─ 纹理流送
```

---

## 五、物理系统 — Chaos Vehicles

### 5.1 参考项目

| 项目 | 用途 | 链接 |
|------|------|------|
| **EngineSimulatorPlugin** | 引擎模拟（RPM、档位、音效） | https://github.com/nicholas477/EngineSimulatorPlugin |
| **Unreal-NebulousVehicle** | 离合器、熄火、车灯、Enhanced Input | https://github.com/MrRobinOfficial/Unreal-NebulousVehicle |
| **Car Race Championship** | C++ 赛车架构参考（AI、计分） | https://github.com/anshulyadav32/car-race-championship |
| **UE5-Vehicle-Template** | 交通 AI + 可驾驶车辆 | https://github.com/AnastasisMarinos/UE5-Vehicle-Template |

### 5.2 Chaos Vehicles 配置

每辆车需要配置以下参数：

```
ChaosVehicleMovementComponent
├─ Engine Setup
│   ├─ Torque Curve（扭矩曲线）
│   ├─ Max RPM
│   ├─ Idle RPM
│   └─ Engine Braking
├─ Transmission Setup
│   ├─ Forward Gear Ratios（前进档齿比）
│   ├─ Reverse Gear Ratio
│   ├─ Final Drive Ratio
│   └─ Shift Time
├─ Steering Setup
│   ├─ Max Steering Angle
│   └─ Steering Curve（速度-转向角曲线）
├─ Suspension Setup（每轮独立）
│   ├─ Spring Rate
│   ├─ Damping Ratio
│   ├─ Max Compression
│   └─ Max Droop
├─ Tire Setup（每轮独立）
│   ├─ Friction Force
│   ├─ Lateral Stiffness
│   └─ Longitudinal Stiffness
├─ Differential Setup
│   ├─ Front/Rear Torque Split
│   └─ Limited Slip (LSD) settings
└─ Aerodynamic Setup
    ├─ Downforce Coefficient（下压力系数，基于 EP9 的 downforce_max_kg 推导）
    ├─ Drag Coefficient（风阻系数）
    └─ Air Density（空气密度，影响高速稳定性）
```

### 5.3 电动车物理特殊处理

NIO 电动车与燃油车物理差异：

| 特性 | 燃油车（CARLA 默认） | NIO 电动车 |
|------|---------------------|-----------|
| 扭矩曲线 | 低转无扭矩，中高转峰值 | 0 RPM 即峰值扭矩，高速衰减 |
| 变速箱 | 多档位手动/自动 | 单速减速器（无换挡） |
| 引擎制动 | 松加速踏板有发动机制动 | 动能回收制动（可调节强度） |
| 重心 | 发动机舱较重 | 电池在底盘，重心更低 |
| 声音 | 引擎轰鸣 | 电机嗡鸣 + 风噪 |

### 5.4 漂移系统

参考原版 nomi-racing 的漂移模型，在 Chaos Vehicles 上重新实现：

```
漂移触发条件：
1. 手刹锁死后轮 → 侧滑角增大
2. 高速入弯 → 后轮超过摩擦力极限
3. 重心转移 → 前轮抓地后轮滑动

漂移维持：
- 检测侧滑角（slip angle）
- 修正方向盘角度（counter-steer）
- 油门控制（power over）

漂移恢复：
- 侧滑角减小 → 恢复正常行驶
- 计时器超时 → 强制恢复
```

---

## 六、NOMI AI 副驾系统

### 6.1 真实 NOMI 形态

基于调研，NOMI 是蔚来的实体车载 AI 伴侣：

- **外观**：球形头部 + 小底座，白色极简设计
- **面部**：圆形显示屏，显示 2D 动画眼睛和表情
- **表情**：开心、难过、困倦、好奇、惊讶、困惑、兴奋
- **动作**：可旋转/倾斜"看向"驾驶员
- **功能**：语音控制空调/车窗/音乐/导航，多乘客识别

### 6.2 UE5 实现方案

```
NOMI 机器人 Blueprint
├─ Static Mesh（球形身体 + 底座）
├─ Material（白色哑光 + 圆形面部区域）
├─ Widget Component（面部表情 UI）
│   ├─ 眼睛动画（Blink、Shape 变化）
│   ├─ 嘴巴动画（说话时开合）
│   └─ 表情切换（happy/nervous/surprised/celebrating）
├─ Rotator Component（头部旋转，跟踪驾驶员方向）
└─ Audio Component（语音播放）
```

### 6.3 评论系统 — 预生成评论池

```
评论池结构（JSON）：
{
  "overtake": {
    "first_place": [
      "漂亮！{player}超越了{rival}，现在领跑！",
      "精彩的超车！{player}抓住了内线机会！"
    ],
    "mid_pack": [
      "{player}又超了一辆！现在排第{position}！",
      "连续超车！{player}正在追赶领先集团！"
    ]
  },
  "drift": {
    "short": [
      "漂亮的漂移！{player}在{corner}展现了技术！"
    ],
    "long": [
      "天呐！{player}漂移了整整{duration}秒！这角度太完美了！"
    ]
  },
  "lap_complete": {
    "fastest": [
      "最快圈速！{player}跑了{time}！新纪录！"
    ],
    "normal": [
      "第{lap}圈完成，用时{time}。"
    ]
  },
  "nio_specific": {
    "ep9_boost": [
      "EP9 的四电机全开！1360匹马力瞬间释放！"
    ],
    "battery_swap": [
      "换电站就在前方！{player}要不要进去换块电池？"
    ],
    "nomi_intro": [
      "我是 NOMI，你的车载 AI 助手。准备好出发了吗？"
    ]
  }
}
```

评论变量：
- `{player}` — 玩家车辆名称
- `{rival}` — 被超越的对手名称
- `{position}` — 当前名次
- `{corner}` — 弯道名称
- `{duration}` — 漂移持续时间
- `{lap}` — 当前圈数
- `{time}` — 圈速时间

### 6.4 评论触发事件

| 事件 | 触发条件 | 评论数量 | 情绪倾向 |
|------|---------|---------|---------|
| 超车 | 名次提升 | 30+ 条 | 兴奋/鼓励 |
| 被超 | 名次下降 | 20+ 条 | 安慰/激励 |
| 漂移 | 侧滑角 > 15° 持续 > 1s | 25+ 条 | 兴奋 |
| 圈速完成 | 通过终点线 | 15+ 条 | 中性/鼓励 |
| 最快圈速 | 刷新个人最快 | 10+ 条 | 庆祝 |
| 高速 | 速度 > 200 km/h | 10+ 条 | 紧张/兴奋 |
| 碰撞 | 与其他车辆/护栏碰撞 | 15+ 条 | 安慰/幽默 |
| 领先 | 名次 = 1 | 10+ 条 | 兴奋 |
| 落后 | 名次 = 最后 | 10+ 条 | **安慰/激励（挫败感防护）** |
| 连续被超 | 30 秒内被超 2 次以上 | 10+ 条 | **安慰/鼓励（挫败感防护）** |
| 反复撞墙 | 10 秒内碰撞 3 次以上 | 5+ 条 | **幽默/建议（挫败感防护）** |
| 赛道特有 | 特定弯道/路段 | 每赛道 10+ 条 | 中性 |
| NIO 特有 | 换电站、电池、电机 | 20+ 条 | 品牌骄傲 |

**总计：500+ 条预生成评论**（其中失败/挫折场景安慰评论 ≥ 60 条）

### 6.5 NOMI 交互规则

| 规则 | 设定 |
|------|------|
| 评论冷却时间 | 最小间隔 3 秒，避免连续轰炸 |
| 队列机制 | 最多缓存 2 条待播评论，新评论替换最旧的 |
| 打断规则 | 高优先级事件（如碰撞）可打断低优先级评论 |
| 显示时长 | 根据文字长度 2-5 秒，淡出过渡 0.3 秒 |
| 用户设置 | 评论频率：高/中/低/关闭（默认：中） |
| 去重策略 | 最近 10 条不重复，直到评论池耗尽后重置 |
| 空态显示 | 无评论时显示 NOMI 默认待机表情 + 随机 idle 眨眼动画 |

---

## 七、UI/HUD 设计

### 7.1 科技风设计规范

**颜色系统**：

| 元素 | 颜色 | 说明 |
|------|------|------|
| 主背景 | #0A0E1A（深蓝黑） | 半透明面板 |
| 主强调色 | #00A1E0（NIO Blue） | 按钮、边框、高亮 |
| 次强调色 | #00D4FF（青色） | 速度数字、发光效果 |
| 成功色 | #7FFF00（绿色） | 完成、正向反馈 |
| 危险色 | #FF2244（红色） | 警告、碰撞 |
| 文字色 | #F0F0F0（白色） | 主要文字 |
| 次文字色 | #8899AA（灰色） | 辅助信息 |
| 色盲替代 | 蓝/橙 替代 红/绿 | 可访问性选项 |

**字体规范**：

| 层级 | 字号 | 用途 |
|------|------|------|
| H1 | 96px | 速度数字 |
| H2 | 36px | 名次、圈数 |
| H3 | 24px | 计时、赛道名 |
| Body | 18px | NOMI 评论、菜单文字 |
| Caption | 14px | 辅助信息 |

推荐字体：Rajdhani / Orbitron / Exo 2（科技感，免费 Google Fonts）

**按钮四态**：

| 状态 | 样式 |
|------|------|
| Normal | NIO Blue 边框 + 半透明背景 |
| Hover | 边框发光 + 背景亮度 +10% |
| Pressed | 背景亮度 -10% + 缩放 0.95 |
| Disabled | 灰色边框 + 50% 透明度 |

**间距系统**：4px 基准网格（所有间距为 4 的倍数）

**动效规范**：0.2-0.3s ease-out 过渡

### 7.2 HUD 布局

```
┌─────────────────────────────────────────────────────────────┐
│  [P1]  LAP 3/5    01:23.456    NIO City Circuit    [MAP]   │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
│  ┌─────────┐                                ┌──────────┐   │
│  │  NOMI   │                                │   312    │   │
│  │  (⊙‿⊙)  │                                │  km/h    │   │
│  │         │                                │ ▓▓▓▓░░░░ │   │
│  └─────────┘                                │ ▓▓░░░░░░ │   │
│                                             └──────────┘   │
│  "EP9 的四电机全开！"                   [加速踏板][制动踏板] │
└─────────────────────────────────────────────────────────────┘
```

### 7.3 HUD 状态矩阵

| HUD 元素 | 状态 1 | 状态 2 | 状态 3 | 状态 4 |
|---------|--------|--------|--------|--------|
| 速度表 | 静止：显示 0 + 驻车图标 | 正常：实时速度 | 超速(>250)：数字变红 | 倒车：显示 R + 倒车速度 |
| 名次 | 正常：P1/P2/... | 上升：闪烁绿色 | 下降：闪烁红色 | - |
| 圈数 | 正常：LAP 3/5 | 最后一圈：闪烁 | 完成：✓ 标记 | - |
| NOMI | 空态：待机表情 + 眨眼 | 评论中：说话表情 | 事件触发：惊讶/兴奋 | - |
| 小地图 | 默认：赛道俯视 + 玩家点 | 放大：显示弯道细节 | - | - |
| 加速踏板条 | 绿色填充，随踏板深度变化 | 色盲模式：蓝色 | - | - |
| 制动踏板条 | 红色填充，随踏板深度变化 | 色盲模式：橙色 | - | - |
| **Baja 模式差异** | 圈数 → 替换为"距终点 X.X km"或"进度 XX%" | 名次基于进度而非圈数 | 小地图显示全程路线 | - |

### 7.4 菜单系统

**菜单导航流程**：

```
主菜单
├─ [GT 模式] → 车库(GT) → 赛道选择 → 比赛设置 → 倒计时 → 比赛
├─ [NIO 模式] → 车库(NIO) → 赛道选择 → 比赛设置 → 倒计时 → 比赛
├─ [Baja 模式] → 车库(Baja) → 赛道选择 → 比赛设置 → 倒计时 → 比赛
├─ [设置] → 音量/画质/分辨率/控制映射
└─ [退出] → 确认弹窗

比赛中按 ESC → 暂停菜单
├─ [继续] → 返回比赛
├─ [重新开始] → 确认弹窗 → 重新加载
├─ [设置] → 暂停中可调设置
└─ [返回主菜单] → 确认弹窗（不保存进度）→ 主菜单

比赛结束 → 结算页面
├─ 本场成绩：名次、总用时、最佳圈速、与第一名差距
├─ 赛季积分变化（+N / -N 动画）
├─ 领奖台动画（前三名，3-5 秒，可跳过）
├─ Baja 模式适配：无"最佳圈速"，替换为"总用时"和"到达率"
└─ 操作按钮：重赛（默认焦点） / 返回车库 / 返回主菜单
```

**车库页面定义（7.4.1）**：

| 元素 | 说明 |
|------|------|
| 布局 | 左侧车辆卡片列表 + 右侧 3D 预览（可旋转） |
| 车辆选择 | 左右切换卡片，选中高亮 + 已选标识 |
| 车辆信息 | 名称、功率、扭矩、0-100、极速、驱动类型 |
| 模式约束 | NIO 模式下非 NIO 车辆置灰（不可选）；Baja 模式下仅显示越野车型 |
| 车漆自定义 | 可选颜色（预设 5-8 种），实时预览 |
| 默认焦点 | 第一辆可用车辆 |

**比赛设置页面定义（7.4.2）**：

| 配置项 | 默认值 | 范围 | 说明 |
|--------|--------|------|------|
| AI 对手数量 | 5 | 0-15 | Baja 模式上限 8 |
| AI 难度 | MEDIUM | EASY / MEDIUM / HARD | - |
| 圈数（GT/NIO） | 3 | 1-10 | Baja 模式不显示 |
| 天气 | 晴天 | 晴天 / 黄昏 / 夜晚 | 仅影响光照 |

**加载状态设计（7.6）**：

| 元素 | 说明 |
|------|------|
| 加载页面 | 赛道预览图 + 进度条（百分比） + 加载提示文案（随机 tips） |
| 加载时间目标 | < 10 秒（SSD），< 20 秒（HDD） |
| 取消操作 | 加载期间按 Esc 取消，返回赛道选择 |
| 首次 vs 后续 | 首次加载较长（着色器编译），后续加载有缓存加速 |

**异常状态处理**：

| 场景 | 系统响应 | 用户可见提示 | 恢复路径 |
|------|---------|------------|---------|
| 车辆卡死 | 检测 5 秒无位移 | "按 R 重置车辆（3 秒倒计时）" | 自动复位到赛道最近路径点 |
| 翻车 | 检测车身倾角 > 80° | "按 R 重置车辆" | 自动翻正 + 复位 |
| 赛道加载失败 | 捕获加载异常 | "赛道加载失败，请重试" | 返回赛道选择 |
| 存档损坏 | 校验存档完整性 | "存档已损坏，已恢复到上一个有效存档" | 回退到备份存档 |
| AI 行为异常 | 检测 AI 静止 > 10 秒 | 无提示（静默处理） | AI 传送回赛道路径点 |

### 7.5 可访问性设计

| 特性 | 说明 |
|------|------|
| 色盲模式 | 提供蓝/橙替代红/绿的配色方案（设置中切换） |
| 键盘导航 | 所有菜单支持 Tab 切换、Enter 确认、Esc 返回 |
| NOMI 字幕 | NOMI 语音评论同步显示文字（默认开启） |
| 字体大小 | 小/中/大三档可调 |
| 高对比度 | 可选高对比度 HUD 模式 |

---

## 八、音频方案

### 8.1 音频资产来源

| 音效类型 | 来源 | 许可证 | URL |
|---------|------|--------|-----|
| 引擎采样（V8/V6） | Freesound.org | CC0/CC-BY | https://freesound.org |
| 电机嗡鸣 | Freesound.org | CC0 | https://freesound.org |
| 轮胎尖叫 | Freesound.org | CC0/CC-BY | https://freesound.org |
| 碰撞声 | Freesound.org | CC0 | https://freesound.org |
| 风噪 | Freesound.org | CC0 | https://freesound.org |
| UI 音效 | Kenney Audio | CC0 | https://kenney.nl/assets?t=audio |
| 高质量游戏音效 | Sonniss GDC Bundle | 免费商用 | https://sonniss.com/gameaudiogdc |

### 8.2 MetaSound 实时处理

```
音频管线：
Engine Sample (WAV)
    │
    ▼
MetaSound Graph
    ├─ Pitch Shift（根据 RPM 实时变调）
    ├─ Low Pass Filter（根据加速踏板开度）
    ├─ Volume Envelope（根据负载）
    ├─ Spatialization（3D 空间音效）
    └─ Reverb（根据环境：隧道/开阔/室内）
    │
    ▼
Audio Output
```

NIO 电动车音频：
- 主音源：电机高频嗡鸣（合成 + 采样混合）
- 辅助：风噪（速度相关）、轮胎噪音（路面相关）
- 无引擎声、无排气声

---

## 九、开发路线图

> **总工期：24-32 周（6-8 个月，单人开发）**

### Phase 0：许可证验证与 Spike（1-1.5 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 0.1 资产许可证审计 | 确认 CARLA 发布包资产的许可证，确认可用于非商业衍生项目 | 明确许可或不可用 | 1 天 |
| 0.2 技术路径 Spike | (A) Fork CARLA 裁剪编译 vs (B) 新建 UE5 + 导入资产 | 两条路径可行性对比 | 3-5 天 |
| 0.3 macOS 编译验证 | 在 macOS 上编译 UE5 项目，确认 Metal 渲染基础可用 | macOS 可编译运行 | 1-2 天 |
| 0.4 性能基准测试 | 用 CARLA 原版城市地图在目标硬件跑帧率测试（统一场景：城市赛道 + 4 辆 AI + 夜间光照） | 记录基准数据 | 1 天 |

**性能基准目标**：

| 平台 | 目标硬件 | 目标帧率 | 最低可接受 | 分辨率 |
|------|---------|---------|-----------|--------|
| Windows | RTX 3060 / i5-12400 / 16GB | 平均 60 FPS，1% Low ≥ 45 | 平均 45 FPS | 1080p Medium |
| macOS | M1 Pro / 16GB | 平均 45 FPS，1% Low ≥ 30 | 平均 30 FPS | 1800p Medium |
| 0.5 路径决策 | 基于 Spike 结果选定技术路径，更新计划 | 最终路径确认 | 0.5 天 |

### Phase 1：基础搭建（4-5 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 1.1 搭建项目 | 根据 Phase 0 选定的路径搭建 UE5 项目 | 项目可编译 | 1 周 |
| 1.2 导入城镇地图 | 加载 CARLA Town03，确认渲染正常 | 城市环境画质达标 | 3-5 天 |
| 1.3 导入车辆 | 加载 CARLA 自带车辆 + EP9 模型 | 车辆可渲染 | 3-5 天 |
| 1.4 配置 Chaos Vehicles | 为 1 辆 CARLA 默认车 + EP9 配置物理参数（共 2 辆，其余 GT 车辆挪到 Phase 2.7） | 2 辆车可驾驶，有基本操控感 | 1-1.5 周 |
| 1.5 macOS 兼容性检查 | 确认材质、光照在 Metal 上正常显示 | macOS 无明显渲染错误 | 2-3 天 |
| 1.6 Git LFS 资产归档 | 所有资产本地归档 + 许可证记录 | 资产仓库完整 | 1 天 |

**参考资源**：
- CARLA：https://github.com/carla-simulator/carla/releases
- Chaos Vehicles 文档：UE5 官方文档
- EngineSimulatorPlugin：https://github.com/nicholas477/EngineSimulatorPlugin
- Unreal-NebulousVehicle：https://github.com/MrRobinOfficial/Unreal-NebulousVehicle

### Phase 2：车辆系统（4-5 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 2.1 下载 NIO 模型 | 从 Sketchfab 下载 ET7/ES7，本地归档 | 文件下载成功 + 归档 | 1 天 |
| 2.2 Blender 处理 + LOD | 拓扑检查、UV、材质槽、LOD 三级生成 | 模型在 Blender 中显示正确，LOD 面数达标 | 1-1.5 周 |
| 2.3 UE5 导入 | 导入 FBX（含 LOD），创建 Vehicle Blueprint | 车辆在 UE5 中可渲染，LOD 切换正常 | 2-3 天 |
| 2.4 物理配置 | 配置每辆 NIO 车的 Chaos Vehicles 参数 | 驾驶手感接近真实数据 | 1-1.5 周 |
| 2.5 车漆材质 | UE5 PBR 材质：金属漆、清漆、环境反射 | 车漆在不同光照下看起来真实 | 3-5 天 |
| 2.6 车灯系统 | 大灯、尾灯、刹车灯、转向灯 | 灯光随输入变化 | 2-3 天 |
| 2.7 GT 车辆配置 | 配置 CARLA 自带车辆的 Chaos Vehicles 参数 | GT 模式可玩 | 2-3 天 |
| 2.8 macOS 材质验证 | 确认车漆/车灯在 Metal 上显示正确 | macOS 无材质错误 | 1 天 |

**参考资源**：
- Sketchfab 模型：见第三节表格
- SketchfabDownloader：https://github.com/SaltyFishOTL/SketchfabDownloader

### Phase 3：赛道制作（6-8 周）

**优先级分批**：先交付 3 条核心赛道，剩余 2 条后续版本。

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 3.1 城市赛道 1 | CARLA Town03 改造为 NIO City Circuit | 完整赛道可跑圈 | 2 周 |
| 3.2 椭圆赛道 | UE5 程序化生成 + 看台模型（最简单） | 完整赛道可跑圈 | 1 周 |
| 3.3 城市赛道 2 | CARLA Town06 改造为 Shanghai Pudong | 完整赛道可跑圈 | 2 周 |
| 3.4 AI 路径 | 每条赛道创建 Spline + AI 路径点 | AI 能沿路径行驶 | 3-5 天 |
| 3.5 检查点系统 | 每条赛道设置检查点触发器 | 圈数验证正确 | 2-3 天 |
| 3.6 环境光照 + HDR | 每条赛道配置光照预设 + Poly Haven HDR 天空盒 | 画面氛围符合设计 | 2-3 天 |
| 3.7 macOS 光照验证 | 确认 Lumen（软件）在 Metal 上正常 | macOS 光照无异常 | 1 天 |

**后续版本（Phase 3.5）**：
- 山地赛道（UE5 地形 + Megascans，2-3 周）
- 沙漠赛道（Megascans + 点对点，2 周）

**参考资源**：
- Poly Haven：https://polyhaven.com/hdris
- ambientCG：https://ambientcg.com
- Quixel Megascans (Fab)：https://fab.com
- UE City Generator：https://github.com/Yorshka-Vermilion/UnrealEngine---City-Generator

### Phase 4：游戏循环（5-6 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 4.1 比赛流程 | IDLE → 预赛 → 倒计时 → 比赛 → 结束 | 完整比赛流程可走通 | 1 周 |
| 4.2 AI 对手 — 路径跟随 | Spline + 速度控制，AI 能完成一圈 | AI 不撞墙，能完赛 | 1-1.5 周 |
| 4.3 AI 对手 — 超车/避让 | 射线检测 + 行为树，简单超车决策 | AI 有基本竞争能力 | 1 周 |
| 4.4 AI 难度分级 | 3 档难度（速度系数、刹车距离、转向噪声） | 不同难度有明显差异 | 3-5 天 |
| 4.5 计时 + 名次系统 | 圈速、总用时、最快圈速、基于进度的实时排名 | 计时和名次准确 | 3-5 天 |
| 4.6 倒计时 + 结算 | 5 灯倒计时 + 结算画面（成绩、积分、领奖台） | 流程完整 | 3-5 天 |
| 4.7 赛季系统 | 多场比赛累计积分 | 赛季积分排名正确 | 1 周 |

**参考资源**：
- Car Race Championship：https://github.com/anshulyadav32/car-race-championship
- 原版 nomi-racing 的 race_manager.gd 和 race_scene.gd

### Phase 5：体验打磨（4-5 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 5.1 NOMI 模型 | 球形机器人 Blueprint + 表情动画 | NOMI 在仪表台显示正确 | 1 周 |
| 5.2 评论池生成 | LLM 批量生成 500+ 条（2 天）+ 人工审核/变量校验/情绪映射（3 天） | JSON 文件完整，变量占位符正确 | 1 周 |
| 5.3 评论匹配引擎 | 事件触发 → 优先级排序 → 冷却/去重 → 播放 | 评论在正确时机出现，无重复 | 3-5 天 |
| 5.4 科技风 HUD | 速度、名次、圈速、小地图、NOMI + 状态矩阵 | HUD 各状态显示正确 | 1 周 |
| 5.5 菜单系统 | 主菜单、车库、赛道选择、设置、结算、暂停 | 所有菜单可操作，流程正确 | 1-1.5 周 |
| 5.6 音效 | 电机嗡鸣 + 风噪 + 轮胎声 + 碰撞声（Freesound.org） | 音效完整 | 3-5 天 |
| 5.7 粒子系统 | 轮胎烟雾、碰撞火花 | 粒子效果正确 | 2-3 天 |
| 5.8 后处理 + 摄像机 | SSAO、Bloom、运动模糊 + 追踪摄像机 + FOV 变化 | 画面质感提升 | 3-5 天 |
| 5.9 可访问性 | 色盲模式、键盘导航、NOMI 字幕、字体大小调节 | 可访问性功能可用 | 2-3 天 |

**参考资源**：
- Freesound.org：https://freesound.org
- Sonniss GDC Bundle：https://sonniss.com/gameaudiogdc
- Kenney Audio：https://kenney.nl/assets?t=audio

### Phase 6：发布准备（3-4 周）

| 任务 | 详情 | 验证标准 | 预估时间 |
|------|------|---------|---------|
| 6.1 性能优化 | LOD、遮挡剔除、纹理流送 | 目标硬件稳定达标（见性能基准表） | 1 周 |
| 6.2 画质预设 | 低/中/高三档画质设置 | 三档都可正常运行 | 3-5 天 |
| 6.3 跨平台测试 | Windows + macOS 完整测试 | 两个平台无严重 Bug | 1 周 |
| 6.4 存档系统 | 最佳圈速、赛季进度、解锁状态 | 存档读写正确，损坏可恢复 | 2-3 天 |
| 6.5 Bug 修复 | 全面测试 + 修复 | 无严重 Bug | 1 周 |
| 6.6 打包发布 | Windows + macOS 打包 | 可执行文件正常运行 | 2-3 天 |
| 6.7 ET5（可选） | 从 ET7 改造或搜索其他平台 | ET5 可驾驶（或决策砍掉） | 1 周 |

---

## 十、降级方案

| 风险 | 降级方案 | 影响 |
|------|---------|------|
| CARLA 资产不可用（许可证/技术） | 使用 UE5 City Sample + Quixel Megascans 自建城市 | 工期 +2-3 周 |
| CARLA Fork 裁剪失败 | 改用"仅提取资产 + 新建项目"路径 | 工期 +1 周（Spike 已覆盖） |
| Sketchfab NIO 模型下架 | 本地 Git LFS 已归档（资产归档策略） | 无影响（已备份） |
| ET5 AI 生成/改造失败 | 砍掉 ET5，仅交付 3 款 NIO 车 | 车型从 4 款减为 3 款 |
| Chaos Vehicles 学习成本过高 | 回退到 CARLA 默认物理 + 微调 | 驾驶手感降级 |
| macOS Metal 渲染不兼容 | 降低画质预设，关闭 Lumen/SSR | Mac 画质降级 |
| Freesound 音效质量不足 | 使用 Kenney CC0 音效 + MetaSound 合成 | 音效风格化 |

---

## 十一、系统接口定义

### 11.1 车辆状态数据结构

```cpp
struct FVehicleState {
    float SpeedKmh;           // 当前速度 (km/h)
    float ThrottleInput;      // 加速踏板输入 (0-1)
    float BrakeInput;         // 制动踏板输入 (0-1)
    float SteeringInput;      // 转向输入 (-1 到 1)
    float RPM;                // 电机转速
    int32 Gear;               // 当前档位（电动车固定 1）
    bool bIsDrifting;         // 是否在漂移
    float SlipAngle;          // 侧滑角
    bool bIsGrounded;         // 是否着地
    FVector Position;         // 世界坐标
    FRotator Rotation;        // 朝向
};
```

### 11.2 比赛事件枚举

```cpp
enum class ERaceEvent : uint8 {
    Overtake,           // 超车
    Overtaken,          // 被超
    DriftStart,         // 开始漂移
    DriftEnd,           // 结束漂移
    LapComplete,        // 圈速完成
    FastestLap,         // 最快圈速
    HighSpeed,          // 高速 (>200 km/h)
    Collision,          // 碰撞
    FirstPlace,         // 领先
    LastPlace,          // 落后
    RaceStart,          // 比赛开始
    RaceFinish,         // 比赛结束
    CountdownStart,     // 倒计时开始
    CheckpointPassed,   // 通过检查点
    VehicleStuck,       // 车辆卡死
    VehicleFlipped,     // 车辆翻车
};
```

### 11.3 HUD 数据绑定接口

```
HUD Widget ← 绑定 → PlayerController → 获取 VehicleState
├─ SpeedText ← SpeedKmh
├─ PositionText ← RaceManager.GetPlayerPosition()
├─ LapText ← RaceManager.GetCurrentLap() / GetTotalLaps()
├─ TimerText ← RaceManager.GetRaceTimer()
├─ ThrottleBar ← ThrottleInput
├─ BrakeBar ← BrakeInput
├─ MiniMap ← Position + TrackSpline
└─ NOMI ← CommentaryEngine.GetNextComment()
```

### 11.4 RaceManager 状态机

```
IDLE → LOADING → COUNTDOWN → RACING → FINISHED
  ↑        ↑          ↑          ↑         │
  └────────┴──────────┴──────────┴─────────┘
                    (重新开始)
```

**状态转换条件**：
- IDLE → LOADING：用户在比赛设置页面点击"开始比赛"
- LOADING → COUNTDOWN：赛道加载完成（加载页面消失）
- COUNTDOWN → RACING：5 灯倒计时结束
- RACING → FINISHED：第一名通过终点线 + 30 秒超时（或所有玩家完赛）
- FINISHED → IDLE：结算页面点击"返回主菜单"

### 11.5 接口细节补充

**FVehicleState 更新频率**：每帧（Tick）更新，HUD 绑定通过 Tick 事件获取最新状态。

**ERaceEvent 传递机制**：使用 UE5 Delegate（多播委托）。RaceManager 广播事件，NOMI CommentaryEngine 和 HUD 通过绑定委托订阅。

**CommentaryEngine 接口**：
```cpp
// 输入：ERaceEvent + 当前 FVehicleState
// 输出：评论文本（FString）+ 优先级（int32）
// 逻辑：事件类型 → 评论池筛选 → 冷却检查 → 去重检查 → 随机选择
FString UCommentaryEngine::GetNextComment(ERaceEvent Event, const FVehicleState& State);
```

**AI 控制器接口**：
```cpp
// AI 控制器继承自 AController，通过 WriteInput() 写入车辆输入
// 与 PlayerController 并行，共享 FVehicleState 结构
void AAICarController::WriteInput(float& Throttle, float& Brake, float& Steering);
```

### 11.6 存档系统接口

```
SaveSystem
├─ SaveFormat: JSON（可读性优先，单人项目性能无瓶颈）
├─ AtomicWrite: 先写 .tmp 再 rename（保证原子性，断电不损坏）
├─ BackupPolicy: 保留最近 3 份（save.json, save.backup-1.json, save.backup-2.json），轮转覆盖
├─ IntegrityCheck: 文件头 version + CRC32 校验
├─ Recovery: 校验失败 → 尝试 backup-1 → backup-2 → 重置为默认
└─ 存档内容:
    ├─ best_laps: {track_id: {vehicle_id: best_time_ms}}
    ├─ season_progress: {mode: {round: N, points: {team: score}}}
    ├─ settings: {volume, graphics_preset, controls}
    └─ version: "1.0.0"
```

**存档时机**：Phase 4 实现基础存档（赛季进度），Phase 6 完善（最佳圈速、设置）。

---

## 十二、AI 对手技术方案

### 分层设计

| 层级 | 功能 | 实现方式 |
|------|------|---------|
| L1：导航 | 沿赛道路径行驶 | Spline + 速度控制（曲线段减速） |
| L2：避让 | 前方有车/障碍时转向 | 射线检测 + 侧行为 |
| L3：超车 | 有机会时主动超车 | 检测前方车辆速度差 + 内/外线判断 |
| L4：难度 | 难度系数调节 | 速度系数(EASY 0.85 / MED 0.95 / HARD 1.0)、刹车距离、转向噪声 |

### 难度参数

| 参数 | EASY | MEDIUM | HARD |
|------|------|--------|------|
| 速度系数 | 0.85 | 0.95 | 1.0 |
| 刹车提前距离 | 1.5x | 1.2x | 1.0x |
| 转向噪声 | ±5° | ±3° | ±1° |
| 橡皮筋（最后→第一） | +6% | +4% | +2% |

---

## 十三、测试策略

| 测试类型 | 频率 | 方法 | 自动化程度 |
|---------|------|------|-----------|
| 编译验证 | 每次代码提交 | UE5 编译通过 | 自动化（本地脚本） |
| 冒烟测试 | 每个 Phase 结束 | 启动 → 加载赛道 → 可驾驶 → 完成一圈 | 半自动化（脚本启动 + 人工验证） |
| 性能基准 | Phase 1/3/6 | 目标帧率测试（使用 Phase 0.4 统一测试场景） | 自动化（stat fps 日志采集） |
| 跨平台测试 | Phase 3/6 | Windows + macOS 双平台验证 | 手动 |
| 回归测试 | Bug 修复后 | 验证修复不引入新问题 | 手动 |
| NOMI 评论测试 | Phase 5 | 触发准确率、冷却机制、去重机制 | 手动 |
| AI 行为测试 | Phase 4 | 完赛率、碰撞频率、难度差异感知 | 手动 |

---

## 十四、风险评估与应对

| 风险 | 概率 | 影响 | 应对方案 | 降级方案 |
|------|------|------|---------|---------|
| CARLA 资产许可证不允许 | 中 | 高 | Phase 0 许可证审计 | → UE5 City Sample + Quixel 自建 |
| CARLA Fork 裁剪失败 | 中 | 高 | Phase 0 Spike 验证 | → 仅提取资产 + 新建项目 |
| Sketchfab 模型下架 | 低 | 中 | Git LFS 本地归档 | 无影响（已备份） |
| Chaos Vehicles 学习曲线 | 中 | 中 | 参考 EngineSimulatorPlugin 源码 | → CARLA 默认物理微调 |
| macOS Metal 渲染问题 | 中 | 中 | Phase 0 macOS 编译验证 | → 降低画质，关闭 Lumen |
| ET5 模型质量不足 | 中 | 低 | 最后阶段尝试改造 | → 砍掉 ET5，仅交付 3 款 |
| 赛道制作耗时 | 中 | 中 | 优先 3 条核心赛道 | 山地/沙漠移至后续版本 |

---

## 十五、资源总览

### 代码仓库

| 仓库 | 用途 | 链接 |
|------|------|------|
| CARLA (ue5-dev) | 项目基础 | https://github.com/carla-simulator/carla |
| EngineSimulatorPlugin | 引擎物理 | https://github.com/nicholas477/EngineSimulatorPlugin |
| Unreal-NebulousVehicle | 车辆基础类 | https://github.com/MrRobinOfficial/Unreal-NebulousVehicle |
| Car Race Championship | 架构参考 | https://github.com/anshulyadav32/car-race-championship |
| UE5-Vehicle-Template | 交通系统 | https://github.com/AnastasisMarinos/UE5-Vehicle-Template |
| TRELLIS | AI 3D 生成 | https://github.com/microsoft/TRELLIS |
| TripoSR | AI 3D 生成 | https://github.com/VAST-AI-Research/TripoSR |
| SketchfabDownloader | 模型下载 | https://github.com/SaltyFishOTL/SketchfabDownloader |
| UE City Generator | 城市生成 | https://github.com/Yorshka-Vermilion/UnrealEngine---City-Generator |

### 资产平台

| 平台 | 内容 | 许可证 | 链接 |
|------|------|--------|------|
| Sketchfab | NIO 车辆模型 | CC BY | https://sketchfab.com |
| Poly Haven | HDR 天空盒 + PBR 贴图 | CC0 | https://polyhaven.com |
| ambientCG | PBR 材质 | CC0 | https://ambientcg.com |
| Quixel Megascans (Fab) | 岩石/植被/路面 | 免费用于 UE | https://fab.com |
| Freesound.org | 音效采样 | CC0/CC-BY | https://freesound.org |
| Sonniss | 游戏音效包 | 免费商用 | https://sonniss.com/gameaudiogdc |
| Kenney | UI 音效 | CC0 | https://kenney.nl |

### 原版项目参考

| 文件 | 用途 |
|------|------|
| nomi-racing/cars/car_base.gd | 物理参数参考（扭矩曲线、空动、漂移） |
| nomi-racing/cars/ai_car_controller.gd | AI 系统参考（路径跟随、避障、橡皮筋） |
| nomi-racing/autoloads/race_manager.gd | 比赛流程参考（状态机、检查点、名次） |
| nomi-racing/nomi/nomi_controller.gd | NOMI 系统参考（状态机、评论触发） |
| nomi-racing/nomi/nomi_commentary.gd | 评论文本参考 |
| nomi-racing/environment/sky_manager.gd | 环境系统参考（光照预设、后处理） |

---

## 附录 A：Cycle 1 UX 改进提案（2026-06-05）

> 基于 Cycle 1 代码审查，以下 5 项改进按玩家体验影响排序。

### A.1 轮胎物理断联（优先级 1 — CRITICAL）

**问题：** Pacejka 轮胎模型每帧计算力，但 `ApplyTireForces()` 是空操作。车辆实际使用默认 Chaos 物理行驶，轮胎类型、路面抓地力、温度和磨损全部是装饰性代码。

**影响：** 核心物理体验与宣传不符，玩家无法感知不同轮胎/路面差异。

**方案：** 将 Pacejka 计算结果接入 Chaos Vehicles 的轮胎力接口。需要研究 `UChaosVehicleMovementComponent` 的自定义轮胎力 API。

**预估工期：** 1-2 周

### A.2 AI 橡皮筋不对称（优先级 2 — HIGH）

**问题：** AI 落后时获得 25% 速度加成，领先时仅减速 5%。这种不对称造成经典的橡皮筋挫败感——玩家永远甩不掉 AI，但 AI 可以轻松超越玩家。

**影响：** 公平性感知差，玩家感到"被系统惩罚"。

**方案：** 将追赶加成和领先减速调整为对称值（如 10%/10%），或改为基于距离的平滑曲线。

**预估工期：** 0.5 天（参数调整）

### A.3 HUD 未填充数据（优先级 3 — HIGH）

**问题：** HUD Widget 已构建，但在比赛期间从未被填充实时数据（速度、名次、圈数）。

**影响：** 玩家在比赛中没有速度/位置反馈。

**方案：** 在 HUD Tick 中绑定 VehicleState 和 RaceManager 数据。参考 PLAN.md 第七节 HUD 数据绑定接口。

**预估工期：** 2-3 天

### A.4 结算画面未显示（优先级 4 — HIGH）

**问题：** 结算画面代码存在，但比赛结束后从未被调用显示。

**影响：** 玩家没有完成感/奖励循环。

**方案：** 在 RaceManager 状态从 RACING 转为 FINISHED 时，创建并显示结算 Widget。

**预估工期：** 2-3 天

### A.5 赛后流程缺失（优先级 5 — MEDIUM）

**问题：** 比赛结束后没有重赛、下一场比赛或返回车库的选项，玩家只能返回主菜单。

**影响：** 流畅度差，增加不必要的操作步骤。

**方案：** 在结算画面添加"重赛"（默认焦点）、"返回车库"、"返回主菜单"三个按钮。

**预估工期：** 1 天

### 优先级总结

| 优先级 | 改进项 | 影响 | 工期 | 类型 |
|--------|--------|------|------|------|
| 1 | 轮胎物理接入 | CRITICAL | 1-2 周 | 功能修复 |
| 2 | AI 橡皮筋对称 | HIGH | 0.5 天 | 参数调优 |
| 3 | HUD 数据绑定 | HIGH | 2-3 天 | 功能连接 |
| 4 | 结算画面显示 | HIGH | 2-3 天 | 功能连接 |
| 5 | 赛后流程 | MEDIUM | 1 天 | 功能连接 |

**关键洞察：** 5 项中有 3 项是"代码存在但未连接"的问题，属于最高性价比的改进。

---

## 十六、许可证合规

| 资产 | 许可证 | 要求 |
|------|--------|------|
| CARLA 代码 | MIT | 保留版权声明 |
| NIO Sketchfab 模型 | CC BY 4.0 | 在游戏中标注原作者 |
| Poly Haven | CC0 | 无要求 |
| ambientCG | CC0 | 无要求 |
| Freesound | CC0/CC-BY | CC-BY 需标注来源 |
| Kenney | CC0 | 无要求 |
| Sonniss | 免费商用 | 保留 Sonniss 标注（推荐） |
| Quixel Megascans | UE EULA | 仅限 UE 项目使用 |

**注意**：
- 本项目为非商业用途，CC BY 许可的模型可在 Credits 中标注原作者后使用
- 避免使用 CC BY-NC-SA 许可的资产（如 2022 NIO ET7 模型）
- Phase 0 需确认 CARLA 发布包资产的具体许可证条款
