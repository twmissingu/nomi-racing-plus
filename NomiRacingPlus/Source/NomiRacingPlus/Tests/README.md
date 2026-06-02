# NIO Racing Plus - 测试文档

## 测试架构

测试文件位于 `Source/NomiRacingPlus/Tests/` 目录，使用 UE5 的自动化测试框架。

## 测试分类

### 1. 单元测试

#### 车辆系统测试 (`VehicleSystemTest.cpp`)
- `FVehicleStateManagerTest` - 状态管理器初始化和状态跟踪
- `FNIOVehiclePhysicsTest` - NIO 车辆物理配置验证
- `FVehicleDriftDetectionTest` - 漂移检测逻辑测试

#### 轮胎物理测试 (`TirePhysicsTest.cpp`)
- `FTirePhysicsBasicTest` - Pacejka 公式基础计算
- `FTireThermalTest` - 轮胎温度模型测试
- `FTireWearTest` - 轮胎磨损计算测试

#### AI 系统测试 (`AITest.cpp`)
- `FAIBehaviorTreeTest` - 行为树决策逻辑
- `FAIOvertakeEvaluatorTest` - 超车评估器测试
- `FAIDefensiveEvaluatorTest` - 防守评估器测试

#### NOMI 系统测试 (`NOMISystemTest.cpp`)
- `FCommentaryEngineTest` - 评论引擎测试
- `FNOMIControllerTest` - NOMI 控制器测试

#### 比赛系统测试 (`RaceSystemTest.cpp`)
- `FRaceManagerTest` - 比赛管理器状态机测试
- `FCheckpointSystemTest` - 检查点系统测试

#### 相机系统测试 (`CameraSystemTest.cpp`)
- `FCameraSystemTest` - 相机模式切换和功能测试

#### 进度系统测试 (`ProgressionTest.cpp`)
- `FRaceProgressionTest` - 比赛进度记录测试
- `FAchievementSystemTest` - 成就系统测试
- `FChampionshipManagerTest` - 锦标赛管理测试

### 2. 集成测试

#### 车辆-比赛集成测试 (`VehicleRaceIntegrationTest.cpp`)
- 车辆与比赛管理器的交互测试
- 检查点触发和圈数计算
- 比赛完成条件验证

#### AI-比赛管理器集成测试 (`AIRaceManagerIntegrationTest.cpp`)
- AI 对手与比赛管理器的交互
- AI 名次更新和比赛状态同步
- 多 AI 竞争场景测试

#### NOMI-事件集成测试 (`NOMIEventIntegrationTest.cpp`)
- NOMI 系统与比赛事件的集成
- 评论触发条件验证
- 情感状态更新测试

### 3. 性能测试

#### 性能基准测试 (`PerformanceBenchmarkTest.cpp`)
- 车辆物理计算性能
- AI 决策性能
- 粒子系统性能
- 内存使用基准

## 测试工具

### TestUtilities (`TestUtilities.cpp/.h`)
- 模拟对象创建
- 测试数据生成
- 断言辅助函数
- 性能测量工具

## 运行测试

### 在 UE5 编辑器中运行

1. 打开 UE5 编辑器
2. 窗口 → 测试自动化
3. 选择测试类别
4. 点击"开始测试"

### 命令行运行

```bash
# 运行所有测试
UE5Editor-Cmd.exe NomiRacingPlus -ExecCmds="Automation RunTests NomiRacingPlus"

# 运行特定测试类别
UE5Editor-Cmd.exe NomiRacingPlus -ExecCmds="Automation RunTests NomiRacingPlus.Unit"
UE5Editor-Cmd.exe NomiRacingPlus -ExecCmds="Automation RunTests NomiRacingPlus.Integration"
UE5Editor-Cmd.exe NomiRacingPlus -ExecCmds="Automation RunTests NomiRacingPlus.Performance"
```

## 测试覆盖率目标

| 组件 | 目标覆盖率 | 当前状态 |
|------|-----------|---------|
| 车辆系统 | 80% | ✅ 达标 |
| 比赛系统 | 80% | ✅ 达标 |
| NOMI 系统 | 75% | ✅ 达标 |
| AI 系统 | 70% | ✅ 达标 |
| 相机系统 | 70% | ✅ 达标 |
| 进度系统 | 80% | ✅ 达标 |

## 测试最佳实践

1. **测试隔离**: 每个测试独立运行，不依赖其他测试
2. **清晰命名**: 测试名称清楚描述测试内容
3. **边界条件**: 测试边界值和异常情况
4. **性能考虑**: 性能测试设置合理阈值
5. **内存管理**: 测试后清理资源

## 添加新测试

1. 在 `Tests/` 目录创建新的 `.h` 和 `.cpp` 文件
2. 继承 `FAutomationTestBase` 或使用 `IMPLEMENT_SIMPLE_AUTOMATION_TEST`
3. 实现 `RunTest()` 方法
4. 在 `.Build.cs` 中添加测试模块依赖（如果需要）

## 常见测试模式

### AAA 模式
```cpp
bool FMyTest::RunTest(const FString& Parameters)
{
    // Arrange - 准备测试数据
    UMyClass* Obj = NewObject<UMyClass>();

    // Act - 执行操作
    Obj->DoSomething();

    // Assert - 验证结果
    TestEqual(TEXT("Result should be X"), Obj->GetResult(), ExpectedValue);

    return true;
}
```

### 边界测试
```cpp
// 测试最小值
TestEqual(TEXT("Min value"), Func(0), 0);

// 测试最大值
TestEqual(TEXT("Max value"), Func(100), 100);

// 测试超出范围
TestEqual(TEXT("Over max"), Func(101), 100); // 应该被限制
```

---

**最后更新**: 2026-06-01
