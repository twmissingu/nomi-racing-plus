# 🚗 自定义车辆导入指南

> **NIO Racing Plus 的核心亮点：支持从 Sketchfab 导入任意 3D 车辆模型！**

---

## 快速开始（3 步添加任何车辆）

### 步骤 1：获取 Sketchfab API Token

1. 访问 https://sketchfab.com/settings/password
2. 登录你的 Sketchfab 账号
3. 复制 **API Token**

### 步骤 2：找到你喜欢的车辆模型

在 Sketchfab 搜索车辆：https://sketchfab.com/search?type=models&q=car

### 步骤 3：一行命令导入！

```bash
# 设置 API Token（只需设置一次）
export SKETCHFAB_TOKEN="你的API_Token"

# 添加车辆！
python3 Scripts/add_vehicle.py "https://sketchfab.com/3d-models/porsche-911-gt3-abc123"
```

---

## 完整命令格式

```bash
python3 Scripts/add_vehicle.py <sketchfab_url> [选项]
```

### 选项参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `--color` | 车漆颜色预设或 RGB 值 | `--color red` 或 `--color 0.8 0.1 0.1` |
| `--name` | 自定义显示名称 | `--name "Porsche 911 GT3"` |
| `--api-token` | Sketchfab API Token | `--api-token abc123` |

### 使用示例

```bash
# 基础用法（自动上色天蓝色）
python3 Scripts/add_vehicle.py https://sketchfab.com/3d-models/ferrari-488-def456

# 指定颜色预设
python3 Scripts/add_vehicle.py https://sketchfab.com/3d-models/lambo-012 --color red

# 指定 RGB 颜色值（0.0-1.0）
python3 Scripts/add_vehicle.py https://sketchfab.com/3d-models/mclaren-p1-xyz --color 0.9 0.6 0.0

# 指定自定义名称
python3 Scripts/add_vehicle.py https://sketchfab.com/3d-models/gtr-r35-abc --name "Nissan GT-R R35"

# 完整示例
python3 Scripts/add_vehicle.py https://sketchfab.com/3d-models/918-spyder-def \
  --color champagne \
  --name "Porsche 918 Spyder" \
  --api-token YOUR_TOKEN
```

---

## 🎨 车漆颜色预设

### 内置颜色

| 预设名 | 颜色 | RGB 值 |
|--------|------|--------|
| `blue` | 经典蓝 | (0.0, 0.2, 0.8) |
| `red` | 法拉利红 | (0.8, 0.05, 0.05) |
| `green` | 英国绿 | (0.05, 0.6, 0.15) |
| `black` | 纯黑 | (0.02, 0.02, 0.02) |
| `white` | 珍珠白 | (0.9, 0.9, 0.9) |
| `silver` | 银色 | (0.7, 0.7, 0.75) |
| `gold` | 金色 | (0.85, 0.7, 0.2) |
| `orange` | 橙色 | (0.9, 0.4, 0.05) |
| `purple` | 紫色 | (0.5, 0.1, 0.7) |
| `sky_blue` | 天蓝色 | (0.4, 0.7, 1.0) |
| `cyan` | 青色 | (0.0, 0.8, 0.8) |
| `pink` | 粉色 | (0.9, 0.3, 0.5) |
| `matte_black` | 哑光黑 | (0.02, 0.02, 0.02) |
| `champagne` | 香槟金 | (0.85, 0.75, 0.6) |

### 自定义颜色

使用 RGB 值（0.0-1.0）：

```bash
# 保时捷 GT 银
--color 0.75 0.75 0.8

# 兰博基尼黄
--color 1.0 0.85 0.0

# 宝马奥斯汀黄
--color 1.0 0.8 0.0

# 奥迪纳多灰
--color 0.3 0.3 0.35
```

---

## 📋 导入流程

当你运行命令时，系统会自动执行以下步骤：

