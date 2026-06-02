# NIO Racing Plus - API Reference

> Generated from source code analysis
> Last updated: 2026-06-01

---

## Core Systems

### VehicleStateManager

Manages vehicle state data including speed, input, physics, and battery level.

```cpp
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class UVehicleStateManager : public UActorComponent
```

#### Key Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `GetVehicleState()` | `const FVehicleState&` | Get current vehicle state |
| `GetVehicleType()` | `ENIOVehicleType` | Get vehicle type |
| `SetVehicleType(type)` | `void` | Set vehicle type |
| `GetPerformanceConfig()` | `const FNIOPerformanceConfig&` | Get performance config |
| `IsNIOVehicle()` | `bool` | Check if NIO vehicle |
| `GetVehicleDisplayName()` | `FString` | Get vehicle display name |

#### FVehicleState Structure

```cpp
USTRUCT(BlueprintType)
struct FVehicleState
{
    float SpeedKmh;           // Current speed (km/h)
    float ThrottleInput;      // Throttle input (0-1)
    float BrakeInput;         // Brake input (0-1)
    float SteeringInput;      // Steering input (-1 to 1)
    float RPM;                // Motor RPM
    int32 Gear;               // Current gear (fixed at 1 for EV)
    bool bIsDrifting;         // Is drifting?
    float SlipAngle;          // Slip angle (degrees)
    bool bIsGrounded;         // Is on ground?
    FVector Position;         // World position
    FRotator Rotation;        // World rotation
    FVector Velocity;         // Velocity vector
    float BatteryPercent;     // Battery level (0-100)
    bool bIsMotorActive;      // Is motor active?
};
```

---

### NIOVehicleMovementComponent

Extends ChaosVehicleMovementComponent for NIO electric vehicles.

```cpp
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class UNIOVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
```

#### Key Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `GetElectricMotorTorque(rpm)` | `float` | Get motor torque at RPM |
| `GetRegenerativeBrakingTorque()` | `float` | Get regen braking torque |
| `IsMotorOverheating()` | `bool` | Is motor overheating? |
| `GetMotorTemperature()` | `float` | Get motor temperature |
| `GetBatteryLevel()` | `float` | Get battery level |
| `ApplyAerodynamicDownforce(dt)` | `void` | Apply downforce |
| `ConfigureForNIOVehicle(type)` | `void` | Configure for vehicle type |

#### Electric Motor Physics

```cpp
// Torque curve: 0 RPM = peak torque, decays at high RPM
float CalculateElectricTorqueCurve(float RPM) const
{
    if (RPM <= TorqueDecayRPM) return PeakMotorTorque;
    if (RPM >= MaxMotorRPM) return 0.0f;
    float DecayFactor = 1.0f - ((RPM - TorqueDecayRPM) / (MaxMotorRPM - TorqueDecayRPM));
    return PeakMotorTorque * DecayFactor;
}
```

---

### RaceManager

Controls race flow, timing, and positions.

```cpp
UCLASS(Blueprintable)
class ARaceManager : public AActor
```

#### Key Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `StartRace(config)` | `void` | Start a new race |
| `PauseRace()` | `void` | Pause the race |
| `ResumeRace()` | `void` | Resume the race |
| `EndRace()` | `void` | End the race |
| `RegisterRacer(pawn, name, isPlayer)` | `void` | Register a racer |
| `RacerPassCheckpoint(pawn, index)` | `void` | Racer passes checkpoint |
| `GetRaceState()` | `ERaceState` | Get current state |
| `GetPlayerPosition()` | `int32` | Get player position |
| `GetRaceTimer()` | `float` | Get race timer |

#### ERaceState Enum

```cpp
enum class ERaceState : uint8
{
    Idle,       // Waiting to start
    Loading,    // Loading track
    Countdown,  // 5-4-3-2-1-GO
    Racing,     // Race in progress
    Paused,     // Race paused
    Finished,   // Race finished
    PostRace    // Post-race screen
};
```

