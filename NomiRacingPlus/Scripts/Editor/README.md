# NIO Racing Plus - Editor Setup Pipeline

## 只需一步

**双击 `NomiRacingPlus.uproject` 打开 UE5 编辑器**，所有 12 步流水线会自动运行。

### 流水线执行内容：
1. ✅ Fix common issues (创建目录/配置/游戏标签)
2. ✅ Import assets (导入 4 台 GLB 车辆、纹理、音频)
3. ✅ Create materials (创建材质)
4. ✅ Build vehicle Blueprints (创建 BP_NIO_EP9 / ET7 / ES7)
5. ✅ Build tracks (创建 5 条赛道 + MainMenu)
6. ✅ Setup audio (配置 SoundClasses / SoundCues)
7. ✅ Configure AI (加载 AIProfiles.json)
8. ✅ Setup UI (创建 Widget Blueprints)
9. ✅ Configure NOMI (加载评论数据)
10. ✅ Compile Blueprints (编译所有蓝图)
11. ✅ Validate project (验证资产依赖)
12. ✅ Save and report (保存并报告)

### 观察进度
- 打开 `Window → Output Log`
- 搜索 `[FullSetup]` 查看每一步的进度

### 首次运行后
- 流水线完成后会创建一个 `.setup_completed` 标记
- 后续打开编辑器时不会再执行流水线

### 重置（重新运行）
如果需要重新运行流水线：
```python
# 在 Output Log 中执行：
exec(open("Scripts/Editor/run_full_setup.py").read())
run_full_setup()
```
或者删除标记文件：`rm Content/../.setup_completed`

---

## 自动化状态

| 阶段 | 方式 | 状态 |
|------|------|------|
| 无头步骤 (5/12) | UE5-Cmd 命令模式 | ✅ 已完成 |
| 完整步骤 (12/12) | `auto_setup.py` + Editor 启动 | ⏳ 等用户打开编辑器 |
| 用户操作 | 双击 `.uproject` | 🧑 只需这一步 |
