// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TirePhysicsModel.h"
#include "NIOVehicleMovementComponent.generated.h"

/**
 * Tire effects state exposed to visual/audio systems
 * Contains aggregated tire data for effects like smoke, screech, sparks
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireEffectsState
{
	GENERATED_BODY()

	// Is any tire slipping beyond threshold?
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	bool bAnyTireSlipping = false;

	// Maximum slip ratio across all tires (0-1+)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float MaxSlipRatio = 0.0f;

	// Maximum slip angle across all tires (degrees)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float MaxSlipAngleDeg = 0.0f;

	// Average tire surface temperature (Celsius)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float AverageTireTemperature = 25.0f;

	// Per-wheel ground contact status (FL, FR, RL, RR) - fixed size for performance
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<bool> WheelGrounded;

	// Per-wheel slip ratio for granular effects - fixed size for performance
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelSlipRatios;

	// Per-wheel slip angle for granular effects - fixed size for performance
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelSlipAngles;

	// Per-wheel tire temperature for heat haze effects - fixed size for performance
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelTemperatures;

	// Maximum number of wheels supported (fixed for memory optimization)
	static constexpr int32 MaxWheelCount = 4;

	// Resize all arrays to MaxWheelCount once, avoid repeated allocations
	void ResizeForWheelCount(int32 WheelCount)
	{
		if (WheelGrounded.Num() != WheelCount) WheelGrounded.SetNum(WheelCount);
		if (WheelSlipRatios.Num() != WheelCount) WheelSlipRatios.SetNum(WheelCount);
		if (WheelSlipAngles.Num() != WheelCount) WheelSlipAngles.SetNum(WheelCount);
		if (WheelTemperatures.Num() != WheelCount) WheelTemperatures.SetNum(WheelCount);
	}
};

/**
 * Per-wheel force data for debug/log output
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FWheelForceDebugEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	bool bGrounded = false;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float SlipRatio = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float SlipAngleDeg = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float WheelLoad = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float PacejkaLongForce = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float PacejkaLatForce = 0.0f;
};

/**
 * Per-frame force debug snapshot for diagnosing Chaos vs Pacejka force superposition
 * Populated when bEnableTireForceDebugLog is true
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FForceDebugSnapshot
{
	GENERATED_BODY()

	// Vehicle forward speed (cm/s) at this snapshot
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float ForwardSpeedCmS = 0.0f;

	// Vehicle forward acceleration (cm/s^2) inferred from velocity delta
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float ForwardAccelCmS2 = 0.0f;

	// Sum of Pacejka longitudinal forces across all grounded wheels (UE force units)
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float TotalPacejkaLongForce = 0.0f;

	// Sum of Pacejka lateral forces across all grounded wheels (UE force units)
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float TotalPacejkaLatForce = 0.0f;

	// Per-wheel debug entries (indices 0-3: FL, FR, RL, RR)
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<FWheelForceDebugEntry> WheelEntries;

	// Resize arrays to match wheel count (avoids reallocation spam)
	void ResizeForWheelCount(int32 Count)
	{
		if (WheelEntries.Num() != Count) WheelEntries.SetNum(Count);
	}
};

/**
 * Diagnostic scheme selection for isolating Pacejka vs Chaos tire forces
 * Used during Iter 0 to determine the optimal approach
 */
UENUM(BlueprintType)
enum class ENIOTireForceScheme : uint8
{
	// Scheme A: Keep Chaos running, set tire friction to near 0
	// Pacejka forces dominate, but Chaos solver still runs (CPU overhead)
	FrictionHack UMETA(DisplayName = "A: Friction Hack"),

	// Scheme B: Skip Chaos internal tire solver entirely
	// bUseInternalVehiclePhysics = false → cleanest isolation
	NoInternalPhysics UMETA(DisplayName = "B: No Internal Physics"),

	// Scheme C: Both — bUseInternalVehiclePhysics=false + friction~0
	// Double safety, slightly more CPU overhead from unused friction system
	Both UMETA(DisplayName = "C: Both"),

	// Default/current behavior: both active (for baseline comparison)
	Default UMETA(DisplayName = "Default (Current)")
};