---

### CommentaryEngine

Manages NOMI's comment system with 500+ pre-generated comments.

```cpp
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class UCommentaryEngine : public UActorComponent
```

#### Key Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `LoadCommentPool(jsonPath)` | `bool` | Load comments from JSON |
| `RequestComment(context)` | `bool` | Request a comment |
| `GetNextComment(outComment)` | `bool` | Get next comment to display |
| `IsCommentPlaying()` | `bool` | Is comment playing? |
| `GetCurrentCommentText()` | `FString` | Get current comment text |
| `GetCurrentEmotion()` | `ENOMIEmotion` | Get current emotion |
| `SetCommentFrequency(freq)` | `void` | Set comment frequency |

#### ENOMIEmotion Enum

```cpp
enum class ENOMIEmotion : uint8
{
    Idle,        // Neutral waiting
    Happy,       // Positive feedback
    Excited,     // High energy
    Nervous,     // Tense moments
    Surprised,   // Unexpected events
    Celebrating, // Victories
    Concerned,   // Problems
    Confused,    // Unclear situations
    Tired        // Long races
};
```

---

### AICarController

Controls AI opponent vehicles with difficulty scaling.

```cpp
UCLASS(Blueprintable)
class AAICarController : public AAIController
```

#### Key Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `SetDifficulty(difficulty)` | `void` | Set AI difficulty |
| `GetDifficulty()` | `EAIDifficulty` | Get current difficulty |
| `SetWaypoints(waypoints)` | `void` | Set race waypoints |
| `StartRacing()` | `void` | Start racing |
| `StopRacing()` | `void` | Stop racing |
| `GetAIState()` | `EAIState` | Get AI state |
| `SetTargetSpeedMultiplier(mult)` | `void` | Set speed multiplier |

#### EAIDifficulty Enum

```cpp
enum class EAIDifficulty : uint8
{
    Easy,    // 85% speed, relaxed
    Normal,  // 95% speed, balanced
    Hard     // 100% speed, aggressive
};
```

---

## Data Structures

### FNIOPerformanceConfig

Vehicle performance configuration.

```cpp
USTRUCT(BlueprintType)
struct FNIOPerformanceConfig
{
    float MassKg;              // Vehicle mass (kg)
    float PowerKw;             // Motor power (kW)
    float TorqueNm;            // Motor torque (Nm)
    FString DriveType;         // "AWD_quad_motor", "AWD_dual_motor"
    float TopSpeedKph;         // Top speed (km/h)
    float Acceleration0100;    // 0-100 km/h time (seconds)
    float DownforceMaxKg;      // Max downforce (kg)
    float WheelbaseMm;         // Wheelbase (mm)
    FString BodyType;          // "hypercar", "sedan", "suv"
    bool bIsElectric;          // Is electric vehicle?
    float RegenBrakingStrength; // Regen strength (0-1)
};
```

### FRaceConfig

Race configuration.

```cpp
USTRUCT(BlueprintType)
struct FRaceConfig
{
    int32 NumLaps;             // Number of laps
    int32 MaxAIOpponents;      // Max AI opponents
    FString TrackName;         // Track name
    FString RaceMode;          // "StreetGT", "NIO", "Baja"
    float CountdownDuration;   // Countdown time (seconds)
    bool bAllowCollisions;     // Allow collisions?
    float CollisionPenalty;    // Collision penalty (seconds)
};
```

### FRacerData

Racer data for tracking.

```cpp
USTRUCT(BlueprintType)
struct FRacerData
{
    int32 RacerID;             // Racer ID (0=player)
    FString DisplayName;       // Display name
    bool bIsPlayer;            // Is player?
    int32 CurrentLap;          // Current lap
    int32 CurrentCheckpoint;   // Current checkpoint
    int32 Position;            // Race position
    TArray<float> LapTimes;    // Lap times
    float BestLapTime;         // Best lap time
    float TotalRaceTime;       // Total race time
    bool bFinished;            // Is finished?
    float TrackProgress;       // Track progress (0-1)
};
```

