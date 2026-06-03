// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TirePhysicsModel.generated.h"

// Forward declarations
class UChaosWheeledVehicleMovementComponent;

/**
 * Pacejka Magic Formula coefficients for tire force calculation
 * F = D * sin(C * arctan(B*x - E*(B*x - arctan(B*x))))
 *
 * B = Stiffness Factor - affects the slope at the origin
 * C = Shape Factor - affects the shape of the curve (1.0-2.0 typical)
 * D = Peak Factor - maximum force value
 * E = Curvature Factor - affects the curve after the peak
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPacejkaCoefficients
{
	GENERATED_BODY()

	// Stiffness Factor (higher = more responsive, typical range: 8-20)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka", meta = (ClampMin = "0.1", ClampMax = "50.0"))
	float B = 10.0f;

	// Shape Factor (typical range: 1.0-2.0, 1.65 for lateral, 1.65 for longitudinal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float C = 1.65f;

	// Peak Factor - maximum force (in Newtons for force, or normalized 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka", meta = (ClampMin = "0.0"))
	float D = 1.0f;

	// Curvature Factor (typical range: -2.0 to 1.0, negative values common)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka", meta = (ClampMin = "-5.0", ClampMax = "2.0"))
	float E = -0.1f;

	// Validate coefficients are within reasonable ranges
	bool IsValid() const;
};

/**
 * Combined Pacejka coefficients for a tire
 * Separate sets for longitudinal (brake/accel) and lateral (cornering) forces
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTirePacejkaSet
{
	GENERATED_BODY()

	// Longitudinal force coefficients (traction/braking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka|Longitudinal")
	FPacejkaCoefficients Longitudinal;

	// Lateral force coefficients (cornering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka|Lateral")
	FPacejkaCoefficients Lateral;

	// Combined slip weighting factor (0-1, how much combined slip reduces forces)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacejka|Combined", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CombinedSlipFactor = 0.85f;
};

/**
 * Surface type for grip calculation
 */
UENUM(BlueprintType)
enum class ETireSurfaceType : uint8
{
	Tarmac      UMETA(DisplayName = "Tarmac (Dry)"),
	TarmacWet   UMETA(DisplayName = "Tarmac (Wet)"),
	Gravel      UMETA(DisplayName = "Gravel"),
	Sand        UMETA(DisplayName = "Sand"),
	Grass       UMETA(DisplayName = "Grass"),
	Concrete    UMETA(DisplayName = "Concrete"),
	Ice         UMETA(DisplayName = "Ice"),
	Custom      UMETA(DisplayName = "Custom")
};

