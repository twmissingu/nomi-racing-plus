# NIO Racing Plus — 开发迭代计划 v1.0

> 基于 `docs/product-critique-senior-player-vs-pm.md` (100轮资深玩家×PM深度对话分析)
> 生成日期: 2026-06-09
> 目标: 用 10~12 周时间，将游戏体验从 6.0/10 提升至 8.0+/10

---

## 目录

1. [迭代总览与依赖关系](#1-迭代总览与依赖关系)
2. [迭代 0: 物理根基 — Pacejka 独立 (Week 1-2)](#2-迭代-0-物理根基--pacejka-独立-week-1-2)
3. [迭代 1: 物理打磨 — 空气阻力 + 悬挂 + 轮胎 (Week 2-3)](#3-迭代-1-物理打磨--空气阻力--悬挂--轮胎-week-2-3)
4. [迭代 2: 游戏模式扩展 — 计时赛 + 加载屏 + HUD (Week 3-4)](#4-迭代-2-游戏模式扩展--计时赛--加载屏--hud-week-3-4)
5. [迭代 3: AI 与赛道系统完善 (Week 4-5)](#5-迭代-3-ai-与赛道系统完善-week-4-5)
6. [迭代 4: NOMI 进化 (Week 5-7)](#6-迭代-4-nomi-进化-week-5-7)
7. [迭代 5: UI/UX 全面打磨 (Week 7-8)](#7-迭代-5-uiux-全面打磨-week-7-8)
8. [迭代 6: 无障碍与游戏模式丰富 (Week 8-9)](#8-迭代-6-无障碍与游戏模式丰富-week-8-9)
9. [迭代 7: 社区基础设施 (Week 9-10)](#9-迭代-7-社区基础设施-week-9-10)
10. [迭代 8: 发布准备 — 分屏 + 锦标赛 + Steam (Week 10-12)](#10-迭代-8-发布准备--分屏--锦标赛--steam-week-10-12)
11. [验证与测试策略](#11-验证与测试策略)
12. [附录: 快速参考](#12-附录-快速参考)

---

## 1. 迭代总览与依赖关系

### 甘特图

```
Week:   1  2  3  4  5  6  7  8  9  10 11 12
       ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┐
Iter 0 │░░│░░│  │  │  │  │  │  │  │  │  │  │ Pacejka独立 + AI Waypoints采集 (并行)
  WP  │░░│░░│  │  │  │  │  │  │  │  │  │  │ ← waypoints Day 1 启动
       └──┴──┘  │  │  │  │  │  │  │  │  │  │
Iter 1    │░░░░░│  │  │  │  │  │  │  │  │  │ 空气阻力+悬挂+轮胎
          └─────┘  │  │  │  │  │  │  │  │  │
Iter 1.5     │░░░░░│  │  │  │  │  │  │  │  │ 音效修复(引擎Loop+轮胎尖叫+碰撞)
             └─────┘  │  │  │  │  │  │  │  │
Iter 2          │░░░░░│  │  │  │  │  │  │  │ 计时赛+加载屏+HUD+留存系统
                └─────┘  │  │  │  │  │  │  │
Iter 3             │░░░░░│  │  │  │  │  │  │ AI样条+人格多样性
                   └─────┘  │  │  │  │  │  │
Iter 4                  │░░░░░░░│  │  │  │  │ NOMI进化(2周)
                        └───────┘  │  │  │  │
Iter 5                       │░░░░░│  │  │  │ UI/UX打磨(含Design Token约束)
                             └─────┘  │  │  │
Iter 6                          │░░░░░│  │  │ 无障碍+模式(需Iter 3 waypoints)
                                └─────┘  │  │
Iter 7                             │░░░░░│  │ 社区基础
                                   └─────┘  │
Iter 8                                │░░░░░░░ 发布准备(3周)
                                       └───────
```

### 依赖矩阵

| 迭代 | 依赖前置 | 阻塞后续 | 说明 |
|------|----------|----------|------|
| **Iter 0** | — | Iter 1-8 | Pacejka修复是所有物理调优的前提 |
| **Iter 0.WP** | — | Iter 3, 6 | AI Waypoints 与 Iter 0 并行采集; 阻塞AI路径和Steering Assist |
| **Iter 1** | Iter 0 | Iter 2-6 | 悬挂参数影响计时赛手感、AI操控 |
| **Iter 1.5** | Iter 0 | — | 音效修复独立，不依赖其他系统 |
| **Iter 2** | Iter 0 | — | 独立交付，但最好在物理修复后做 |
| **Iter 3** | Iter 0, WP | — | AI路径依赖Waypoints采集; 参数调优建议Iter 1(悬挂+空阻)就绪后再验证 |
| **Iter 4** | Iter 1 | — | NOMI独立系统，但可读场景数据 |
| **Iter 5** | Iter 2 | — | UI开发可随时开始; 建议先产出一份Design Token约束跨迭代的UI一致性 |
| **Iter 6** | Iter 0, **Iter 3** | — | ⚠️ Steering Assist需要AI waypoints/样条数据(来自Iter 3)，非仅依赖物理 |
| **Iter 7** | — | — | 完全独立，可随时开始 |
| **Iter 8** | Iter 3, 4 | — | 分屏需要AI路径完善; HUD需先重构为Per-Player架构(见5.2) |

> **隐藏耦合警示**: Iter 3 的 AI 参数(样条 LookAhead、入弯速度)在 Iter 1(悬挂+空气阻力)未就绪时调优的结果，后续需要重调。建议 Iter 3 先完成基础设施(样条生成器+waypoint导入)，参数调优延后到 Iter 1 完成后。

#### Milestone Checkpoint

```
Week 2 End: Iter 0 出口检查
├─ ✅ 通过 → 按计划推进 Iter 1
└─ ❌ 未通过 → 自动降级: 采用摩擦系数 hack 方案(bUseInternalVehiclePhysics=false)
   保证后续迭代不受阻塞。Pacejka 架构分离留到 Iter 1 完成后追加。
```

### 并行建议 (修订版)

```
Phase 1 (Week 1-3): Iter 0 ∥ WP → Iter 1 (Pacejka + Waypoints 并行采集, 物理核心)
Phase 2 (Week 3-6): Iter 1.5 ∥ Iter 2 ∥ Iter 3 (音效 + 模式 + AI 并行)
Phase 3 (Week 6-9): Iter 4 ∥ Iter 5 ∥ Design Token ∥ Sprinkled WP补完 (最大并行)
Phase 4 (Week 9-13): Iter 6 ∥ Iter 7 ∥ Iter 8 (无障碍 + 社区 + 发布准备)
```

---

## 2. 迭代 0: 物理根基 — Pacejka 独立 (Week 1-2)

### 为什么是迭代 0

> 来自 #16 轮对话: "你们的整个物理系统建立在Pacejka模型上，但如果它和Chaos内置模型冲突，所有基于它的系统(轮胎温度、磨损、抓地力)都在'错误的假设'上运行。"

**这是所有改进的前提。不做这个，后续所有物理调优都是在沙滩上建城堡。**

### 任务分解

#### Task 0.1: 诊断 — 确认 Pacejka 与 Chaos 轮胎力叠加方式

**文件**: `NIOVehicleMovementComponent.cpp`, `TirePhysicsModel.cpp`

**当前问题**:
- `TirePhysicsModel` 实现了一套完整的Pacejka Magic Formula轮胎模型
- `NIOVehicleMovementComponent` 继承自UE5 Chaos的 `UChaosWheeledVehicleMovementComponent`
- 不确定当前 Pacejka 计算的力是*替代*还是*叠加*在 Chaos 默认轮胎模型上
- 大概率是叠加(或 Chaos 默认摩擦系数未调零)，导致操控不可预测

**实施步骤**:

1. **添加调试输出** — 在 `NIOVehicleMovementComponent::TickComponent` 或每帧物理更新中添加:
   ```
   - Chaos默认为: 纵向力 = ?, 横向力 = ?
   - Pacejka计算为: 纵向力 = ?, 横向力 = ?
   - 最终施加 = ?
   ```

2. **A/B/C 三种方案对比验证**:
   ```cpp
   // 方案A: 摩擦系数hack — SetTireFriction(0.01f)
   // 方案B: 关闭内部物理 — bUseInternalVehiclePhysics = false
   // 方案C: 两者皆做 — bUseInternalVehiclePhysics=false + friction≈0
   ```
   在诊断阶段跑完所有三种方案，记录各自的力输出和车辆行为，选择最优路径进入 Task 0.2。

3. **探索 `bUseInternalVehiclePhysics` 路径**:
   - 在 `NIOVehicleMovementComponent::BeginPlay()` 中测试该标志位
   - 确认关闭后 Chaos 内部轮胎求解器被跳过
   - 对比方案 A/B 的 CPU 开销差异 (Unreal Insights)

4. **捕获关键截面** — 在直道加速、中等速度弯道、低速发夹弯三个场景记录力数据

**验收标准**:
- [ ] 明确知道当前Pacejka和Chaos力的关系(替代/叠加)
- [ ] A/B/C 三种方案的力输出可对比
- [ ] 确认最优方案(力最干净+CPU开销最低)

---

#### Task 0.2: 实现 Pacejka 独立控制 (基于 Task 0.1 选出的最优方案)

**文件**: `NIOVehicleMovementComponent.cpp`, `NIOVehicleMovementComponent.h`, `TirePhysicsModel.cpp`

**实施 (以方案C为例; 如Task 0.1选择其他方案则调整)**:

1. **在 `NIOVehicleMovementComponent.h` 添加配置**:
   ```cpp
   UPROPERTY(Config)
   bool bOverrideChaosTireWithPacejka = true;
   
   UPROPERTY(Config)
   bool bUseInternalVehiclePhysics = false;  // 关闭内部求解器
   
   UPROPERTY(Config)  
   float ChaosTireFrictionOverride = 0.01f; // 后备: 将Chaos摩擦降到近乎0
   ```

2. **在 `NIOVehicleMovementComponent::BeginPlay` 和 `TickComponent` 中实现**:
   ```cpp
   void NIOVehicleMovementComponent::BeginPlay() {
       Super::BeginPlay();
       SetUseInternalVehiclePhysics(bUseInternalVehiclePhysics);
   }
   
   void NIOVehicleMovementComponent::TickComponent(...) {
       Super::TickComponent(...);
       
       if (bOverrideChaosTireWithPacejka) {
           // 1. 限制Chaos默认轮胎力的贡献
           SetTireFriction(ChaosTireFrictionOverride);
           
           // 2. 确保Pacejka力被正确应用
           //    (确认ApplyTireForces中的调用链)
       }
   }
   ```

3. **添加可视化调试模式** (可选但推荐):
   ```cpp
   // 在屏幕上绘制轮胎力向量
   #if ENABLE_DRAW_DEBUG
       DrawDebugForceVectors(GetWorld(), TireForces, TireLocations);
   #endif
   ```

**验收标准**:
- [ ] `bUseInternalVehiclePhysics` 标志位生效(方案B/C)
- [ ] 开启时，Chaos默认轮胎力贡献 < 5%
- [ ] Pacejka 力被正确施加到刚体
- [ ] 车辆在开启模式下可驾驶完成一圈
- [ ] 相比方案A有可测量的CPU开销降低(Unreal Insights)

---

#### Task 0.3: 基本参数调优

**文件**: `TirePhysicsModel.cpp`, `VehicleConfig.json`

**对 5 条赛道的调优**:
- 城市赛道 (NIOCityCircuit): 高抓地力，频繁转向
- 山路: 中低抓地力，速度变化大
- 赛道/专业赛道: 中等抓地力，高速弯
- 测试跑道: 各种工况

**参数范围参考 (Pacejka Magic Formula)**:
- `B` (刚度因子): 8~12 (越高过弯响应越快)
- `C` (形状因子): 1.3~1.6 (控制峰值形状)
- `D` (峰值因子): 0.8~1.2 (峰值摩擦系数)
- `E` (曲率因子): -0.5~0.5 (控制大滑移区域特征)

**步骤**:
1. 选取一条赛道(建议城市赛道)和一辆车(EP9)
2. 设置 `B=10, C=1.4, D=1.0, E=-0.2` 作为基线
3. 驾驶并记录主观感受(转向不足/转向过度程度、抓地力极限)
4. **客观指标校准** — 对比真实车辆数据或合理游戏范围:
   ```
   - 0-100km/h: < 2.8s (EP9实际2.7s)
   - 100-0刹车距离: 35-40m
   - S弯80km/h入弯最大侧倾角: 3-5°
   - 固定转向输入下横摆角速度曲线无异常振荡
   ```
5. 迭代调整直到指标达标且手感合理
6. 在其他赛道上验证通用性

**验收标准**:
- [ ] EP9在NIOCityCircuit完成一圈无异常行为
- [ ] 车辆在弯道中有可预测的转向特性(侧倾角在3-5°范围)
- [ ] 急速转向时车辆响应不延迟/不突兀
- [ ] 不同赛道上的行为一致(同一辆车)
- [ ] 客观指标在目标范围内

---

#### Task 0.4: 更新单元测试

**文件**: `Tests/` 目录, 确认测试文件位置

**确认现有测试覆盖**:
- 检查 `TirePhysicsModelTest.cpp` 或类似测试是否存在
- 如果有: 更新测试用例以适应新的力计算模式
- 如果没有: 添加 GoogleTest 基本测试

**需要添加的测试**:
```cpp
TEST(PacejkaModel, ForceDirection_MatchesSlipDirection) {
    // 验证: 纵向力方向与纵向滑移方向一致
}
TEST(PacejkaModel, PeakForce_ReasonableRange) {
    // 验证: 峰值摩擦系数在 0.8~1.2 范围内
}
TEST(PacejkaModel, CombinedMode_PrecedesChaosTest) {
    // 验证: 开启bPacejkaOnly后Chaos贡献可忽略
}
```

**验收标准**:
- [ ] 所有现有测试通过(或更新后通过)
- [ ] 新增测试验证 Pacejka 独立行为
- [ ] A/B 对比有可复现的测试用例

---

### Iteration 0 出口标准

- [ ] Pacejka 完全掌控轮胎力且没有来自 Chaos 默认轮胎的干扰
- [ ] 基本调优完成，车辆可正常驾驶
- [ ] 调试可视化工具可用
- [ ] 现有测试通过，新增测试覆盖
- [ ] 完成迭代回顾记录

**预计工时**: 25~35 小时

---

## 3. 迭代 1: 物理打磨 — 空气阻力 + 悬挂 + 轮胎 (Week 2-3)

### 依赖
- **必须**: Iteration 0 完成 (Pacejka独立)
- **可选**: 可与 Iter 2 并行(如果不涉及同一组件)

### 3.1 空气阻力实现

**文件**: `NIOVehicleMovementComponent.cpp`, `VehicleConfig.json`

**当前问题**: `VehicleConfig.json` 中已为每辆车配置了 `DragCoefficient`，但 `NIOVehicleMovementComponent` 中未读取并使用该值。

**实施**:

1. **验证当前空气阻力状态**:
   ```cpp
   // 在 NIOVehicleMovementComponent 中搜索 DragCoefficient 或空气阻力相关代码
   // 确认是否已有任何空气阻力实现或依赖Chaos默认方案
   ```

2. **实现空气阻力力计算**:
   ```cpp
   // 在物理更新中加入:
   float AirDensity = 1.225f; // kg/m^3，标准大气
   float DragCoeff = /* 来自 VehicleConfig */;
   float FrontalArea = /* 来自车辆配置 */;
   float SpeedSq = GetVelocity().SizeSquared();
   
   FVector DragForce = -0.5f * AirDensity * DragCoeff * FrontalArea * SpeedSq * GetVelocity().GetSafeNormal();
   AddForce(DragForce, NAME_None);
   ```

3. **从 VehicleConfig 读取**:
   ```cpp
   void NIOVehicleMovementComponent::InitializeDragCoefficient() {
       // 从 VehicleConfig.json 的当前车辆条目读取
       // 使用 VehicleStateManager 或者直接读配置文件
       if (const FVehicleConfig* Config = /* 获取当前车辆配置 */) {
           DragCoefficient = Config->DragCoefficient;
           FrontalArea = Config->FrontalArea;
       }
   }
   ```

4. **调试验证**: 在直道测试，对比 Speed-Time 曲线：
   - EP9 (低风阻，~0.25 Cd): 极速应接近~300km/h
   - SU7 Ultra (较高风阻，~0.3 Cd): 极速略低于EP9

**验收标准**:
- [ ] `DragCoefficient` 从 VehicleConfig 正确读取
- [ ] 不同车辆的极速差异符合其风阻系数
- [ ] 松开油门后滑行减速率合理(不突兀不过慢)
- [ ] 空气阻力随速度平方增长

**预计工时**: 3~5 小时

---

### 3.2 悬挂改进 — 防倾杆效果 + 行程限制

**文件**: `NIOVehicleMovementComponent.cpp`, `NIOVehicleMovementComponent.h`, `NIOVehicleBase.cpp`

**当前问题**:
- 没有主动防倾杆逻辑
- 悬挂行程未做软/硬限制
- 侧倾率可能不真实(缺少侧倾刚度计算)

**实施**:

1. **防倾杆效果** — 计算左右轮之间的力传递:
   ```cpp
   // 在物理更新中加入防倾杆扭矩:
   float AntiRollForce = AntiRollStiffness * (LeftSuspensionCompression - RightSuspensionCompression);
   ApplyAntiRollForce(AntiRollForce, LeftWheelIndex, RightWheelIndex);
   ```
   - `AntiRollStiffness` 从 VehicleConfig 读取
   - 在高重心车辆(如ES7)上设置更高值减少侧倾
   - 在低重心车辆(EP9)上设低值允许更多侧倾

2. **悬挂行程限制**:
   ```cpp
   // 在悬挂压缩计算后添加:
   float ClampedCompression = FMath::Clamp(SuspensionCompression, MinTravel, MaxTravel);
   if (!FMath::IsNearlyEqual(ClampedCompression, SuspensionCompression)) {
       // 到达行程限制，添加硬停冲击力
       AddBumpStopForce(/*...*/);
   }
   ```

3. **悬挂参数从 VehicleConfig 扩展**:
   ```json
   {
     "Suspension": {
       "AntiRollStiffness": 5000.0,
       "SpringRate": 35000.0,
       "DampingRate": 3000.0,
       "MinTravel": -0.05,
       "MaxTravel": 0.12
     }
   }
   ```

**验收标准**:
- [ ] 高速过弯时车辆有明显的侧倾(但受控)
- [ ] 防倾杆参数调高后侧倾明显减少
- [ ] 悬挂压缩到达极限时不会穿透(无抖动/弹跳)
- [ ] 不同车型(EP9低悬挂 vs ES7高悬挂)手感差异明显

**预计工时**: 8~12 小时

---

### 3.3 轮胎温度模型调优 — HeatFromSlip 参数校准

**文件**: `TirePhysicsModel.cpp`, `NIOTirePresets.h`, `VehicleConfig.json`

**当前问题**: 轮胎温度模型存在(`FTireThermalState`)，但 `HeatFromSlip` 参数需要校准以产生合理的温度变化。

**实施**:

1. **检查当前传热逻辑**:
   ```cpp
   // 在 TirePhysicsModel 中搜索 heat, thermal, temperature
   // 确认 HeatFromSlip 的当前算法和参数
   ```

2. **参数校准 — 在直道和弯道测量温度变化**:
   - 直道巡航: 轮胎温度应缓慢上升(摩擦低)
   - 剧烈漂移/急刹: 温度应快速上升
   - 各轮胎独立: 左前轮胎在连续右弯中应该明显更热

3. **温度影响**:
   - 低温: 抓地力降低（轮胎未达到工作温度）
   - 工作温度: 峰值抓地力
   - 过热: 抓地力下降(过热衰退)
   ```cpp
   float GripMultiplier = GetTemperatureGripFactor(TireTemp, OptimalTemp, OverheatTemp);
   // 在 Pacejka 力计算中乘以 GripMultiplier
   ```

4. **地面传导热交换 (新增)**:
   ```cpp
   // 轮胎温度冷却增加地面热传导项
   // 原: CoolingRate = f(空气对流)
   // 新: CoolingRate = f(空气对流) + f(地面温度 - 轮胎温度) * 热传导系数
   
   float GroundTemp = GetSurfaceTemperature();  // 从赛道配置读取
   float ConductionCoeff = 0.001f;             // 热传导系数
   float GroundHeatTransfer = ConductionCoeff * (GroundTemp - TireTemp);
   TireTemp += GroundHeatTransfer * DeltaTime;
   
   // 效果: 停在原地轮胎温度逐渐降至地面温度
   // 暖胎圈: 轮胎摩擦生热超过地面冷却 → 温度上升
   // 寒冷赛道(山路): 轮胎更难达到工作温度
   ```

5. **添加温度可视化**:
   - 在 `TireTempWidget` 中显示当前各轮胎温度
   - 颜色编码: 蓝色(冷)→绿色(工作)→红色(过热)

**验收标准**:
- [ ] 漂移/急刹后轮胎温度明显上升
- [ ] 冷胎起步 → 工作温度 → 过热有明确的温度曲线
- [ ] 轮胎温度影响抓地力的幅度合理(±10~15%)
- [ ] 停在原地轮胎温度逐渐降到环境/地面温度
- [ ] 不同赛道温度(山路冷/城市热)影响轮胎升温速度
- [ ] TireTempWidget 正确显示温度状态
- [ ] 不同地面摩擦和驾驶风格产生不同的温度分布

**预计工时**: 6~10 小时

---

### Iteration 1 出口标准

- [ ] 空气阻力正确实现，极速差异真实
- [ ] 悬挂有防倾杆逻辑，不同车型侧倾区别明显
- [ ] 轮胎温度变化合理，影响抓地力
- [ ] 所有更改在 5 条赛道和 6 辆车上通过基本测试
- [ ] 回归测试通过

**总预计工时**: 17~27 小时

---

## 4. 迭代 1.5: 基础音效修复 (Week 3-4)

> 来自第7轮批评: "你们的音效系统MetaSound资产全是STUB状态...比没有更糟糕"

**为什么在物理之后**: 音效不依赖物理结果，但物理调优需要"听"引擎声和轮胎声来感知速度。先修物理再修音效，因为物理决定了"应该发出什么声音"。

### 4.1 引擎声循环修复

**文件**: `Core/AudioManager.cpp`, Content 音频资产

**当前问题**: 电机加速声不对/缺失，使用 MetaSound 但资产 stub。

**实施**:

1. **从 freesound.org 搜索 CC0/CC-BY 电动引擎声**:
   - 搜索词: `electric motor acceleration`, `EV engine loop`, `tesla motor whine`
   - 下载 3-5 个不同转速段的素材
   - 用 Audacity 剪辑为循环(Loop)文件

2. **实现速度→音调映射**:
   ```cpp
   // AudioManager.cpp
   void UAudioManager::UpdateEngineSound(float Speed, float RPM) {
       if (EngineLoopComponent && EngineLoopComponent->IsPlaying()) {
           // 速度比例 → 音调(Pitch)映射
           float SpeedRatio = FMath::GetMappedRangeValueClamped(
               {0.0f, MaxSpeed}, {0.5f, 2.0f}, Speed);
           EngineLoopComponent->SetPitchMultiplier(SpeedRatio);
           
           // 速度比例 → 音量(随速增大)
           float VolumeRatio = FMath::GetMappedRangeValueClamped(
               {0.0f, MaxSpeed}, {0.3f, 1.0f}, Speed);
           EngineLoopComponent->SetVolumeMultiplier(VolumeRatio);
       }
   }
   ```

3. **验证不同车辆的音调差异**:
   - EP9: 高频啸叫(高速电机)
   - ES7/ET7: 较柔和的中频
   - SU7 Ultra: 低沉的强力电机

### 4.2 轮胎尖叫分层

1. **采集 3 种轮胎素材**:
   - 低速尖叫(停车场/发夹弯): 高频尖锐
   - 高速啸叫(高速弯): 中等频率
   - 漂移音(持续侧滑): 粗糙摩擦声

2. **映射到 TirePhysicsModel 输出**:
   ```cpp
   float SlipAngle = /* 当前侧滑角 */;
   float SlipRatio = /* 当前滑移率 */;
   float TireLoad  = /* 当前轮胎负载 */;
   
   // 使用滑移数据驱动音效混合
   float ScreamIntensity = FMath::Clamp(SlipAngle * TireLoad * 0.001f, 0.0f, 1.0f);
   TireScreamComponent->SetVolumeMultiplier(ScreamIntensity);
   TireScreamComponent->SetPitchMultiplier(1.0f + SlipRatio * 0.5f);
   ```

### 4.3 碰撞+路肩音效

1. **采集碰撞音效**: 金属碰撞声、刮擦声、路肩震动声
2. **映射到碰撞事件**: `OnVehicleCollision` → 根据相对速度/碰撞角度播放对应音效
3. **路肩检测**: 车轮压上路肩时播放低频震动声

**验收标准**:
- [ ] 引擎声随速度平滑变化(从慢速低沉到高速啸叫)
- [ ] 不同车辆引擎声有所区别
- [ ] 轮胎尖叫在漂移/急转时触发
- [ ] 碰撞时有对应音效
- [ ] 音效不影响游戏性能(FPS 下降 < 3%)

**预计工时**: 15~20 小时

---

## 5. 迭代 2: 游戏模式扩展 — 计时赛 + 加载屏 + HUD + 留存系统 (Week 4-5)

> 迭代 2 与迭代 1.5 并行执行(互不依赖)
>
> **UI Design Token 约束**: 在 Iter 2 启动前, 先产出一份 UI Design Token 文档来约束跨迭代的 UI 一致性。
> Token 文档包括: 间距基准(4px网格)、圆角半径(4/8/12px层级)、动画曲线(EaseInOut Cubic)、NIO 品牌色(蓝/绿/金)使用规则、字体层级(H1/H2/Body/Caption 字号行高)、阴影层级(elevation 1/2/3)。
> 所有 UI 迭代(Iter 2/5/6/8)必须遵循此 Token, 确保视觉一致性不碎片化。

### 4.1 本地排行榜计时赛模式

**文件**: `RaceManager.cpp`, `RaceManager.h`, `RaceProgression.cpp`, `RaceProgression.h`, `RaceProgressionTypes.h`, `ProgressionSerializer.cpp`

**当前优势**: 
- `RaceManager` 已有完整的比赛状态机(空闲→准备→倒计时→比赛→结束→结算)
- `RaceProgression` 有 `FSessionHistory` 结构，可复用到计时赛记录
- `ProgressionSerializer` 已有 JSON 持久化能力

**实施**:

1. **添加计时赛模式枚举**:
   ```cpp
   // RaceProgressionTypes.h
   UENUM()
   enum class ERaceMode : uint8 {
       Standard,
       Championship,
       TimeTrial,    // 新增
   };
   ```

2. **计时赛状态机简化** — 从 RaceManager 继承:
   ```cpp
   // TimeTrialManager.h (新增)
   // 不需要AI车辆、不需要位置判定、不需要计分
   // 只需要: 计时器 + 圈数计数 + 终点判定
   ```

3. **排行榜数据模型**:
   ```cpp
   // RaceProgressionTypes.h
   USTRUCT()
   struct FTimeTrialRecord {
       FString TrackName;
       FString VehicleName;
       float BestLapTime;
       float BestSector1Time;
       float BestSector2Time;
       float BestSector3Time;
       FDateTime RecordDate;
   };
   ```

4. **本地排行榜存储**:
   ```cpp
   // ProgressionSerializer 中添加:
   static bool SaveTimeTrialRecords(const TArray<FTimeTrialRecord>& Records);
   static TArray<FTimeTrialRecord> LoadTimeTrialRecords();
   ```

5. **计时赛 UI**:
   - 赛道选择 → 车辆选择 → "GO!"
   - HUD 上显示分段计时(左1段、中2段、右3段+总时间)
   - 当前圈 vs 最佳圈对比(Gold/Silver/ Purple 分段)
   - 过终点后的排行榜展示

6. **复用现有组件**:
   - `CheckpointSystem` → 用于分段计时
   - `RaceHUD` → 可扩展显示计时数据
   - `ResultsWidget` → 可复用记录展示逻辑

**验收标准**:
- [ ] 计时赛模式可选并正常开始
- [ ] 有分段计时(至少3段)
- [ ] 单圈计时精度到毫秒
- [ ] 最佳圈对比(Gold/Purple)
- [ ] 排行榜保存在本地磁盘
- [ ] 最好成绩按赛道+车辆分别存储
- [ ] 重启游戏后排行榜不丢失

**预计工时**: 12~16 小时

---

### 4.2 加载屏幕改进

**文件**: `LoadingScreenWidget.cpp`, `LoadingScreenWidget.h`

**当前问题**: 加载屏幕为空/只有"加载中"文字。

**实施**:

1. **添加进度指示器** (区分精确进度和粗粒度进度):
   ```cpp
   // LoadingScreenWidget.cpp
   // 如果使用 FStreamableManager 的 AsyncLoad → 精确进度
   // 否则使用基于预估总大小的粗粒度进度
   
   void ULoadingScreenWidget::OnLoadProgress(float Progress) {
       ProgressBar->SetPercent(Progress);
       ProgressText->SetText(FText::AsPercent(Progress));
   }
   ```

2. **加载信息展示**:
   - 当前加载的赛道名称和预览图
   - 选用的车辆缩略图
   - 随机的游戏小贴士(从 `TutorialManager` 或独立 `Tips.json` 取):
     - "EP9的四电机扭矩控制让出弯更快"
     - "利用NOMI的赛道提示提升过弯技巧"
     - "保持轮胎温度在工作范围以获得最佳抓地力"

3. **异常状态处理** (三条失败分支):
   - **超时**(15秒无进度) → 显示"加载超时"按钮: [重试] / [返回菜单]
   - **资源缺失**(加载失败) → 显示具体缺失项 + 建议重新安装
   - **崩溃**(上次异常退出) → 显示"上次游戏异常退出"提示 + 安全模式建议

4. **视觉设计**:
   - 背景: 当前赛道的远景或概念图
   - 风格: NIO 极简设计语言(深色背景 + 绿/金点缀)
   - 动画: 渐变进度条 + 平滑过渡

**验收标准**:
- [ ] 加载时显示百分比进度(精确或粗粒度)
- [ ] 加载时显示随机小贴士
- [ ] 加载完成自动过渡到游戏场景
- [ ] 加载超时/资源缺失/崩溃恢复三条异常路径有 UI
- [ ] 无显著性能开销

**预计工时**: 5~8 小时 (含异常状态处理)

---

### 4.3 分层 HUD 刷新率优化

**文件**: `RaceHUD.cpp`, `RaceHUD.h`

**当前问题**: 所有 HUD 数据以相同频率更新，导致不必要的 CPU 开销。

**实施**:

1. **定义分层策略**:
   ```cpp
   enum class EHUDUpdateTier {
       High    = 0,  // 每帧更新: 车速、转速、挡位
       Medium  = 1,  // 每3帧: 位置、圈数、排名变化
       Low     = 2,  // 每10帧: 轮胎温度、油耗、续航
       Static  = 3,  // 事件触发: 分段时间、圈记录、通知
   };
   ```

2. **实现更新管理器**:
   ```cpp
   class FRaceHUDUpdateManager {
       int32 FrameCounter = 0;
       
       bool ShouldUpdate(EHUDUpdateTier Tier) {
           switch (Tier) {
               case High:   return true;
               case Medium: return (FrameCounter % 3 == 0);
               case Low:    return (FrameCounter % 10 == 0);
               case Static: return false; // 由事件触发
           }
       }
   };
   ```

3. **扩展 TireTempWidget 验证更新层级**:
   - 轮胎温度 → Low (每10帧)
   - 显示效果无感知差异

**验收标准**:
- [ ] High 层数据每帧更新
- [ ] Medium 层数据每3帧更新
- [ ] Low 层数据每10帧更新
- [ ] 玩家无感知差异
- [ ] HUD 更新导致的主线程开销降低(通过Unreal Insights可测量)

**预计工时**: 3~5 小时

---

### 4.4 UI 动画基础框架

**文件**: `MenuManager.cpp`, `MainMenuWidget.cpp`, `RaceHUD.cpp` (各UI组件)

**当前问题**: UI 缺少动画反馈，按钮切换生硬。

**实施**:

1. **按钮悬浮/按下反馈**:
   ```cpp
   // 在按钮的 Hovered/Unhovered/Clicked 事件中添加
   // - 缩放动画: 1.0 → 1.05 (悬浮), 1.0 → 0.98 (按下)
   // - 颜色过渡: 默认 → 高亮
   ```

2. **页面切换过渡**:
   ```cpp
   // MenuManager::SwitchToWidget
   // - 旧页面: 透明度 1.0 → 0.0 (200ms)
   // - 新页面: 透明度 0.0 → 1.0 (200ms)
   ```

3. **HUD 数字滚动效果**:
   - 分段计时成绩切换时数字滚动动画
   - 速度/转速数字平滑变化

**验收标准**:
- [ ] 所有按钮有悬浮和按下反馈(缩放+颜色)
- [ ] 页面切换有淡入淡出效果
- [ ] 核心数据显示数值变化时有平滑过渡
- [ ] 动画不阻碍操作

**预计工时**: 5~8 小时

---

### 4.5 基础留存系统 — 成就墙 + 统计追踪

**文件**: `Race/RaceProgression.cpp`, `RaceProgression.h`, UI Widgets

**当前问题**: "这个游戏没有让我'再来一局'的钩子"(第14轮) — 代码中存在 RaceProgression 但 UI 上缺少成就展示。

**实施**:

1. **成就数据结构** (复用 RaceProgression 已有的 session history):
   ```cpp
   USTRUCT()
   struct FPlayerAchievement {
       FString AchievementId;
       FString Title;
       FString Description;
       bool bUnlocked;
       FDateTime UnlockTime;
       float Progress;      // 0.0~1.0 进度
       float Target;        // 目标值
   };
   
   // 初始成就列表 (纯本地, 10个)
   // - "初出茅庐": 完成第一场比赛
   // - "十圈老手": 累计完成10圈
   // - "全能车手": 所有6辆车都开过
   // - "赛道征服者": 所有5条赛道都跑过
   // - "金牌得主": 计时赛获得任意赛道金牌
   // - "永不停歇": 累计游戏时间超过2小时
   // - "完美起步": 获得一次完美起步
   // - "超车达人": 累计超越20辆AI
   // - "漂移之王": 累计漂移距离超过1000米
   // - "冠军🏆": 完成一个锦标赛
   ```

2. **成就 UI**:
   - 主菜单新增"成就"入口按钮
   - 成就墙: 网格展示所有成就(已解锁/锁定)
   - 进度条显示未解锁成就的完成进度

3. **每周计时赛挑战** (纯本地):
   - 每周一重置: 指定赛道+车辆组合
   - 显示玩家该周的当前最佳圈速
   - 无需奖励系统，纯"和自己赛跑"

**验收标准**:
- [ ] 成就墙在 UI 中可访问
- [ ] 比赛完成后自动检测并弹出解锁通知
- [ ] 成就解锁持久化保存
- [ ] 每周挑战在对应日期重置

**预计工时**: 8~12 小时

---

### Iteration 2 出口标准 (修订版)

- [ ] 计时赛模式完整可用(含本地排行榜)
- [ ] 加载屏幕显示进度+赛道信息+小贴士
- [ ] HUD 分层刷新率机制实现
- [ ] UI 动画基础框架部署到主要页面
- [ ] 成就墙+10个基础成就+每周挑战
- [ ] 所有更改回归测试通过

**总预计工时**: 31~46 小时

---

## 6. 迭代 3: AI 与赛道系统完善 (Week 5-6)

### 5.1 AI 路径样条化 — Waypoint → Catmull-Rom 样条

**文件**: `AI/AICarController.cpp`, `AI/AICarController.h`, `AI/AIBehaviorTree.cpp`

**当前问题**: AI 使用离散 Waypoint 导航，导致转弯时出现明显的"指向-追逐"式生硬转向。

**实施**:

1. **添加 Catmull-Rom 样条生成器**:
   ```cpp
   // AICarController.h
   class FCatmullRomSpline {
   public:
       void BuildFromWaypoints(const TArray<FVector>& Waypoints);
       FVector EvaluatePosition(float t) const;    // t: 0~1
       FVector EvaluateTangent(float t) const;
   private:
       TArray<FVector> ControlPoints;
       float TotalLength;
   };
   ```

2. **AI 跟随从 Waypoint → 样条切换**:
   ```cpp
   // AICarController.cpp
   // 原来: ChaseWaypoint(CurrentWaypointIndex);
   // 现在: 
   float SplineT = /* 当前在样条上的位置 */;
   FVector TargetPos = Spline.EvaluatePosition(SplineT + LookAheadDistance);
   FVector TargetDir = Spline.EvaluateTangent(SplineT);
   SetSteeringTarget(TargetPos, TargetDir);
   ```

3. **Look-Ahead 自适应**:
   - 直线: LookAheadDistance = 大(30m)
   - 急弯: LookAheadDistance = 小(10m)
   - 根据路径曲率自动调整

**验收标准**:
- [ ] AI 在弯道中走线平滑，无明显"折线"
- [ ] AI 在直线上充分利用赛道宽度
- [ ] Look-Ahead 自适应在不同曲率弯道中工作
- [ ] 与手动驾驶的赛车在同一条线上无明显差距

**预计工时**: 8~12 小时

---

### 5.2 AI 人格多样性 — 进攻型 / 防守型 / 平衡型

**文件**: `AI/AICarController.cpp`, `AI/AICarController.h`, `AI/AIBehaviorTree.cpp`, `AI/AIProfiles.json`

**当前问题**: 所有 AI 对手使用相同的驾驶策略。

**实施**:

1. **定义人格枚举**:
   ```cpp
   UENUM()
   enum class EAIPersonality : uint8 {
       Aggressive,   // 激进: 更晚刹车、更早开油、冒险超车
       Defensive,    // 防守: 保持线路、不易被超、保守刹车
       Balanced,     // 平衡: 介于两者之间
       Rookie,       // 新手: 频繁失误、路线不优、慢
   };
   ```

2. **人格参数化** (从 `AIProfiles.json` 加载):
   ```json
   {
     "personalities": {
       "aggressive": {
         "brakeBias": 0.55,
         "cornerEntrySpeed": 1.15,
         "overtakeAggression": 0.9,
         "mistakeChance": 0.05,
         "defensiveLineChance": 0.1
       },
       "defensive": {
         "brakeBias": 0.45,
         "cornerEntrySpeed": 0.9,
         "overtakeAggression": 0.3,
         "mistakeChance": 0.02,
         "defensiveLineChance": 0.8
       },
       "rookie": {
         "brakeBias": 0.4,
         "cornerEntrySpeed": 0.7,
         "overtakeAggression": 0.1,
         "mistakeChance": 0.25,
         "defensiveLineChance": 0.3
       }
     }
   }
   ```

3. **AI 分配策略**:
   - 锦标赛: 每个对手随机分配一个独特人格
   - 快速比赛: 可根据难度级别选择人格池

4. **行为差异化**:
   - 激进 AI: 弯中更接近前车、尝试内线/外线超车
   - 防守 AI: 入弯前占据内线、阻挡超车路线
   - 新手 AI: 偶尔错过刹车点、出弯给油太晚

**验收标准**:
- [ ] 不同人格的 AI 驾驶风格可感知区别
- [ ] 激进 AI 的超车尝试明显更多
- [ ] 防守 AI 更难被超越
- [ ] 新手 AI 有明显失误

**预计工时**: 8~12 小时

---

### 5.3 赛道 AI Waypoints 补全 (采集工作 Day 1 与 Iter 0 并行启动)

**文件**: `Content/Maps/TrackConfig.json`, 赛道配置

**当前问题**: 5 条赛道中只有 NIOCityCircuit 有完整 AI waypoints，其他赛道(山路、赛道、专业赛道、测试跑道)的 AI 无法正常运行。

**并行策略**: 
- ⏰ **Day 1 启动**: Waypoints 采集不依赖任何物理修复，可与 Iter 0 并行开始
- 🎯 **范围控制**: 在资源约束下优先完成 3 条赛道(城市+山路+赛道)×Professional 难度
- 🌱 **社区贡献**: 其余赛道+难度级别标记"community needed"，在 Discord 发布采集指南

**实施**:

1. **确认缺失清单**:
   ```
   ✅ NIOCityCircuit — 完整 waypoints
   ❌ 山路 (MountainRoad) — 缺失 (优先级: 高)
   ❌ 赛道 (RacingTrack) — 缺失 (优先级: 高)
   ❌ 专业赛道 (ProCircuit) — 缺失 (优先级: 中)
   ❌ 测试跑道 (TestTrack) — 缺失 (优先级: 低)
   ```

2. **Waypoint 采集方法**:
   - 选项 A: 手动驾驶最佳路线，用脚本记录位置(精度高、耗时长)
   - 选项 B: 在 UE5 编辑器中沿赛道中心线放置 spline actors(中等精度)
   - 选项 C: 从赛道 spline 数据自动采样(如果 spline 存在)

3. **AI 难度分层**:
   - 每条赛道至少 1 个 waypoint 集: Professional (最佳路线)
   - Normal/Easy 可通过算法从 Professional 缩放: 入弯速度 -10%/+10%

4. **每个赛道配置结构**:
   ```json
   {
     "TrackName": "MountainRoad",
     "AIWaypoints": {
       "Professional": [...],
       "Normal": [...],
       "Easy": [...]
     },
     "AISplinePoints": [...],  // 可选, 如果使用样条
     "PitEntry": { "x": ..., "y": ..., "z": ... },
     "PitExit": { "x": ..., "y": ..., "z": ... }
   }
   ```

**验收标准**:
- [ ] 3 条赛道(城市+山路+赛道)有 Professional waypoints
- [ ] AI 在每条已完成的赛道上正常跑完 3 圈
- [ ] AI 不会驶出赛道边界(除非失误)
- [ ] Normal/Easy 通过缩放算法可用(即使不是最优)

**预计工时**: 28~35 小时 (Day 1 即启动, 与 Iter 0 并行)

---

### 5.4 赛道变体: 逆向/昼夜/湿滑模式

**文件**: `TrackConfig.json`, `RaceManager.cpp`

**实施**:

1. **赛道变体配置**:
   ```json
   {
     "TrackName": "NIOCityCircuit",
     "Variants": {
       "Reverse": {
         "AIWaypoints": [...],  // 逆向 waypoints
         "StartGrid": [...],    // 逆向发车位置
         "Direction": -1
       },
       "Night": {
         "PostProcessSettings": "NightTime_Profile",
         "AIWaypoints": "SameAsNormal"  // 复用正向 waypoints
       },
       "Wet": {
         "FrictionMultiplier": 0.7,
         "AIWetMode": true,
         "WeatherEffect": "Rain_ParticleSystem"
       }
     }
   }
   ```

2. **天气/时间选择 UI**:
   - 赛道选择界面添加: 天气/时间下拉选择
   - 选项: 白天、夜晚、雨中

3. **黑夜设置** (最低成本):
   - 全局光照调暗
   - 车灯自动开启
   - NOMI: "天黑路滑，小心驾驶"

4. **湿地设置** (摩擦系数修改):
   - `TirePhysicsModel` 读取天气配置中的 `FrictionMultiplier`
   - 轮胎抓地力降低 ~30%
   - AI 在湿地模式下走线更保守

**验收标准**:
- [ ] 逆向赛道正常工作
- [ ] 夜晚赛道视觉可感知
- [ ] 湿地赛道摩擦系数降低
- [ ] NOMI 对天气变体有评论

**预计工时**: 6~10 小时

---

### Iteration 3 出口标准

- [ ] AI 路径样条化实现且走线平滑
- [ ] AI 人格多样性可感知(至少3种人格)
- [ ] 所有 5 条赛道 AI waypoints 完整
- [ ] 至少 2 种赛道变体工作(逆向 + 一个环境变体)
- [ ] 回归测试通过

**总预计工时**: 37~54 小时

---

## 7. 迭代 4: NOMI 进化 (Week 6-8)

### 6.1 NOMI TTS 语音集成

**文件**: `NOMI/ReplayVoiceManager.cpp`, `NOMI/NOMIController.cpp`, `NOMI/CommentaryEngine.cpp`

**当前问题**: `ReplayVoiceManager` 已有语音管理框架，但无实际 TTS 后端连接。NOMI 默认是"哑巴"。

**实施**:

1. **TTS 抽象层 (事件驱动 + 超时保护)**:
   ```cpp
   // NOMI/TTSBackend.h (新增)
   class ITTSBackend {
   public:
       DECLARE_MULTICAST_DELEGATE(FOnTTSComplete);
       
       virtual ~ITTSBackend() = default;
       virtual bool Speak(const FString& Text, float Volume, float Rate) = 0;
       virtual void Stop() = 0;
       virtual bool IsSpeaking() const = 0;
       virtual void SetVoice(const FString& VoiceName) = 0;
       virtual void SetMaxSpeakDuration(float Seconds) = 0;  // 硬超时保护
       
       FOnTTSComplete OnTTSComplete;  // 事件驱动, 非轮询
   };
   ```

2. **平台 TTS 实现**:
   ```cpp
   // Windows: NOMI/TTS_Windows.cpp (需 COM + 游戏线程管理)
   // macOS: NOMI/TTS_Mac.mm (需 Objective-C++ 后缀)
   
   #if PLATFORM_WINDOWS
   class FWINDOWSTTSBackend : public ITTSBackend {
       ISpVoice* Voice;
       bool Speak(...) override {
           // CoInitializeEx(COINIT_MULTITHREADED) ← UE5 多线程环境
           // 使用 AsyncTask 在工作线程完成 COM 操作
           // SpCreateVoice + Speak(SPF_ASYNC)
           // 设置超时定时器: FTimerHandle
       }
   };
   #elif PLATFORM_MAC
   class FMACTTSBackend : public ITTSBackend {
       // NSSpeechSynthesizer delegate 回调
       // 注意: .mm 文件后缀, macOS 14+ 权限检查
   };
   #else
   // 回退: 预录制 50 条核心评论的 WAV 文件
   #endif
   ```

3. **集成到 ReplayVoiceManager**:
   ```cpp
   void UReplayVoiceManager::PlayComment(const FString& CommentText) {
       if (bTTSEnabled && TTSBackend) {
           EMotionState Emotion = GetCurrentCommentEmotion();
           float Rate = (Emotion == EMotionState::Excited) ? 1.2f : 1.0f;
           float Volume = (Emotion == EMotionState::Calm) ? 0.7f : 1.0f;
           
           TTSBackend->OnTTSComplete.Clear();
           TTSBackend->OnTTSComplete.AddLambda([this]() {
               // 完成回调: 驱动队列中的下一条
               TryPlayNextInQueue();
           });
           
           TTSBackend->SetMaxSpeakDuration(15.0f); // 最长说话15秒
           TTSBackend->Speak(CommentText, Volume, Rate);
       }
   }
   
   // 降级方案: 如果两周内 TTS 集成未完成
   // 使用预录制的 WAV 文件播放核心 50 条评论
   // 估时 6h (录制+导入UE5) + 无需 COM/线程管理
   ```

4. **缓震/去重**:
   - 同一评论在 5 秒内不重复
   - TTS 说话时不打断(除非优先级更高的事件)
   - 说话中不做新的 TTS 调用
   - `IsSpeaking()` 轮询仅作为兜底(每30帧检查一次超时)

**验收标准**:
- [ ] NOMI 在 Windows 上用 SAPI5 开口说话
- [ ] NOMI 在 macOS 上用 NSSpeechSynthesizer 开口说话
- [ ] TTS 完成通过事件回调驱动(非轮询)
- [ ] 硬超时保护生效(超过15秒自动截断)
- [ ] 不同情绪有不同的语速/音量
- [ ] 同一评论在冷却期内不重复
- [ ] 降级方案(预录制WAV)可用

**预计工时**: 25~30 小时 (含 COM 生命周期、线程安全、超时保护)

---

### 6.2 NOMI 记忆系统

**文件**: `NOMI/CommentaryEngine.cpp`, `NOMI/CommentaryEngine.h`, `Race/RaceProgression.cpp`

**当前问题**: NOMI 每场比赛都是"失忆"的。

**实施**:

1. **记忆数据结构**:
   ```cpp
   // CommentaryEngine.h
   USTRUCT()
   struct FNOMIMemory {
       FString LastTrackPlayed;
       FString LastVehicleUsed;
       int32 LastRacePosition;
       float BestLapTimeOnTrack;
       FString BestVehicleOnTrack;
       int32 ConsecutiveRacesPlayed;
       int32 TotalRacesPlayed;
       float TotalPlayTimeMinutes;
   };
   ```

2. **记忆持久化 (原子写入保护)**:
   ```cpp
   void UCommentaryEngine::SaveMemory() {
       // 写入原子化: 临时文件 → 验证 → rename
       FString TempPath = GetMemoryFilePath() + ".tmp";
       FString FinalPath = GetMemoryFilePath();
       
       Memory.Version = FNOMIMemory::CURRENT_VERSION;  // 版本号
       Memory.CRC32 = CalculateCRC32(Memory);          // CRC校验
       
       FFileHelper::SaveStringToFile(Memory.ToJson(), TempPath);
       // 验证临时文件完整性
       IFileManager::Get().Move(*FinalPath, *TempPath);
   }
   
   void UCommentaryEngine::LoadMemory() {
       FString FilePath = GetMemoryFilePath();
       FString JsonContent;
       if (FFileHelper::LoadFileToString(JsonContent, FilePath)) {
           FNOMIMemory Loaded;
           if (Loaded.FromJson(JsonContent) && 
               Loaded.CRC32 == CalculateCRC32(Loaded) &&
               Loaded.Version <= FNOMIMemory::CURRENT_VERSION) {
               Memory = Loaded;
               return;
           }
       }
       // 读取失败时: 空记忆(不是崩溃或静默失败)
       Memory = FNOMIMemory();
   }
   ```

3. **基于记忆的评论生成**:
   ```cpp
   FString UCommentaryEngine::GenerateMemoryBasedComment() {
       if (Memory.TotalRacesPlayed == 0) {
           return "欢迎来到NIO Racing Plus！我是你的AI副驾NOMI！";
       }
       if (CurrentTrack == Memory.LastTrackPlayed && Memory.BestLapTimeOnTrack > 0) {
           return FString::Printf(TEXT("欢迎回来！上次在这条赛道你跑了 %.2f 秒，看看这次能不能更快！"), Memory.BestLapTimeOnTrack);
       }
       // ... 更多记忆组合
   }
   ```

**验收标准**:
- [ ] NOMI 记住上场比赛的赛道和车辆
- [ ] NOMI 记住最好圈速并在重访赛道时提及
- [ ] 记忆写入原子化(写临时文件→验证→rename)
- [ ] 记忆文件有版本号和 CRC 校验
- [ ] 读取失败时回退到空记忆(非崩溃)
- [ ] 首次游玩 NOMI 有欢迎语

**预计工时**: 8~12 小时 (含原子写入+CRC+版本号)

---

### 6.3 NOMI 评论频率优化 — 分层冷却

**文件**: `NOMI/CommentaryEngine.cpp`, `NOMI/CommentaryEngine.h`

**当前问题**: 评论频率可能过高或过低，没有分层控制。

**实施**:

1. **分层冷却系统**:
   ```cpp
   enum class ECommentPriority {
       Critical,  // 冲线、夺冠、重大事件 — 总是触发
       High,      // 超车、被超、碰撞 — 冷却 10s
       Medium,    // 圈速提升、位置变化 — 冷却 20s
       Low,       // 空闲评论、环境评论 — 冷却 30s
       Ambient,   // 背景闲聊 — 冷却 60s+，只在空闲>15s触发
   };
   
   // 每层独立冷却计时器
   TMap<ECommentPriority, float> CooldownTimers;
   ```

2. **动态冷却调整**:
   - 比赛前5圈: 冷却较短(评论更密集)
   - 最后一圈: 冷却极大缩短(评论更密集)
   - 玩家连续失误: 增加安慰评论概率

3. **评论队列**:
   ```cpp
   struct FCommentRequest {
       FString Text;
       ECommentPriority Priority;
       EMotionState Emotion;
       float Timeout;  // 若超时未播放则丢弃
   };
   TQueue<FCommentRequest> CommentQueue;
   ```

**验收标准**:
- [ ] 不同优先级独立冷却
- [ ] 比赛紧张时评论密度增加
- [ ] 不会出现评论"轰炸" (>2条/10秒)
- [ ] 空闲超过10秒有环境评论

**预计工时**: 3~5 小时

---

### 6.4 NOMI 人格面具 — 每辆车不同的评论风格

**文件**: `NOMI/CommentaryEngine.cpp`, `NOMI/CommentaryEngine.h`, `DefaultComments.json`

**实施**:

1. **车辆→人格映射**:
   ```cpp
   // CommentaryEngine.h
   enum class ECommentPersona {
       Competitive,   // EP9 — 赛道竞技、激烈、专业术语
       Relaxed,       // ET7 — 舒适、续航、长途旅行
       Adventurous,   // ES7 — 户外、远足、视野
       Sporty,        // ET5 — 运动感、年轻、时尚
       Power,         // SU7 Ultra — 极致性能、前卫科技
       Neutral        // Custom — 默认
   };
   
   ECommentPersona GetPersonaForVehicle(const FString& VehicleName);
   ```

2. **评论过滤** — 根据人格筛选评论池:
   ```cpp
   // 加载评论时根据当前车辆人格筛选
   TArray<FCommentEntry> Pool = LoadCommentPool(/*...*/);
   Pool.FilterByPersona(GetPersonaForVehicle(CurrentVehicle));
   
   // 回退: 如果当前人格没有匹配评论，使用 Neutral
   ```

3. **人格化 JSON 扩展**:
   ```json
   {
     "overtake": {
       "competitive": ["干净利落的超车！这就是EP9的实力！"],
       "relaxed": ["平稳地超越对手，一切尽在掌控。"]
     }
   }
   ```

**验收标准**:
- [ ] EP9 上车后 NOMI 语气更激烈
- [ ] ET7 上车后 NOMI 语气更放松
- [ ] 不同人格使用不同的评论池
- [ ] 人格切换在车库选择车辆后立即生效

**预计工时**: 4~6 小时

---

### 6.5 NOMI 英语评论本地化

**文件**: `Content/NOMI/Comments/DefaultComments.json`, `NOMI/CommentaryEngine.cpp`

**实施**:

1. **评论 JSON 结构化** — 双语言:
   ```json
   {
     "overtake": {
       "first_place": {
         "zh": ["漂亮！{player}超越{rival}领跑！"],
         "en": ["Amazing! {player} takes the lead from {rival}!"]
       }
     }
   }
   ```

2. **语言选择逻辑**:
   ```cpp
   FString GetCurrentLanguage() const;
   // 使用 FInternationalization 或直接从游戏设置读取
   ```

3. **英文评论撰写** (非翻译，英语思维写作):
   - 重点: 使用符合英语文化的赛车表达
   - 如: "P1 baby!"、"Hold the line!"、"That's how it's done!"
   - 避免中式英语

4. **评论覆盖度目标**:
   - 第一波: 100 条核心英文评论(覆盖高频事件)
   - 后续: 社区贡献扩展

**验收标准**:
- [ ] 英文设置下 NOMI 说英文评论
- [ ] 中文设置下 NOMI 说中文评论
- [ ] 英文评论语法和表达自然
- [ ] 首波至少 100 条英文评论

**预计工时**: 8~12 小时 (内容撰写)

---

### Iteration 4 出口标准

- [ ] NOMI 通过 TTS 开口说话(Windows + macOS)
- [ ] NOMI 记忆系统实现并持久化
- [ ] 评论冷却分层工作正常
- [ ] 每辆车有不同的 NOMI 人格面具
- [ ] 英文评论第一波完成
- [ ] 回归测试通过

**总预计工时**: 37~53 小时

---

## 8. 迭代 5: UI/UX 全面打磨 (Week 8-9)

### 7.1 完整 UI 动画系统

**文件**: 所有 UI Widget 文件

在 Iteration 2 基础框架上铺开:

1. **页面过渡标准化**:
   - `MenuManager`: 所有页面切换有统一过渡(淡入淡出 + 轻微缩放)
   - 动画时长: 200ms (入), 150ms (出)
   - 动画曲线: Cubic Ease In/Out

2. **HUD 元素动画**:
   - 速度表数字: 平滑变化(不是直接SetText)
   - 分段计时: 刷出动画 + 颜色闪烁(新纪录时)
   - 排名变化: 滑动动画(上升/下降)
   - 轮胎温度指示: 颜色渐变(蓝→绿→红)

3. **奖牌/成就动画**:
   - 比赛结束时: 奖牌从屏幕外飞入
   - 新的最好成绩: 金色闪光粒子 + 缩放

**验收标准**:
- [ ] 所有页面切换有过渡动画
- [ ] HUD 数值变化有平滑动画
- [ ] 比赛结束有奖牌展示动画
- [ ] 动画不引入卡顿(FPS 稳定)

**预计工时**: 8~12 小时

---

### 7.2 动态 HUD — 场景感知信息权重

**文件**: `RaceHUD.cpp`, `RaceHUD.h`

**当前问题**: HUD 在比赛各个阶段显示相同的信息。

**实施**:

1. **场景定义**:
   ```cpp
   enum class ERaceScene {
       Start,      // 起步: 显示转速/离合信息
       Racing,     // 正常比赛: 标准布局
       CloseFight, // 近身缠斗: 强调对手信息和距离
       FinalLap,   // 最后一圈: 突出圈数和提醒
       Crash,      // 碰撞后: 突出损坏状态
   };
   auto DetectRaceScene() -> ERaceScene;
   ```

2. **权重调整**:
   - `CloseFight` 场景: 对手差异(时间差)*2 大字显示，轮胎温度隐藏
   - `FinalLap` 场景: 圈数计数闪烁 + NOMI 评论增加
   - `Crash` 场景: 损坏度/维修进度/倒计时 前置

3. **最小侵入式布局**:
   - `Start` 场景: 只显示转速和起步提示
   - 其他场景元素默认半透明，场景相关元素高亮

**验收标准**:
- [ ] HUD 在不同比赛阶段自动调整
- [ ] 近身缠斗时对手信息突出
- [ ] 最后一圈有明显视觉提醒
- [ ] 场景切换无闪烁

**预计工时**: 6~10 小时

---

### 7.3 涂装系统(基础版)

**文件**: UI 相关 + 车辆渲染配置

**当前问题**: 没有颜色/涂装自定义。

**实施**:

1. **车身颜色选择器**:
   - 10 种基础颜色 + 5 种金属色
   - 颜色选择网格 UI
   - 即时预览(3D 车辆旋转视角)

2. **颜色应用到车辆**:
   ```cpp
   // 设置车辆材质颜色参数
   void SetVehicleBodyColor(const FLinearColor& NewColor) {
       if (UMaterialInstanceDynamic* MID = VehicleMesh->CreateAndSetMaterialInstanceDynamic(0)) {
           MID->SetVectorParameterValue("BodyColor", NewColor);
       }
   }
   ```

3. **应用/取消确认交互**:
   - 颜色修改先进入预览状态(不持久化)
   - 预览 → 点击"应用" → 保存
   - 预览 → 点击"取消" → 恢复到上次保存颜色
   - 新增"恢复默认"按钮

4. **颜色持久化**:
   ```cpp
   // 存储到 ProgressionSerializer 或 GarageSettings
   void SavePlayerColor(const FString& VehicleName, const FLinearColor& Color);
   FLinearColor LoadPlayerColor(const FString& VehicleName);
   ```

**验收标准**:
- [ ] 车库中可以选择并预览车身颜色(PaintJob)
- [ ] 车辆在颜色选择器中自动缓慢旋转, 用户可拖拽旋转查看
- [ ] 颜色切换时有 0.3s 材质颜色 Lerp 过渡
- [ ] "应用/取消"按钮组: 应用后持久化, 取消恢复上次
- [ ] 选中颜色在赛道中正确显示
- [ ] 颜色设置持久化保存
- [ ] 至少 10 种颜色可选

**预计工时**: 8~12 小时

---

### 7.4 控制设置 — 键位重映射

**文件**: `UI/SettingsWidget.cpp`, `SettingsWidget.h`

**实施**:

1. **输入设置 UI**:
   - 列表显示所有可配置操作(加速/刹车/转向/换挡等)
   - 点击操作 → 等待输入 → 绑定新键
   - 冲突检测(同一键绑定两个操作时提示)

2. **键位绑定逻辑**:
   ```cpp
   // 使用 UE5 Enhanced Input 系统的玩家映射
   void RemapAction(const FString& ActionName, const FKey& NewKey) {
       if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetSubsystem()) {
           Subsystem->AddPlayerMappedKey(ActionName, NewKey);
       }
   }
   ```

3. **键位配置持久化**:
   ```cpp
   void SaveKeyBindings(const TMap<FString, FKey>& Bindings);
   TMap<FString, FKey> LoadKeyBindings();
   ```

**验收标准**:
- [ ] 所有核心操作可绑定任意键盘键
- [ ] 冲突检测正常工作
- [ ] 键位设置持久化保存
- [ ] 恢复默认按钮工作

**预计工时**: 5~8 小时

---

### 7.5 HUD 赛道小地图

**文件**: `RaceHUD.cpp`

**实施**:

1. **圆形赛道缩略图**:
   - HUD 右下角 180px 圆形小地图
   - 从 `TrackConfig.json` 的 waypoints 渲染简化轨道线
   - 玩家位置三角标记(指向行进方向)
   - 前车/后车位置标记点 + 距离数字

2. **场景感知复用**:
   - `CloseFight` 场景: 小地图放大 + 突出显示对手位置
   - `FinalLap` 场景: 小地图边缘闪烁提示

**验收标准**:
- [ ] 小地图显示赛道拓扑
- [ ] 玩家位置实时更新
- [ ] 前/后车位置可见
- [ ] 不影响核心 HUD 布局

**预计工时**: 4~6 小时

---

### 7.6 暂停菜单 + 结果界面改进

**文件**: `UI/PauseMenuWidget.cpp`, `UI/ResultsWidget.cpp`

**实施**:

1. **暂停菜单增加比赛信息**:
   - 右半屏叠加: 赛道缩略图 + 当前排名列表 + 当前圈vs最佳圈对比
   - 复用 `RaceHUD` 数据的子集渲染

2. **结果界面增加叙事条**:
   - 顶部一句话: "从 P{start} 发车 → P{finish} 完赛, 超越 {overtakes} 辆车"
   - 使用 NOMI 评论引擎的模板系统拼接文本
   - 最佳圈速高亮(金色)

**验收标准**:
- [ ] 暂停菜单显示排名+圈速对比
- [ ] 结果界面顶部有叙事条
- [ ] 比赛数据(发车位/终位/超车数)正确

**预计工时**: 4~6 小时

---

### 7.7 教程重玩 + 自由驾驶练习场

**文件**: `UI/TutorialManager.cpp`, `UI/TutorialWidget.cpp`, `GarageWidget.cpp`

**实施**:

1. **教程重玩入口**:
   - 设置菜单 → "重新开始教程" 按钮
   - 首次跳过教程的玩家可在之后补学

2. **自由驾驶练习场**:
   - 车库界面新增 "自由驾驶" 按钮
   - 复用现有赛道(建议 NIOCityCircuit 或 Desert Rally)
   - 无 AI、无计时、无碰撞伤害
   - 可在练习中随时按 ESC 返回车库

**验收标准**:
- [ ] 设置中有"重新开始教程"按钮
- [ ] 教程可完整重玩
- [ ] 自由驾驶可用(无 AI/无计时)
- [ ] 练习中可随时退出

**预计工时**: 4~6 小时

---

### Iteration 5 出口标准 (修订版)

- [ ] 全 UI 动画系统部署完成
- [ ] 动态 HUD 场景感知工作 + 赛道小地图
- [ ] 涂装系统(基础颜色选择+应用/取消确认)可用
- [ ] 暂停菜单有比赛信息 + 结果界面有叙事条
- [ ] 教程可重玩 + 自由驾驶练习场
- [ ] 键位重映射功能完整
- [ ] HUD 字号自适应缩放(0.8x~1.5x)
- [ ] 回归测试通过

**总预计工时**: 39~60 小时

---

## 9. 迭代 6: 无障碍与游戏模式丰富 (Week 9-10)

### 8.1 辅助功能开关

**文件**: `UI/AccessibilityManager.cpp`, `RaceHUD.cpp`, `NIOVehicleMovementComponent.cpp`

**实施**:

1. **自动转向 Assists**:
   ```cpp
   void ApplySteeringAssist(float& SteeringInput, const FVector& TrackAhead) {
       if (bSteeringAssist) {
           float AssistStrength = SteeringAssistStrength; // 0~1
           FVector IdealDir = /* 赛道前方方向 */;
           float DesiredSteer = CalculateSteerToAlign(IdealDir);
           SteeringInput = FMath::Lerp(SteeringInput, DesiredSteer, AssistStrength);
       }
   }
   ```

2. **自动刹车 Assist**:
   ```cpp
   void ApplyBrakeAssist(float& BrakeInput, float DistanceToCorner) {
       if (bBrakeAssist) {
           if (DistanceToCorner < BrakeThreshold) {
               BrakeInput = FMath::Max(BrakeInput, BrakeAssistStrength);
           }
       }
   }
   ```

3. **牵引力控制** — 限制输出扭矩防止打滑:
   ```cpp
   // 在 NIOVehicleMovementComponent 中
   float TC_ThrottleLimit = 1.0f;
   if (bTractionControl && /* 检测到车轮打滑 */) {
       TC_ThrottleLimit = 0.7f; // 限制输出
       // 当抓地力恢复后逐渐放开
   }
   ActualThrottle *= TC_ThrottleLimit;
   ```

4. **无障碍 UI 入口 (已有代码实现, 缺设置入口)**:
   - 设置菜单增加"无障碍"分组 Tab
   - 色盲模式: 3 种预设(红绿色盲/蓝黄色盲/全色盲) — 代码已实现
   - 字幕大小: 滑块 0.8x~1.5x — 代码已实现
   - 对比度模式: 高对比度开关 — 代码已实现
   - 文字缩放: 百分比滑块 — 代码已实现

5. **HUD 字号缩放**:
   ```cpp
   // 在 RaceHUD 基类中, 从 AccessibilityManager 读取缩放比例
   float HUDScalar = GetAccessibilityManager()->GetHUDScalar(); // 0.8~1.5
   CanvasPanel->SetRenderScale(FVector2D(HUDScalar, HUDScalar));
   // 分屏模式下自动启用 1.2x+
   ```

6. **辅助驾驶 UI 面板**:
   - 设置菜单中的"辅助驾驶"分组 (与"无障碍"分开)
   - 每个辅助选项有滑块(强度)/开关
   - 说明文字解释每个选项的效果

**验收标准**:
- [ ] 无障碍设置 Tab 包含色盲/字幕/对比度/文字缩放入口 (已有代码, 补UI)
- [ ] HUD 字号可缩放(0.8x~1.5x)
- [ ] 所有辅助驾驶开关在设置中可配置
- [ ] 自动转向有效减少赛道偏离
- [ ] 自动刹车在弯道前正确减速
- [ ] 牵引力控制减少低抓地力车轮空转
- [ ] 辅助可以在比赛中动态开启/关闭

**预计工时**: 8~12 小时

---

### 8.2 起步完美判定

**文件**: `RaceManager.cpp`, `RaceProgressionTypes.h`

**实施**:

1. **起步信号系统**:
   ```cpp
   // 红灯序列: 5盏灯逐一亮起，随机延迟后全部熄灭 = 起步!
   void FRaceStartController::BeginStartSequence() {
       LightsOn[0] = true;  // 250ms后
       LightsOn[1] = true;  // 250ms后
       LightsOn[2] = true;  // 250ms后
       LightsOn[3] = true;  // 250ms后
       LightsOn[4] = true;  
       // 随机等待 500~1500ms
       // 全部熄灭 → 起步!
   }
   ```

2. **完美起步判定**:
   ```cpp
   enum class EStartQuality {
       Perfect,    // 灯灭后 < 100ms 给油
       Good,       // 灯灭后 100~300ms
       Average,    // 灯灭后 300~500ms
       Slow,       // 灯灭后 > 500ms
       FalseStart, // 灯灭前给油 → 罚时/退回
   };
   ```

3. **起步效果**:
   - Perfect: 初始扭矩 +15% (1秒)
   - FalseStart: 罚时 3秒 (固定)
   - NOMI 根据起步质量评论

**验收标准**:
- [ ] 起步灯序列可见
- [ ] 完美起步有奖励
- [ ] 抢跑被检测并惩罚
- [ ] NOMI 评论起步质量

**预计工时**: 4~6 小时

---

### 8.3 漂移辅助模式

**文件**: `NIOVehicleMovementComponent.cpp`, `TirePhysicsModel.cpp`

**实施**:

1. **漂移辅助逻辑**:
   ```cpp
   void ApplyDriftAssist(float& SteeringInput, float Throttle, float SlipAngle) {
       if (bDriftAssist && IsPlayerDrifting()) {
           // 检测到漂移时，轻微调整转向帮助维持
           float TargetSlipAngle = 25.0f; // 目标漂移角度
           float SlipError = TargetSlipAngle - SlipAngle;
           SteeringInput += SlipError * 0.1f;
           
           // 漂移时自动调节油门维持
           // 不是全油门也不是全松
       }
   }
   ```

2. **漂移评分系统**:
   - 角度 × 速度 × 持续时间 → 漂移分
   - 连续漂移: 倍数加成

3. **漂移模式 UI**:
   - 计数: 漂移分、连续漂移数
   - NOMI: 评价漂移表现

**验收标准**:
- [ ] 开启漂移辅助后漂移更容易维持
- [ ] 漂移评分系统工作
- [ ] 关闭辅助后漂移行为恢复正常

**预计工时**: 4~6 小时

---

### 8.4 天气随机事件

**文件**: `RaceManager.cpp`, `TirePhysicsModel.cpp`

**实施**:

1. **天气变化事件**:
   ```cpp
   // RaceManager 中: 比赛进行到 30% 时检测是否触发天气变化
   // 概率: 每场比赛 20%
   // 类型: 小雨(摩擦 -15%), 大雨(-30%), 夜晚(仅视觉)
   // 过渡时间: 30 秒(平滑过渡)
   ```

2. **NOMI 天气评论**:
   - 雨开始时: "下雨了，赛道会变滑，小心驾驶"
   - 大雨时: "雨越来越大了，要不要进站换雨胎？"(幽默)
   - 雨停: "雨停了，赛道正在变干"

3. **视觉切换**:
   - 粒子: 雨滴效果(Niagara System)
   - 后处理: 对比度降低，路面反射增强

**验收标准**:
- [ ] 比赛中有概率出现天气变化
- [ ] 天气变化对摩擦系数有影响
- [ ] NOMI 对天气变化有评论
- [ ] 天气可在设置中关闭

**预计工时**: 4~6 小时

---

### Iteration 6 出口标准

- [ ] 辅助功能(自动转向/刹车/牵引力控制)可用
- [ ] 起步完美判定系统工作
- [ ] 漂移辅助模式可用
- [ ] 天气随机事件工作
- [ ] 回归测试通过

**总预计工时**: 20~30 小时

---

## 10. 迭代 7: 社区基础设施 (Week 10-11)

### 9.1 add_vehicle.py 管道 (分阶段交付)

**文件**: `Scripts/add_vehicle.py` (新建), `Content/Vehicles/VehicleConfig.json`, `Vehicles/` 车辆源文件

**当前问题**: README 中提到了 `add_vehicle.py` 但实际不存在。

**架构决策**: 管道输出应为**数据驱动**(JSON + 资产引用)而非C++源码生成。避免每辆车需要独立C++类→触发UE5编译的慢路径。

#### Phase A — JSON 配置生成 + 通用车辆支持 (12h)

1. **新增通用车辆 Actor**:
   ```cpp
   // Vehicles/AGenericVehicle.h — 运行时从 VehicleConfig.json 读取物理参数
   // 继承 NIOVehicleBase, 但物理参数在 BeginPlay 时从配置动态加载
   // 无需为每辆车单独编写 C++ 类
   ```

2. **管道功能 Phase A**:
   ```python
   # add_vehicle.py --name "NIO_ET9" --type hypercar --wheelbase 3100 --drag 0.25
   
   # Phase A 实现:
   # 1. 生成 JSON 配置块追加到 VehicleConfig.json
   # 2. 生成 Pacejka 参数(使用参数数据库或默认值)
   # 3. 更新 VehicleStateManager 的车辆枚举(自动注册)
   # 4. 输出结果 = JSON + 无需 C++ 编译, 重启UE5即可生效
   ```

3. **参数验证**:
   - 验证车辆名不重复
   - 验证物理参数在合理范围
   - 验证必要的必填项

#### Phase B — 3D 资产导入 (15h, 按需启动)

1. **Sketchfab 下载** (可选):
   - 使用 Sketchfab API 下载模型
   - 失败降级: 提示手动下载并指定本地文件路径

2. **Blender 自动展 UV**:
   - 使用 Blender Python API 自动展 UV + 导出 FBX
   - 需处理不同模型的 UV 布局差异(大量边界情况)
   - `--offline` 模式: 仅生成 JSON 骨架, 资产留空

#### Phase C — Pacejka 参数生成 (8-10h, 后续迭代)

1. **Pacejka 参数数据库**:
   - 收集 20+ 款电动车的真实轮胎测试数据
   - 基于 wheelbase/weight/tire_width 做线性插值
   - 初期: 使用硬编码默认值(B=10, C=1.4, D=1.0, E=-0.2)

**验收标准 (Phase A)**:
- [ ] `add_vehicle.py` 运行成功生成 JSON 配置块
- [ ] VehicleConfig.json 正确追加
- [ ] 车辆通过 `AGenericVehicle` 在游戏中选择并驾驶
- [ ] Phase B/C 在 README 中标记为 alpha/coming soon

**预计工时**: 30~40 小时 (三阶段合计); Phase A 独立 12h 可交付
> 当前 README 中的说法(管道功能完整)与实际能力脱节会损害社区信任。建议先标记 `add_vehicle.py` 为 alpha。

---

### 9.2 CI/CD — GitHub Actions

**文件**: `.github/workflows/build.yml` (新建)

**实施**:

1. **Build Pipeline**:
   ```yaml
   name: Build and Test
   on: [push, pull_request]
   jobs:
     build:
       runs-on: ubuntu-latest
       steps:
         # UE5 的自动构建限制(UE5需要Windows/macOS agent)
         # 替代: 编译检查、测试运行
         - uses: actions/checkout@v4
         - name: Run C++ Syntax Check
           run: ./Scripts/check_syntax.sh
         - name: Run Unit Tests
           run: ./Scripts/run_tests.sh
   ```

2. **代码质量检查**:
   - C++ 代码风格检查(clang-format)
   - JSON 格式验证(所有配置文件)
   - 文件命名规范检查

3. **Issue/PR 模板**:
   - `.github/ISSUE_TEMPLATE/bug_report.md`
   - `.github/ISSUE_TEMPLATE/feature_request.md`
   - `.github/PULL_REQUEST_TEMPLATE.md`

**验收标准**:
- [ ] GitHub Actions 在 PR 时运行
- [ ] JSON 格式验证通过
- [ ] C++ 风格检查通过
- [ ] Issue/PR 模板可用

**预计工时**: 4~6 小时

---

### 9.3 开发者文档 — CONTRIBUTING.md

**文件**: `CONTRIBUTING.md` (新建)

**内容**:
1. 如何设置 UE5 开发环境
2. 构建和测试指南
3. 代码风格规范
4. PR 提交流程
5. 添加新车辆的步骤
6. 本地化贡献指南
7. 社区行为准则

**验收标准**:
- [ ] CONTRIBUTING.md 完整覆盖以上内容
- [ ] 链接可访问
- [ ] 3 个示例步骤任何人都能完成

**预计工时**: 3~5 小时

---

### 9.4 Discord 社区基础

**通道结构**:
```
#welcome — 新玩家入口
#announcements — 开发更新
#general — 社区交流
#gameplay — 游戏讨论
#bug-reports — 问题反馈
#suggestions — 功能建议
#vehicle-modding — 车辆/改装讨论  
#development — 开发协作
#media — 截图/视频分享
```

**工具集成**:
- Discord Webhook → GitHub (新Release通知)
- Discord Bot 基础命令

**验收标准**:
- [ ] Discord 服务器创建并配置基础通道
- [ ] GitHub 集成(可选)

**预计工时**: 2~3 小时

---

### Iteration 7 出口标准

- [ ] add_vehicle.py 管道可用
- [ ] GitHub Actions 配置完成
- [ ] CONTRIBUTING.md 完整
- [ ] Discord 社区通道就绪
- [ ] 回归测试通过

**总预计工时**: 21~30 小时

---

## 11. 迭代 8: 发布准备 — 分屏 + 锦标赛 + Steam (Week 11-13)

### 10.1 本地分屏多人

**文件**: `RaceManager.cpp`, `NomiRaceGameMode.cpp`, UI 扩展

**实施**:

1. **UE5 本地多玩家架构**:
   ```cpp
   // 使用 UE5 的本地多玩家系统
   // GameMode 中创建多个 Local Player
   UGameInstance* GI = GetGameInstance();
   GI->CreateLocalPlayer(1, /* out Error */, /* bSpawnController */ true);
   GI->CreateLocalPlayer(2, /* out Error */, /* bSpawnController */ true);
   ```

2. **HUD 架构预重构**:
   - 当前 `RaceHUD` 是全局单例, 分屏需要每位玩家独立实例
   - **预重构**: 拆分 `ARaceHUDManager`(全局: 管理分屏布局 + 事件路由) 和 `APlayerRaceHUD`(Per-Player: 各玩家的速度/排名/圈速)
   - 非分屏模式: 复用单实例路径, 不增加复杂度

3. **分屏渲染**:
   - 水平分割(宽屏)或垂直分割(标准屏)
   - 每个玩家独立的摄像机
   - 每个 `APlayerController` 绑定独立的 `APlayerRaceHUD` 实例
   - HUD 在分屏下自动适配视口大小

4. **分屏 UI**:
   - 分屏游戏模式选择
   - 玩家各自选择车辆和赛道
   - 暂停菜单仅对操作者有效("仅暂停者有效"逻辑)
   - 分屏过程中可暂停退出

5. **AI 填充**:
   - 1P + AI = 6辆 (1 player)
   - 2P + AI = 6辆 (2 players split)

**验收标准**:
- [ ] HUD 预重构完成: APlayerRaceHUD + ARaceHUDManager 分离
- [ ] 2 个本地玩家可同时进行游戏
- [ ] 分屏正确渲染(水平/垂直)
- [ ] 各玩家独立控制车辆
- [ ] 比赛完全功能(位置跟踪、圈数、计时)
- [ ] FPS 在分屏模式下保持 > 30
- [ ] 暂停菜单仅影响操作者

**预计工时**: 30~40 小时 (含 HUD 预重构 + 分屏适配)

---

### 10.2 锦标赛 UI

**文件**: `Race/ChampionshipManager.cpp`, UI Widgets

**实施**:

1. **锦标赛可视化**:
   - 赛程表: 下拉树展示每轮比赛和对手
   - 积分榜: 实时更新的排名+积分图表
   - 下一场预告: 赛道预览+建议车辆

2. **锦标赛进程**:
   ```cpp
   // ChampionshipManager 完善:
   // 1. 比赛间数据持久化
   // 2. 自动推进下一轮
   // 3. 最终结果展示(奖杯/🏆)
   ```

**验收标准**:
- [ ] 锦标赛菜单显示赛程和积分
- [ ] 比赛结果自动更新积分榜
- [ ] 锦标赛完成有奖杯展示

**预计工时**: 6~8 小时

---

### 10.3 Steam 上架准备

**文件**: 配置 + 文档

**流程**:

1. **Steamworks SDK 集成** (可选, 取决于是否使用 Steam 功能):
   - 成就系统
   - Steam 排行榜(作为云端排行榜)
   - Steam 游戏内覆盖层

2. **商店页面准备**:
   - 游戏描述(中英文)
   - 屏幕截图(10+ 张)
   - 预告片(录屏)
   - 分类标签: Racing, Simulation, Open Source, Electric Vehicles

3. **Steam 审核准备**:
   - 100 美元发布费用
   - 构建配置(Windows/macOS)
   - 合规检查(GDPR、退款政策)

4. **定价策略**:
   - 建议: 免费(Free to Play)
   - 可选: "支持者" DLC 付费包(含额外内容)

**验收标准**:
- [ ] Steam 开发者账号准备
- [ ] 商店页面草稿完成
- [ ] 构建配置验证通过

**预计工时**: 8~12 小时 (不含审批等待)

---

### Iteration 8 出口标准

- [ ] 本地分屏多人工作(2P)
- [ ] 锦标赛 UI 可视化完整
- [ ] Steam 商店页面就绪
- [ ] 全系统回归测试通过
- [ ] 最终的"1.0"构建可用

**总预计工时**: 30~40 小时

---

## 12. 验证与测试策略

### 11.1 每迭代验证清单

每次迭代完成时执行:

```bash
# 1. 编译检查
# 在 UE5 编辑器中编译 C++ 项目

# 2. 现有单元测试
# Window → Test Automation → 运行所有测试

# 3. 核心功能冒烟测试 (见 11.2)
```

### 11.2 自动化冒烟测试 (渐进式)

**目标**: 将高频回归点做成 UE5 Functional Test, 降低社区贡献者的合入门槛。

**实施步骤**:

1. **GitHub Actions workflow**:
   ```yaml
   name: Smoke Test
   on: [push, pull_request]
   jobs:
     smoke:
       runs-on: ubuntu-latest
       steps:
         - uses: actions/checkout@v4
         - name: Run C++ Syntax Check
           run: ./Scripts/check_syntax.sh
         - name: JSON Format Validation
           run: python Scripts/validate_json.py
         - name: Test Header Pragma Check
           run: ./Scripts/check_pragma_once.sh
   ```

2. **UE5 Functional Test 路线图**:
   - Phase 1 (Iter 2): 编译检查 + JSON 验证 + 头文件规范
   - Phase 2 (Iter 5): 基础 Functional Test (启动→菜单→快速比赛→退出)
   - Phase 3 (Iter 8): 全冒烟测试自动化

### 11.3 存档版本号 + 数据迁移框架

**文件**: `Race/ProgressionSerializer.cpp`

**当前问题**: 多个 Iter 会新增存档字段(Iter 2 TimeTrial, Iter 4 NOMI Memory, Iter 5 Paint), 但无版本兼容性策略。

**实施**:

```cpp
// ProgressionSerializer.h
USTRUCT()
struct FSaveDataHeader {
    GENERATED_BODY()
    
    UPROPERTY() int32 Version = 1;          // 存档版本号
    UPROPERTY() FDateTime SaveTime;
    UPROPERTY() FString GameVersion;         // "v0.8.0"
    UPROPERTY() uint32 CRC32 = 0;            // CRC校验
};

// 读取时检查版本号
FSaveDataHeader Header = LoadHeader(SavePath);
if (Header.Version < CURRENT_SAVE_VERSION) {
    MigrateSaveData(SavePath, Header.Version, CURRENT_SAVE_VERSION);
}

// 每个 Iter 新增字段时递增版本
// Iter 2 → Version 2: FTimeTrialRecord 新增
// Iter 4 → Version 3: FNOMIMemory 新增
// Iter 5 → Version 4: PaintColor 新增
```

**规则**:
- 每个 Iter 修改存档格式时递增版本号
- 写迁移函数 `Migrate_V{N}_to_V{N+1}()`, 新字段填充默认值
- 读取失败时回退到最近的可用版本(非崩溃)

### 11.4 核心功能冒烟测试手册

```markdown
## 冒烟测试清单

### 每次迭代后执行

- [ ] 游戏启动 → 主菜单加载成功
- [ ] 菜单导航 → 所有子页面可进入/返回
- [ ] 车辆选择 → 6辆车都可选显示
- [ ] 赛道选择 → 5条赛道都可选
- [ ] 快速比赛 → 加载赛道、开始比赛、完成一圈
- [ ] 碰撞检测 → 撞墙/撞车无异常
- [ ] 完成比赛 → 结果显示正常
- [ ] 返回主菜单 → 无崩溃

### 物理相关迭代额外检查

- [ ] EP9 加速0-100(直线) < 3秒
- [ ] ES7 过弯侧倾明显 > EP9
- [ ] 刹车距离合理(100-0 < 40m)
- [ ] 漂移后轮胎温度上升
- [ ] 不同赛道抓地力感觉不同

### NOMI 相关迭代额外检查

- [ ] NOMI 评论在中/英设置下语言正确
- [ ] NOMI 记忆在重启后保留
- [ ] 连续快速事件不会评论轰炸
- [ ] TTS 说话不卡顿
```

### 11.5 性能指标

| 指标 | 当前 | 目标 | 测量工具 |
|------|------|------|----------|
| 编辑器 FPS (城市赛道) | N/A | > 60 | Stat FPS |
| 构建 FPS (城市赛道) | N/A | > 120 | Stat FPS |
| 分屏 FPS | N/A | > 30 | Stat FPS |
| 加载时间 | N/A | < 15s | 手工计时 |
| HUD 更新 CPU 占用 | N/A | < 0.5ms | Unreal Insights |

### 11.6 回归测试优先级

```
P0 (每次提交必测):
├── 启动 → 主菜单
├── 快速比赛(任意车+赛道)3圈
└── 返回主菜单 → 退出

P1 (每次迭代末测):
├── 所有赛道可玩
├── 所有车辆可选
├── AI 比赛完整(6辆)
└── 设置保存/加载

P2 (每两迭代末测):
├── 计时赛
├── NOMI 评论
├── 车库功能
└── 本地排行榜
```

---

## 13. 附录: 快速参考

### 12.1 文件变更总览

| 区域 | 文件 | Iteration | 变更类型 |
|------|------|-----------|----------|
| **物理** | `TirePhysicsModel.cpp/h` | 0, 1 | 重构/增强 |
| **物理** | `NIOVehicleMovementComponent.cpp/h` | 0, 1 | 重构/增强|
| **物理** | `NIOVehicleBase.cpp/h` | 1 | 增强 |
| **物理** | `NIOTirePresets.h` | 1 | 增强 |
| **物理** | `VehicleConfig.json` | 1 | 扩展 |
| **AI** | `AICarController.cpp/h` | 3 | 重构 |
| **AI** | `AIBehaviorTree.cpp/h` | 3 | 增强 |
| **AI** | `AIProfiles.json` | 3 | 扩展 |
| **AI** | `TrackConfig.json` | 3 | 扩展 |
| **NOMI** | `CommentaryEngine.cpp/h` | 4 | 增强 |
| **NOMI** | `NOMIController.cpp/h` | 4 | 增强 |
| **NOMI** | `ReplayVoiceManager.cpp/h` | 4 | 重构 |
| **NOMI** | `TTSBackend.h` | 4 | **新建** |
| **NOMI** | `TTSBackend_Platform.cpp` | 4 | **新建** |
| **NOMI** | `DefaultComments.json` | 4 | 扩展 |
| **NOMI** | `NOMIMemory.json` | 4 | **新建** |
| **Race** | `RaceManager.cpp/h` | 2, 6, 8 | 增强 |
| **Race** | `RaceProgression.cpp/h` | 2, 4 | 增强 |
| **Race** | `RaceProgressionTypes.h` | 2 | 扩展 |
| **Race** | `ChampionshipManager.cpp/h` | 8 | 增强 |
| **Race** | `ProgressionSerializer.cpp/h` | 2, 4 | 增强 |
| **Race** | `TimeTrialManager.cpp/h` | 2 | **新建** |
| **UI** | `LoadingScreenWidget.cpp/h` | 2 | 增强 |
| **UI** | `RaceHUD.cpp/h` | 2, 5 | 重构 |
| **UI** | `SettingsWidget.cpp/h` | 5 | 增强 |
| **UI** | `AccessibilityManager.cpp/h` | 6 | 增强 |
| **UI** | `MainMenuWidget.cpp/h` | 5 | 增强 |
| **UI** | `MenuManager.cpp/h` | 5 | 增强 |
| **UI** | `GarageWidget.cpp/h` | 5 | 增强 |
| **基础设施** | `add_vehicle.py` | 7 | **新建** |
| **基础设施** | `.github/workflows/build.yml` | 7 | **新建** |
| **基础设施** | `CONTRIBUTING.md` | 7 | **新建** |

### 12.2 总工时估算

| 迭代 | 预计工时(最小) | 预计工时(最大) | 人周(1人) | 备注 |
|------|---------------|---------------|-----------|------|
| Iter 0 | 25h | 35h | ~0.7周 | +客观指标, A/B/C验证 |
| Iter 1 | 17h | 27h | ~0.6周 | +地面传导热交换 |
| Iter 1.5 | 15h | 20h | ~0.4周 | **新增: 基础音效修复** |
| Iter 2 | 31h | 46h | ~0.9周 | +留存系统(成就墙+每周挑战) |
| Iter 3 | 37h | 54h | ~1.1周 | Waypoints Day 1与Iter 0并行启动 |
| Iter 4 | 39h | 58h | ~1.2周 | TTS上调至25-30h; 记忆+原子写入 |
| Iter 5 | 39h | 60h | ~1.2周 | +小地图/+暂停信息/+叙事条/+教程重玩 |
| Iter 6 | 22h | 34h | ~0.7周 | +无障碍UI入口(已有代码) |
| Iter 7 | 33h | 48h | ~1.0周 | add_vehicle.py三阶段30-40h |
| Iter 8 | 44h | 58h | ~1.2周 | 分屏上调至30-40h(含HUD预重构) |
| **总计** | **302h** | **440h** | **~9周** | 含新增迭代和工时修正 |

> 工时调整说明: 相较于原版(237~345h), 修订版(302~440h)新增了音频修复(Iter 1.5)、留存系统、TTS线程安全、add_vehicle.py数据驱动、分屏HUD预重构、以及更多UI细节。涨幅 ~30% 反映了实际UE5集成的工程量。

### 12.3 一人团队 vs 小团队建议

**一人团队** (建议串行):
- 每天 2~4 小时 → 16~22 周完成全计划
- 重点: Iter 0 → 1 → 1.5 → 2 → 4 → 5
- 缩减: Iter 3 (仅补 waypoints 不做样条), Iter 8 (暂不分屏+HUD预重构)

**两人团队** (建议并行):
- A: Iter 0 → 1 → 1.5 → 2 → 5
- B: WP采集(并行) → 3 → 4 → 6
- 合并: Iter 7 → 8

**三人+团队** (最大并行):
- A: Iter 0 → 1 → 1.5
- B: WP采集(并行) → 3 → 6
- C: Iter 2 → 5
- D(NOMI专门): Iter 4
- 合并: Iter 7 → 8

### 12.4 最高 ROI 子集 (修订版)

如果时间极其有限, 重新权衡取舍 (基于多角色审查建议):

| 优先级 | 项目 | Solo 时间 | 影响评分 | 审查建议 |
|--------|------|-----------|----------|---------|
| ⭐1 | **Pacejka独立 (Iter 0.1+0.2)** | 12h | 10/10 | 保留 |
| ⭐2 | **AI Waypoints 3条赛道 (Day 1启动)** | 20h | 9/10 | 保留, 上调工时 |
| ⭐3 | **计时赛 (Iter 2.1)** | 14h | 9/10 | 保留 |
| ⭐4 | **加载屏幕 (Iter 2.2)** | 5h | 7/10 | 保留 |
| ⭐5 | **基础留存系统 (Iter 2.5)** | 10h | 8/10 | **新增** – 解决"无钩子"核心问题 |
| ⭐6 | **基础音效 (Iter 1.5)** | 15h | 8/10 | **新增** – 替换NOMI TTS完整版 |
| ⭐7 | **HUD分层+NOMI冷却 (Iter 2.3+4.3)** | 6h | 7/10 | **新增** – 体验优化不依赖物理 |
| | **总计 (MVP)** | **82h** | | **移出**: TTS完整版(→P1), UI动画基础(→P1)**移入**: 留存系统, 音效修复, HUD+NOMI优化 |

**82 小时后，游戏体验从 6.0 → 8.0+ 的核心提升。**
> 修订版 MVP 更聚焦: 修复了原版"忽略音效灾难、忽略留存系统"的结构性缺陷。

---

> 本文档基于 `docs/product-critique-senior-player-vs-pm.md` (100轮资深玩家×产品经理深度分析) 编制。
>
> 所有工时估算基于一人全职开发。实际耗时取决于开发者的 UE5 C++ 熟练度、已有资产就绪程度、以及社区贡献。
>
> "技术是为体验服务的，不是反过来。" — 🏎️ 资深玩家