---

## Events

### Race Events

```cpp
enum class ERaceEvent : uint8
{
    Overtake,           // Player overtakes
    Overtaken,          // Player is overtaken
    DriftStart,         // Drift begins
    DriftEnd,           // Drift ends
    LapComplete,        // Lap completed
    FastestLap,         // New fastest lap
    HighSpeed,          // High speed reached
    Collision,          // Collision occurred
    FirstPlace,         // Takes first place
    LastPlace,          // Falls to last
    RaceStart,          // Race begins
    RaceFinish,         // Race ends
    CountdownStart,     // Countdown starts
    CheckpointPassed,   // Checkpoint passed
    VehicleStuck,       // Vehicle stuck
    VehicleFlipped      // Vehicle flipped
};
```

### Delegate Signatures

```cpp
// Race event delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnRaceEvent,
    ERaceEvent, Event,
    const FRacerData&, RacerData
);

// Achievement delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnAchievementUnlocked,
    const FAchievementData&, Achievement
);

// Camera state delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnCameraModeChanged,
    ECameraMode, NewMode
);
```

---

## Constants

### Vehicle Constants

```cpp
namespace VehicleConstants
{
    constexpr float CmPerSecToKmPerHour = 0.036f;
    constexpr float DriftAngleThreshold = 15.0f;
    constexpr float MinSpeedForSlipAngle = 5.0f;
    constexpr float BatteryDrainRate = 0.01f;
    constexpr float BatteryRegenRate = 0.005f;
}
```

### Race Constants

```cpp
namespace RaceConstants
{
    constexpr int32 MaxLaps = 20;
    constexpr int32 MaxAIOpponents = 7;
    constexpr float CountdownDuration = 5.0f;
    constexpr int32 CheckpointsPerLap = 10;
}
```

### NOMI Constants

```cpp
namespace NOMIConstants
{
    constexpr float CommentCooldown = 3.0f;
    constexpr int32 MaxQueueSize = 2;
    constexpr int32 RecentCommentsMax = 10;
    constexpr float DefaultFrequency = 0.7f;
}
```

---

## Usage Examples

### Starting a Race

```cpp
// Get race manager
ARaceManager* RaceManager = GetWorld()->SpawnActor<ARaceManager>();

// Configure race
FRaceConfig Config;
Config.NumLaps = 3;
Config.TrackName = TEXT("NIO City Circuit");
Config.RaceMode = TEXT("NIO");

// Register racers
RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);
RaceManager->RegisterRacer(AIVehicle, TEXT("AI 1"), false);

// Start race
RaceManager->StartRace(Config);
```

### Getting Vehicle State

```cpp
// Get vehicle state manager
UVehicleStateManager* StateManager = Vehicle->FindComponentByClass<UVehicleStateManager>();

// Get current state
const FVehicleState& State = StateManager->GetVehicleState();

// Use state
float Speed = State.SpeedKmh;
bool IsDrifting = State.bIsDrifting;
float Battery = State.BatteryPercent;
```

### Requesting NOMI Comment

```cpp
// Get commentary engine
UCommentaryEngine* Engine = NOMIController->GetCommentaryEngine();

// Create context
FCommentContext Context;
Context.Event = ERaceEvent::Overtake;
Context.PlayerName = TEXT("Player");
Context.Position = 1;
Context.bIsNIOVehicle = true;

// Request comment
if (Engine->RequestComment(Context))
{
    // Comment will be displayed
    FString Text = Engine->GetCurrentCommentText();
    ENOMIEmotion Emotion = Engine->GetCurrentEmotion();
}
```

---

*This API reference is auto-generated from source code comments.*
*For detailed implementation, see the header files in Source/NomiRacingPlus/*