/**
 * Result of a ground contact raycast for a single wheel
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireGroundContact
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Ground")
	bool bHitGround = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ground")
	float DistanceToGround = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Ground")
	ETireSurfaceType SurfaceType = ETireSurfaceType::Tarmac;

	UPROPERTY(BlueprintReadOnly, Category = "Ground")
	FVector HitNormal = FVector::UpVector;
};

/**
 * Surface-specific grip and drag parameters
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireSurfaceParams
{
	GENERATED_BODY()

	// Grip multiplier relative to base (1.0 = nominal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float GripMultiplier = 1.0f;

	// Rolling resistance multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float RollingResistanceMultiplier = 1.0f;

	// How much slip generates heat (higher = more heat per slip unit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float HeatGenerationMultiplier = 1.0f;

	// Surface drag (affects lateral slide behavior)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SurfaceDrag = 0.0f;
};

/**
 * Tire thermal state
 * Temperature affects grip: cold tires have less grip, optimal temp gives max grip,
 * overheated tires lose grip
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireThermalState
{
	GENERATED_BODY()

	// Current tire core temperature (Celsius)
	UPROPERTY(BlueprintReadOnly, Category = "Thermal")
	float CoreTemperature = 25.0f;

	// Current tire surface temperature (Celsius) - changes faster than core
	UPROPERTY(BlueprintReadOnly, Category = "Thermal")
	float SurfaceTemperature = 25.0f;

	// Optimal operating temperature for maximum grip (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float OptimalTemperature = 85.0f;

	// Temperature below which grip drops significantly (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float ColdThreshold = 40.0f;

	// Temperature above which grip drops (overheating) (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float OverheatThreshold = 110.0f;

	// Maximum safe temperature before damage (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float MaxTemperature = 150.0f;

	// Current grip multiplier from thermal state (0.5 - 1.1)
	UPROPERTY(BlueprintReadOnly, Category = "Thermal")
	float ThermalGripMultiplier = 1.0f;

	// Tire wear factor (0 = destroyed, 1 = new)
	UPROPERTY(BlueprintReadOnly, Category = "Thermal")
	float WearFactor = 1.0f;
};

/**
 * Per-wheel tire state for real-time simulation
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireState
{
	GENERATED_BODY()

	// Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	UPROPERTY(BlueprintReadOnly, Category = "Tire")
	int32 WheelIndex = 0;

	// Current slip ratio (-1 to 1, negative = locked/overspin)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Slip")
	float SlipRatio = 0.0f;

	// Current slip angle (degrees, negative = turning left)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Slip")
	float SlipAngleDeg = 0.0f;

	// Longitudinal force from Pacejka (Newtons)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Force")
	float LongitudinalForce = 0.0f;

	// Lateral force from Pacejka (Newtons)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Force")
	float LateralForce = 0.0f;

	// Current wheel angular velocity (rad/s)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Wheel")
	float WheelAngularVelocity = 0.0f;

	// Wheel load (normal force in Newtons)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Wheel")
	float WheelLoad = 0.0f;

	// Current surface type
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Surface")
	ETireSurfaceType SurfaceType = ETireSurfaceType::Tarmac;

	// Thermal state for this tire
	UPROPERTY(BlueprintReadOnly, Category = "Tire|Thermal")
	FTireThermalState Thermal;

	// Is this wheel on the ground?
	UPROPERTY(BlueprintReadOnly, Category = "Tire")
	bool bIsGrounded = false;
};

/**
 * Tire model preset configurations for different vehicle types
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireModelPreset
{
	GENERATED_BODY()

	// Preset name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FString PresetName = TEXT("Sport");

	// Pacejka coefficients
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FTirePacejkaSet Pacejka;

	// Tire width in mm (affects lateral grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Dimensions", meta = (ClampMin = "100", ClampMax = "400"))
	float TireWidthMm = 265.0f;

	// Tire aspect ratio (sidewall height as % of width)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Dimensions", meta = (ClampMin = "20", ClampMax = "80"))
	float AspectRatio = 35.0f;

	// Rim diameter in inches
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Dimensions", meta = (ClampMin = "14", ClampMax = "24"))
	float RimDiameterInches = 20.0f;

	// Tire pressure in PSI (affects contact patch and grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Pressure", meta = (ClampMin = "15.0", ClampMax = "50.0"))
	float PressurePSI = 32.0f;

	// Base friction coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Friction", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float BaseFriction = 1.1f;

	// Rolling resistance coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Resistance", meta = (ClampMin = "0.005", ClampMax = "0.05"))
	float RollingResistance = 0.015f;

	// Tire mass in kg (affects rotational inertia)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Mass", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float TireMassKg = 12.0f;

	// Thermal parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Thermal")
	float OptimalTemp = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Thermal")
	float ColdThreshold = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Thermal")
	float OverheatThreshold = 110.0f;

	// Wear rate (higher = faster wear, 0 = no wear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Wear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WearRate = 0.01f;
};

/**
 * Tire physics model component
 * Implements Pacejka Magic Formula tire model with thermal dynamics
 * Attach to vehicle to replace default Chaos tire behavior
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UTirePhysicsModel : public UActorComponent
{
	GENERATED_BODY()

public:
	UTirePhysicsModel();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === Core Calculations ===

	// Calculate Pacejka force for given slip input
	UFUNCTION(BlueprintCallable, Category = "Tire|Pacejka")
	static float CalculatePacejkaForce(const FPacejkaCoefficients& Coeffs, float SlipInput);

	// Calculate slip ratio for a wheel (longitudinal slip)
	UFUNCTION(BlueprintCallable, Category = "Tire|Slip")
	float CalculateSlipRatio(int32 WheelIndex, float WheelAngularVelocity, float VehicleForwardSpeed) const;

	// Calculate slip angle for a wheel (lateral slip in degrees)
	UFUNCTION(BlueprintCallable, Category = "Tire|Slip")
	float CalculateSlipAngle(int32 WheelIndex, const FVector& WheelVelocity, const FVector& WheelForward) const;

	// === Force Outputs ===

	// Get longitudinal tire force (Newtons) for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "Tire|Force")
	float GetLongitudinalForce(int32 WheelIndex) const;

	// Get lateral tire force (Newtons) for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "Tire|Force")
	float GetLateralForce(int32 WheelIndex) const;

	// === Thermal Model ===

	// Update tire thermal state
	UFUNCTION(BlueprintCallable, Category = "Tire|Thermal")
	void UpdateThermalState(float DeltaTime, int32 WheelIndex, float SlipMagnitude, float WheelLoad);

	// Get thermal grip multiplier for a wheel
	UFUNCTION(BlueprintCallable, Category = "Tire|Thermal")
	float GetThermalGripMultiplier(int32 WheelIndex) const;

	// === Surface Detection ===

	// Set surface type for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "Tire|Surface")
	void SetSurfaceType(int32 WheelIndex, ETireSurfaceType SurfaceType);

	// Get surface grip multiplier
	UFUNCTION(BlueprintCallable, Category = "Tire|Surface")
	float GetSurfaceGripMultiplier(int32 WheelIndex) const;

	// === Configuration ===

	// Apply a tire model preset
	UFUNCTION(BlueprintCallable, Category = "Tire|Config")
	void ApplyPreset(const FTireModelPreset& Preset);

	// Set custom Pacejka coefficients
	UFUNCTION(BlueprintCallable, Category = "Tire|Config")
	void SetPacejkaCoefficients(const FTirePacejkaSet& NewCoeffs);

	// === State Access ===

	// Get tire state for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	const FTireState& GetTireState(int32 WheelIndex) const;

	// Get all tire states
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	const TArray<FTireState>& GetAllTireStates() const { return TireStates; }

	// Is any tire currently slipping significantly?
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	bool IsAnyTireSlipping(float Threshold = 0.15f) const;

	// Perform ground contact detection for a wheel via raycast
	UFUNCTION(BlueprintCallable, Category = "Tire|Ground")
	FTireGroundContact CheckGroundContact(int32 WheelIndex, const FVector& WheelWorldPos, float TraceLength) const;

	// Get average tire temperature across all wheels
	UFUNCTION(BlueprintCallable, Category = "Tire|Thermal")
	float GetAverageTireTemperature() const;

protected:
	// Pacejka coefficients for this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Pacejka")
	FTirePacejkaSet PacejkaCoefficients;

	// Tire states for all wheels (initialized to 4)
	UPROPERTY(BlueprintReadOnly, Category = "Tire|State")
	TArray<FTireState> TireStates;

	// Surface parameters lookup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Surface")
	TMap<ETireSurfaceType, FTireSurfaceParams> SurfaceParams;

	// Tire preset (can be overridden)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config")
	FTireModelPreset CurrentPreset;

	// Number of wheels (default 4)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config", meta = (ClampMin = "2", ClampMax = "8"))
	int32 NumWheels = 4;

	// Wheel offsets from vehicle center (cm): X=forward, Y=right, Z=up
	// FL, FR, RL, RR
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config")
	TArray<FVector> WheelOffsets;

	// Suspension rest length (cm) for ground detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config", meta = (ClampMin = "5.0", ClampMax = "100.0"))
	float SuspensionRestLength = 35.0f;

	// Vehicle mass (kg) for suspension force calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float VehicleMassKg = 1800.0f;

	// Aerodynamic downforce coefficient for load calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config")
	float DownforceCoefficient = 0.0f;

	// Frontal area (m^2) for downforce calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tire|Config")
	float FrontalArea = 2.2f;

private:
	// Initialize default surface parameters
	void InitializeDefaultSurfaceParams();

	// Update slip calculations for all wheels
	void UpdateSlipCalculations(float DeltaTime);

	// Update thermal model for all wheels
	void UpdateThermalModel(float DeltaTime);

	// Update tire wear
	void UpdateTireWear(float DeltaTime, int32 WheelIndex, float SlipMagnitude);

	// Calculate grip from thermal state
	float CalculateThermalGrip(const FTireThermalState& Thermal) const;

	// Get wheel world position relative to vehicle center
	FVector GetWheelWorldPosition(int32 WheelIndex, const FTransform& VehicleTransform) const;

	// Calculate suspension force (wheel load) for a wheel
	float CalculateSuspensionForce(int32 WheelIndex, float VehicleSpeedCmS, const FVector& VehicleAcceleration, bool bIsGrounded) const;

	// Cached vehicle component reference
	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

	// Previous velocity for acceleration calculation
	FVector PreviousVelocity = FVector::ZeroVector;
};