/**
 * NIO Electric Vehicle Movement Component
 * Extends ChaosVehicleMovementComponent with electric vehicle physics:
 * - Instant torque from 0 RPM
 * - Single-speed transmission (no gear shifts)
 * - Regenerative braking
 * - Lower center of gravity (battery pack)
 * - Integrated Pacejka tire physics model
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UNIOVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()

public:
	UNIOVehicleMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginDestroy() override;

	// Electric vehicle specific functions

	// Get motor torque at current RPM (electric: high at 0 RPM, decays at high RPM)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetElectricMotorTorque(float CurrentRPM) const;

	// Get effective motor torque with battery level applied (does not mutate PeakMotorTorque)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetEffectiveMotorTorque() const;

	// Get regenerative braking torque
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetRegenerativeBrakingTorque() const;

	// Is motor at overheat limit?
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	bool IsMotorOverheating() const { return MotorTemperature > MotorOverheatThreshold; }

	// Get motor temperature (0-100)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetMotorTemperature() const { return MotorTemperature; }

	// Get battery level (0-100%)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetBatteryLevel() const { return BatteryLevel; }

	// Apply downforce based on speed (EP9: 2000kg at max speed)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Aero")
	void ApplyAerodynamicDownforce(float DeltaTime);

	// Configure for specific NIO vehicle type
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Config")
	void ConfigureForNIOVehicle(ENIOVehicleType VehicleType);

	// Get the integrated tire physics model
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	UTirePhysicsModel* GetTirePhysicsModel() const { return TireModel; }

	// Get tire state for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	const FTireState& GetTireState(int32 WheelIndex) const;

	// Is any tire slipping significantly?
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	bool IsAnyTireSlipping(float Threshold = 0.15f) const;

	// Get tire effects state for visual/audio systems
	// Returns cached reference (avoids TArray copy every frame)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	const FTireEffectsState& GetTireEffectsState() const;

protected:
	// Electric motor parameters

	// Peak motor torque in Nm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float PeakMotorTorque = 850.0f;

	// RPM at which torque starts to decay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float TorqueDecayRPM = 3000.0f;

	// Maximum motor RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float MaxMotorRPM = 12000.0f;

	// Motor efficiency (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float MotorEfficiency = 0.95f;

	// Regenerative braking strength (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float RegenBrakingStrength = 0.3f;

	// Battery parameters

	// Current battery level (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryLevel = 100.0f;

	// Battery discharge rate (%/second at full throttle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryDischargeRate = 0.05f;

	// Battery regen rate (%/second at full regen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryRegenRate = 0.02f;

	// Motor thermal parameters

	// Current motor temperature (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorTemperature = 20.0f;

	// Temperature rise rate (degrees/second at full power)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorHeatRate = 5.0f;

	// Temperature cooling rate (degrees/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorCoolRate = 2.0f;

	// Overheat threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorOverheatThreshold = 80.0f;

	// Aerodynamic parameters

	// Custom downforce coefficient (overrides parent class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float NIODownforceCoefficient = 0.0f;

	// Custom drag coefficient (overrides parent class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float NIODragCoefficient = 0.35f;

	// Frontal area in m^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float FrontalArea = 2.2f;

private:
	// Calculate torque curve for electric motor
	float CalculateElectricTorqueCurve(float RPM) const;

	// Apply regenerative braking
	void ApplyRegenerativeBraking(float DeltaTime);

	// Update motor temperature
	void UpdateMotorTemperature(float DeltaTime, float ThrottleInput);

	// Update battery level
	void UpdateBatteryLevel(float DeltaTime, float ThrottleInput, float RegenInput);

	// Initialize tire physics model
	void InitializeTireModel();

	// Apply tire forces to the vehicle simulation
	void ApplyTireForces(float DeltaTime);

	// Tire physics model component (auto-created)
	UPROPERTY()
	TObjectPtr<UTirePhysicsModel> TireModel;

	// Cached tire effects state (avoid per-frame TArray copies)
	mutable FTireEffectsState CachedEffectsState;

	// Debug frame counter for throttled force logging
	int32 ForceDebugFrameCounter = 0;

	// Previous frame velocity for debug acceleration calculation
	FVector ForceDebugPreviousVelocity = FVector::ZeroVector;

	// Previous frame velocity for capture acceleration calculation (separate from debug logging)
	FVector CapturePreviousVelocity = FVector::ZeroVector;

	// CSV file handle for force data capture (non-UPROPERTY, runtime only)
	FArchive* ForceCaptureFile = nullptr;

	// Frame counter within the current capture session
	int32 CaptureFrameIndex = 0;

protected:
	// === Debug / Diagnostic ===

	// Enable per-frame tire force debug logging (use with log level Verbose)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Debug")
	bool bEnableTireForceDebugLog = false;

	// Log force data every N frames (0 = every frame when enabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Debug", meta = (ClampMin = "0", ClampMax = "600"))
	int32 ForceDebugLogInterval = 60;

	// Force debug snapshot — accumulated per tick for external readout
	UPROPERTY(BlueprintReadOnly, Category = "NIO Vehicle|Debug")
	FForceDebugSnapshot ForceDebugSnapshot;

	// Enable on-screen debug force vector visualization at each wheel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Debug")
	bool bDrawDebugForceVectors = false;

	// Log current tire forces to Verbose log and populate ForceDebugSnapshot
	void LogTireForces(float DeltaTime);

	// Write one frame of captured force data to the open CSV file
	void WriteForceCaptureFrame(float DeltaTime);

	// Draw debug force vectors at each wheel position (called from TickComponent)
	void DrawDebugForceVectors();

	// === Tire Force Independence (Iter 0) ===

	// When true, Pacejka forces replace (not overlay) Chaos default tire forces.
	// This is the production flag that controls the final behavior after diagnosis.
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	bool bOverrideChaosTireWithPacejka = true;

	// Force-disable Chaos internal vehicle physics solver.
	// When true, Chaos skips its built-in tire integration;
	// only forces from ApplyTireForces() affect the vehicle.
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	bool bChaosUseInternalVehiclePhysics = false;

	// Fallback: Chaos tire friction coefficient when Pacejka is overriding.
	// Set to near 0 so even if the internal solver sneaks through, its contribution is minimal.
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float ChaosTireFrictionOverride = 0.01f;

	// === Objective Metrics (Iter 0 Tuning) ===

	// Run 0-100 km/h acceleration test
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Metrics")
	void RunAccelerationTest();

	// Run 100-0 km/h braking test
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Metrics")
	void RunBrakingTest();

	// Run all metrics in sequence (accel → braking) and log results
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Metrics")
	void RunAllMetrics();

	// Get last recorded metrics as a formatted string
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Metrics")
	FString GetLastMetricsReport() const;

private:
	// Metrics test state
	struct FMetricsTestState
	{
		bool bActive = false;
		FString TestName;
		float StartTime = 0.0f;
		float StartSpeedKmh = 0.0f;
		float StartDistanceCm = 0.0f;
		bool bCompleted = false;
		bool bAutoChain = false; // When true, auto-chain to next test on completion
		float ResultValue = 0.0f;
		FString ResultUnit;
	};
	FMetricsTestState MetricsState;

	// Tick handler for active metrics test
	void UpdateMetricsTest(float DeltaTime);

public:
	// === Diagnostic Scheme Selection ===

	// Select which tire force scheme to use for diagnosis (Iter 0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Diagnostic")
	ENIOTireForceScheme TireForceScheme = ENIOTireForceScheme::Default;

	// Apply the selected tire force scheme at runtime.
	// Call this after changing TireForceScheme to apply immediately.
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Diagnostic")
	void ApplyTireForceScheme();

	// === Data Capture (Iter 0 Diagnosis) ===

	// Enable CSV force data capture to file
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Diagnostic")
	bool bEnableForceDataCapture = false;

	// Current scenario label for data capture markers
	UPROPERTY(BlueprintReadOnly, Category = "NIO Vehicle|Diagnostic")
	FString CaptureScenarioLabel;

	// Start force data capture with a scenario label (e.g. "StraightAccel", "MediumCorner", "Hairpin")
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Diagnostic")
	void StartForceCapture(const FString& ScenarioLabel);

	// Stop force data capture and close the CSV file
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Diagnostic")
	void StopForceCapture();

public:
	// Front tire preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	FTireModelPreset FrontTirePreset;

	// Rear tire preset (typically wider, different compound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	FTireModelPreset RearTirePreset;
};
