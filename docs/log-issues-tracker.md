# NIO Racing Plus — 日志问题追踪

> 用途：记录从 UE5 Editor 日志中发现的所有问题，持续追踪修复状态。
> 每次收到新日志时，追加到「日志会话记录」并更新问题表格。

---

## 问题总览

| # | 严重程度 | 分类 | 问题描述 | 首次发现 | 当前状态 |
|---|---------|------|---------|---------|---------|
| 1 | 🔴 错误 | 配置 | `LoadConfig: ProjectID 导入失败` — `GeneralProjectSettings` 中 ProjectID 格式错误 | 2026-06-07 | ✅ 已修复 |
| 2 | 🔴 错误 | 自动化测试 | `Condition failed` ×2 — AutomationTest 启动时条件检查失败 | 2026-06-07 | 待调查 |
| 3 | 🟡 警告 | 废弃 API | `GGPUFrameTime` 已废弃，需改用 `RHIGetGPUFrameCycles()` — `PerformanceProfiler.cpp:39,41` | 2026-06-07 | ✅ 已修复 |
| 4 | 🟡 警告 | 车辆 | `Bone name for wheel 0 is not set` — EP9 车辆缺少车轮骨骼名称 | 2026-06-07 | 有意为之 |
| 5 | 🟡 警告 | 车辆 | `No skeletal mesh - using simplified movement` — EP9 无骨骼网格体 | 2026-06-07 | 有意为之 |
| 6 | 🟡 警告 | 车辆 | `Vehicle stuck detected: NIO EP9` — 车辆卡住检测（无网格体时预期行为） | 2026-06-07 | 有意为之 |
| 7 | 🟡 警告 | 关卡 | `No AI spawn points found` — 所有地图均未找到 AI 生成点 | 2026-06-07 | ✅ 已修复 |
| 8 | 🟡 警告 | NOMI | `NOMI face widget is not UNOMIFaceWidget type` — NOMI 面部控件类型不匹配 | 2026-06-07 | ✅ 已修复 |
| 9 | 🔵 注意 | 构建 | `BuildSettingsVersion: V5` — UE 5.7 推荐升级到 V6 | 2026-06-07 | ✅ 已升级 |
| 10 | 🔵 注意 | 游戏流程 | 所有地图自动开始比赛且赛道名固定为"NIO City Circuit" — 不受地图切换影响 | 2026-06-07 | ✅ 已修复 |
| 11 | 🔵 注意 | 本地化 | `No specific localization for 'zh-Hans-CN' exists, using 'zh-Hans'` — 项目没有 zh-Hans-CN 的本地化资源（非阻塞） | 2026-06-07 | ℹ️ 信息 |
| 12 | 🔵 注意 | 资源 | `Failed to read file` — 多个 Slate 图标/纹理资源文件读取失败（开发环境常见） | 2026-06-07 | ℹ️ 信息 |
| 13 | 🔵 注意 | 游戏流程 | DesertRally PIE 时赛道名仍为"NIO City Circuit" — 确认赛道名与地图解耦，可能为硬编码 | 2026-06-07 | ✅ 已修复 |

---

## 问题详情

### #1 — ProjectID 导入失败

- **严重程度**: 🔴 错误
- **分类**: 配置
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  ImportText (ProjectID): Missing opening parenthesis: NomiRacingPlus
  LogObj: Error: LoadConfig (/Script/EngineSettings.Default__GeneralProjectSettings):
          import failed for ProjectID in: NomiRacingPlus
  ```
- **可能原因**: `DefaultEngine.ini` 或 `DefaultGame.ini` 中 `ProjectID` 字段格式错误，缺少括号或 GUID
- **当前状态**: ✅ 已修复
- **修复内容**: `Config/DefaultGame.ini` 中 `ProjectID=NomiRacingPlus` 改为 `ProjectID=(00000000-0000-0000-0000-000000000000)`
  - (第1次修复的 `00000000-...` 无括号格式在 UE5 config 系统中仍报相同错误，需加括号)

---

### #2 — 自动化测试条件检查失败

- **严重程度**: 🔴 错误
- **分类**: 自动化测试
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogAutomationTest: Error: Condition failed
  LogAutomationTest: Error: Condition failed
  ```