```
┌─────────────────────────────────────────────────────────────┐
│  1. 分析模型                                                 │
│     • 获取模型信息（面数、材质、纹理）                          │
│     • 检查是否可下载                                          │
│     • 评估模型质量                                            │
├─────────────────────────────────────────────────────────────┤
│  2. 下载模型                                                 │
│     • 从 Sketchfab 下载 GLB 文件                             │
│     • 保存到 Content/Vehicles/[VehicleID]/                   │
├─────────────────────────────────────────────────────────────┤
│  3. 处理模型                                                 │
│     • 应用车漆颜色材质                                        │
│     • 设置玻璃、车灯、轮胎材质                                 │
│     • 保留原始纹理（如果有）                                   │
├─────────────────────────────────────────────────────────────┤
│  4. 生成配置                                                 │
│     • 物理参数 JSON                                          │
│     • C++ 车辆类文件                                         │
│     • 电机音效                                               │
│     • 更新 VehicleConfig.json                                │
│     • 更新 AudioConfig.json                                  │
│     • 添加 GameplayTag                                       │
├─────────────────────────────────────────────────────────────┤
│  5. 完成！                                                   │
│     • 车辆可在 UE5 编辑器中使用                                │
│     • 构建项目后即可游玩                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎮 导入后使用

### 在 UE5 编辑器中

1. 打开 UE5 编辑器
2. 运行一键设置：
   ```python
   exec(open('Scripts/Editor/run_full_setup.py').read())
   ```
3. 在车辆选择界面找到你的自定义车辆

### 调整车辆参数

导入后可以手动调整物理参数：

```bash
# 编辑物理配置
vim Content/Vehicles/[VehicleID]_Physics.json
```

可调整参数：
- `mass.total_kg` — 车辆重量
- `engine.motor_details.total_power_kw` — 功率
- `performance_targets.top_speed_kph` — 极速
- `performance_targets.zero_to_100_kph_sec` — 0-100 加速

---

## ⚠️ 注意事项

### 模型质量建议

| 指标 | 推荐范围 | 说明 |
|------|----------|------|
| **面数** | 50,000 - 1,500,000 | 太少细节不够，太多影响性能 |
| **材质** | 5-50 个 | 太少外观单一，太多增加复杂度 |
| **纹理** | 0-20 个 | 无纹理会自动上色，有纹理保留原貌 |
| **格式** | glTF/GLB | 系统自动处理此格式 |

### 常见问题

**Q: 模型没有纹理怎么办？**
A: 系统会自动应用你选择的车漆颜色，效果很好！

**Q: 下载失败怎么办？**
A: 检查 API Token 是否正确，模型是否可下载

**Q: 如何删除已添加的车辆？**
A: 删除以下文件：
- `Content/Vehicles/[VehicleID]/` 目录
- `Content/Vehicles/[VehicleID]_Physics.json`
- `Source/.../Vehicles/A[VehicleID].h`
- `Source/.../Vehicles/A[VehicleID].cpp`
- 从配置文件中移除相关条目

---

## 🌟 推荐 Sketchfab 车辆模型

### 超跑

| 模型 | 链接 | 特点 |
|------|------|------|
| Porsche 911 GT3 RS | [链接](https://sketchfab.com/3d-models/porsche-911-gt3-rs) | 经典后置引擎 |
| Ferrari 488 GTB | [链接](https://sketchfab.com/3d-models/ferrari-488) | 意大利激情 |
| Lamborghini Huracán | [链接](https://sketchfab.com/3d-models/lamborghini-huracan) | 蛮牛风格 |
| McLaren P1 | [链接](https://sketchfab.com/3d-models/mclaren-p1) | 混动超跑 |

### 电动车型

| 模型 | 链接 | 特点 |
|------|------|------|
| Tesla Model S Plaid | [链接](https://sketchfab.com/3d-models/tesla-model-s) | 电动轿车标杆 |
| Porsche Taycan | [链接](https://sketchfab.com/3d-models/porsche-taycan) | 德系电动 |
| BMW iX | [链接](https://sketchfab.com/3d-models/bmw-ix) | 电动 SUV |

### 经典车型

| 模型 | 链接 | 特点 |
|------|------|------|
| Toyota AE86 | [链接](https://sketchfab.com/3d-models/toyota-ae86) | 头文字 D |
| Nissan Skyline GT-R | [链接](https://sketchfab.com/3d-models/nissan-skyline) | 东瀛战神 |
| Mazda RX-7 | [链接](https://sketchfab.com/3d-models/mazda-rx7) | 转子传奇 |

---

## 🔧 技术细节

### 文件结构

导入后会创建以下文件：

```
NomiRacingPlus/
├── Content/
│   ├── Vehicles/
│   │   └── [VehicleID]/
│   │       └── source.glb              # 车辆模型
│   ├── Vehicles/
│   │   └── [VehicleID]_Physics.json    # 物理配置
│   └── Audio/
│       └── Motor/
│           └── [VehicleID]_motor_loop.wav  # 电机音效
└── Source/
    └── NomiRacingPlus/
        └── Vehicles/
            ├── A[VehicleID].h          # 车辆类头文件
            └── A[VehicleID].cpp        # 车辆类实现
```

### 配置文件更新

- `VehicleConfig.json` — 添加车辆性能参数
- `AudioConfig.json` — 添加电机音效配置
- `DefaultGameplayTags.ini` — 添加 GameplayTag

---

## 📜 许可证说明

使用 Sketchfab 模型时请注意许可证：

| 许可证 | 说明 | 可用于 |
|--------|------|--------|
| CC BY | 署名 | ✅ 个人和商业 |
| CC BY-SA | 署名-相同方式共享 | ✅ 个人和商业 |
| CC BY-NC | 署名-非商业性 | ✅ 仅个人 |
| CC BY-ND | 署名-禁止演绎 | ⚠️ 需注意修改限制 |

**建议：** 选择 CC BY 或 CC BY-SA 许可证的模型

---

## 🎯 开源贡献

欢迎社区贡献！

1. Fork 本项目
2. 添加你喜欢的车辆模型
3. 提交 Pull Request
4. 分享给社区！

---

**让每个人都能驾驶梦想之车！** 🏎️✨

---

*更多问题请查看 [GitHub Issues](https://github.com/twmissingu/nomi-racing-plus/issues)*