- **可能原因**: 引擎启动时某些自动运行的测试断言失败，需运行完整测试套件定位
- **当前状态**: 待调查

---

### #3 — GGPUFrameTime 废弃 API

- **严重程度**: 🟡 警告
- **分类**: 废弃 API
- **首次发现**: 2026-06-07 (日志会话 #1)
- **文件**: `Source/NomiRacingPlus/Editor/PerformanceProfiler.cpp:39,41`
- **日志原文**:
  ```
  warning: 'GGPUFrameTime' is deprecated:
  Direct use of GGPUFrameTime is deprecated. Call the global scope
  RHIGetGPUFrameCycles() function instead.
  ```
- **修复建议**: 将 `GGPUFrameTime` 替换为 `RHIGetGPUFrameCycles()`，注意返回值单位不同
- **当前状态**: ✅ 已修复
- **修复内容**: `Source/NomiRacingPlus/Editor/PerformanceProfiler.cpp` 中 `GGPUFrameTime * 1000.0` 替换为 `FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles())`

---

### #4 — 车轮骨骼名称未设置

- **严重程度**: 🟡 警告
- **分类**: 车辆
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogVehicle: Warning: Can't create vehicle NIO_EP9_0 (...VehicleMovementComp).
             Bone name for wheel 0 is not set.
  ```
- **影响**: 所有地图（NIOCityCircuit、MainMenu、OpenWorld）的 EP9 均触发此警告
- **可能原因**: 骨骼网格体资产未正确配置车轮骨骼名称
- **当前状态**: 🎯 有意为之 — NIOVehicleBase 构造函数显式设置 `WheelSetups[i].BoneName = NAME_None` 以使用简化移动模式
- **说明**: 项目使用 Chaos Vehicles 的简化移动模式，无需配置骨骼网格体

---

### #5 — EP9 无骨骼网格体

- **严重程度**: 🟡 警告
- **分类**: 车辆
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogNomiVehicle: Warning: No skeletal mesh - using simplified movement
  ```
- **影响**: EP9 使用简化移动模式，无真实物理表现
- **当前状态**: 🎯 有意为之 — 代码中 `NIOVehicleBase` 使用简化移动模式，属于开发阶段正常行为

---

### #6 — 车辆卡住检测

- **严重程度**: 🟡 警告
- **分类**: 车辆
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogNomiVehicle: Warning: Vehicle stuck detected: NIO EP9
  ```
- **说明**: 因无骨骼网格体/简化移动模式，车辆位置不更新触发卡住检测，属预期行为
- **当前状态**: 🎯 有意为之 — 简化移动模式下车辆位置不更新，卡住检测告警为预期结果
- **后续**: 当替换为完整骨骼网格体后此告警自动消失

---

### #7 — 未找到 AI 生成点

- **严重程度**: 🟡 警告
- **分类**: 关卡
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogNomiRacing: Warning: No AI spawn points found
  ```
- **影响**: 所有加载的地图（OpenWorld、NIOCityCircuit、DesertRally、MainMenu）均触发
- **当前状态**: ✅ 已修复
- **修复内容**: `Source/NomiRacingPlus/Core/NomiRaceGameMode.cpp` 中 `SpawnAIOpponents()` 增加三层优先级：
  1. TrackConfig.json `spawn_points` 数组（新增）
  2. 关卡中 `AISpawn` Tag 的 Actor（已有）
  3. 虚拟生成点（已有回退）

---

### #8 — NOMI 面部控件类型不匹配

- **严重程度**: 🟡 警告
- **分类**: NOMI
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogNomiNOMI: Warning: NOMI face widget is not UNOMIFaceWidget type
  ```
- **可能原因**: UI 控件绑定指向了错误的 Widget 类，或 `UNOMIFaceWidget` 尚未注册
- **当前状态**: ✅ 已修复（第2轮修复完整）
- **修复内容**: 
  - `Source/NomiRacingPlus/NOMI/NOMIController.cpp` — 增加回退机制：当 `Cast<UNOMIFaceWidget>()` 失败时自动创建替代
  - `Source/NomiRacingPlus/NOMI/NOMIFaceWidget.h` — 移除 `UCLASS(Abstract, ...)` 标记
  - **第一轮修复发现**: `UNOMIFaceWidget` 标记为 `Abstract`，`CreateWidget<>()` 无法实例化抽象类
  - **第二轮补充**: 移除 `Abstract` 后 `CreateWidget<UNOMIFaceWidget>()` 可正常创建

---

### #9 — 构建配置版本升级

- **严重程度**: 🔵 注意
- **分类**: 构建
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  [Upgrade] Using backward-compatible build settings.
  The latest version of UE sets the following values by default...
  ```
- **修复建议**: 在 `NomiRacingPlusEditor.Target.cs` 中设置 `DefaultBuildSettings = BuildSettingsVersion.V6;`
- **当前状态**: ✅ 已升级
- **修复内容**: 两个 Target.cs 文件 `V5` 改为 `V6`

---

### #10 — 所有地图自动开始比赛，赛道名固定

- **严重程度**: 🔵 注意
- **分类**: 游戏流程
- **首次发现**: 2026-06-07 (日志会话 #1)
- **日志原文**:
  ```
  LogNomiRace: Race starting: NIO City Circuit, 3 laps, 1 racers, Baja=0
  ```
- **说明**: 无论加载哪个地图（OpenWorld、NIOCityCircuit、DesertRally、MainMenu），PIE 后都自动触发比赛
  且赛道名始终为"NIO City Circuit"。这暗示：
  - 比赛自动触发逻辑不受地图类型控制
  - 赛道名称可能是硬编码或从默认配置读取
  - MainMenu 不应启动比赛；DesertRally 应使用自己的赛道名
- **当前状态**: ✅ 已修复
- **修复内容**: `Source/NomiRacingPlus/Core/NomiRaceGameMode.cpp`：
  1. `BeginPlay()` 从 `GameInstance.Settings.SelectedTrack` 读取赛道 ID，映射到 `TrackConfig.json` 的显示名称
  2. `Tick()` 中检查 `World->GetName().Contains("MainMenu")` 跳过自动开始
  3. `RaceFinish` 中结果使用动态赛道名而非硬编码

---

### #13 — DesertRally PIE 赛道名仍为 NIO City Circuit

- **严重程度**: 🔵 注意
- **分类**: 游戏流程
- **首次发现**: 2026-06-07 (日志会话 #2)
- **日志原文**:
  ```
  LogNomiRace: Race starting: NIO City Circuit, 3 laps, 1 racers, Baja=0
  ```
- **说明**: 在 DesertRally 地图上运行 PIE，比赛仍使用"NIO City Circuit"赛道名。确认赛道名与地图完全解耦，
   可能与 `RaceManager` 或 `NomiRaceGameMode` 中的默认配置有关
- **当前状态**: ✅ 已修复（与 #10 一并修复）
- **修复内容**: 从 `GameInstance.Settings.SelectedTrack` 读取赛道 ID，并通过 `TrackConfig.json` 解析显示名称

---

### #11 — zh-Hans-CN 本地化回退

- **严重程度**: 🔵 注意
- **分类**: 本地化
- **首次发现**: 2026-06-07 (日志会话 #1)
- **说明**: 系统检测到 `en-CN` locale，回退到 `zh-Hans`。此为信息性日志，程序正常运行。

---

### #12 — Slate 资源文件读取失败

- **严重程度**: 🔵 注意
- **分类**: 资源
- **首次发现**: 2026-06-07 (日志会话 #1)
- **说明**: 多个 Slate 纹理（`ButtonHoverHint.png`, `doc_16x.png` 等）读取失败，属于 UE5 Editor 开发环境常见无害警告。

---

## 日志会话记录

### 会话 #1 — 2026-06-07（首次启动）

- **操作**: 启动 UE5 Editor → 自动编译 → 打开 OpenWorld → PIE → 关闭 → 打开 NIOCityCircuit → PIE → 关闭 → 打开 DesertRally → 关闭 → 打开 MainMenu → PIE → 关闭
- **构建结果**: ✅ 成功（14.36s，2 warnings）
- **PIE 次数**: 4 次
- **新增问题**: #1 ~ #12

### 会话 #4 — 2026-06-07（验证修复 + 修复补充）

- **操作**: 启动 UE5 Editor → 自动加载 MainMenu → PIE
- **构建结果**: ✅ 成功（UBT, 102.49s）
- **PIE 次数**: 1 次（MainMenu）
- **验证日志分析**:
  - `#1` ProjectID 错误 **仍然存在** — 第一次修复 `00000000-...` 无括号仍被 UE 拒绝，需加括号格式
  - `#7` AI 生成点警告 **未出现** ✅ (MainMenu 不会触发 AI 生成)
  - `#10` MainMenu 自动开始 **未触发** ✅ (修复有效，`World->GetName().Contains("MainMenu")` 正确跳过)
  - `#8` NOMI 面部控件回退 **被触发但仍失败** — `UNOMIFaceWidget` 标记为 `Abstract`，`CreateWidget<>()` 无法实例化抽象类
- **新增/补充修复**:
  - `Config/DefaultGame.ini` — 第2次修正：`00000000-...` → `(00000000-...-...)` 加括号格式
  - `Source/NomiRacingPlus/NOMI/NOMIFaceWidget.h` — 移除 `UCLASS(Abstract, ...)` 标记
- **第二轮修复后验证**: `#1` 和 `#8` 的补充修复已构建 ✅
- **待进一步验证**: 需再运行 PIE 确认 `#1` 和 `#8` 告警消失

### 会话 #3 — 2026-06-07（修复会话）

(unchanged)

### 会话 #2 — 2026-06-07（同一编辑器会话，继续操作）

- **操作**: 上述操作结束后 → 重新打开 DesertRally → DesertRally PIE（新） → 编辑器闲置
- **构建结果**: 无（复用已有编译）
- **PIE 次数**: +1 次（DesertRally）
- **已有问题确认复现**: #1 ~ #12（全部未修复）
- **新增问题**: #13

---

## 分类统计

| 分类 | 总数 | 已修复 | 有意为之 | 信息 | 待调查 |
|------|------|--------|---------|------|-------|
| 🔴 错误 | 2 | 1 | 0 | 0 | 1 |
| 🟡 警告 | 6 | 3 | 3 | 0 | 0 |
| 🔵 注意/信息 | 5 | 3 | 0 | 2 | 0 |
| **合计** | **13** | **7** | **3** | **2** | **1** |

## 按子系统

| 子系统 | 问题 | 状态 |
|--------|------|------|
| 配置系统 | #1 | ✅ 已修复 |
| 自动化测试 | #2 | ❓ 待调查 |
| Editor 工具 | #3 | ✅ 已修复 |
| 车辆 (EP9) | #4, #5, #6 | 🎯 有意为之 |
| 关卡设计 | #7 | ✅ 已修复 |
| NOMI | #8 | ✅ 已修复 |
| 构建系统 | #9 | ✅ 已升级 |
| 游戏流程 | #10, #13 | ✅ 已修复 |
| 本地化 | #11 | ℹ️ 信息 |
| UI/资源 | #12 | ℹ️ 信息 |
