// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TirePhysicsModel.h"
#include "VehicleStateManager.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UNIOVehicleMovementComponent;
class ARaceManager;

/**
 * Vehicle state data structure for real-time telemetry
 * Used by HUD, NOMI system, and race manager
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOVehicleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Speed")
	float SpeedKmh = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Input")
	float ThrottleInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Input")
	float BrakeInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Input")
	float SteeringInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Engine")
	float RPM = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Engine")
	int32 Gear = 1; // Electric vehicles: fixed at 1

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Drift")
	bool bIsDrifting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Drift")
	float SlipAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Physics")
	bool bIsGrounded = true;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Physics")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Physics")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Physics")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Electric")
	float BatteryPercent = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Electric")
	bool bIsMotorActive = true;

	// Tire telemetry from Pacejka model

	// Average tire temperature across all wheels (Celsius)
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	float AverageTireTemperature = 25.0f;

	// Average tire wear factor (1.0 = new, 0.0 = destroyed)
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	float AverageTireWear = 1.0f;

	// Is any tire currently slipping beyond threshold?
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	bool bAnyTireSlipping = false;

	// Maximum slip ratio across all wheels
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	float MaxSlipRatio = 0.0f;

	// Maximum slip angle across all wheels (degrees)
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	float MaxSlipAngleDeg = 0.0f;

	// Current surface type under the vehicle (most common)
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tire")
	ETireSurfaceType CurrentSurface = ETireSurfaceType::Tarmac;
};

/**
 * Vehicle model types
 */
UENUM(BlueprintType)
enum class ENIOVehicleType : uint8
{
	EP9       UMETA(DisplayName = "NIO EP9"),
	ET7       UMETA(DisplayName = "NIO ET7"),
	ES7       UMETA(DisplayName = "NIO ES7"),
	ET5       UMETA(DisplayName = "NIO ET5"),
	SU7Ultra  UMETA(DisplayName = "Xiaomi SU7 Ultra"),
	Custom    UMETA(DisplayName = "Custom Vehicle")
};

/**
 * Vehicle display specs for UI
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FVehicleSpecs
{
	GENERATED_BODY()

	// Display name
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	FString VehicleName;

	// Vehicle description for UI display
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	FString Description;

	// Vehicle type (hypercar, sedan, suv, super_sedan)
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	FString VehicleType;

	// Max power in HP
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float MaxPower = 0.0f;

	// Max torque in Nm
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float MaxTorque = 0.0f;

	// 0-100 km/h time in seconds
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float ZeroToHundredTime = 0.0f;

	// Top speed in km/h
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float TopSpeed = 0.0f;

	// Drivetrain type description
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	FString DrivetrainType;

	// Battery capacity in kWh
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float BatteryCapacityKwh = 0.0f;

	// Range in km
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	float RangeKm = 0.0f;

	// Build from performance config
	static FVehicleSpecs FromPerformanceConfig(const FNIOPerformanceConfig& Config, const FString& DisplayName);
};

/**
 * Vehicle performance configuration data
 * Loaded from JSON or set in Blueprint
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOPerformanceConfig
{
	GENERATED_BODY()

	// Mass in kg
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MassKg = 1800.0f;

	// Motor power in kW
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float PowerKw = 400.0f;

	// Max torque in Nm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TorqueNm = 800.0f;

	// Drive type: "AWD_quad_motor", "AWD_dual_motor", "RWD", "FWD"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FString DriveType = TEXT("AWD_dual_motor");

	// Top speed in km/h
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TopSpeedKph = 250.0f;

	// 0-100 km/h acceleration time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float Acceleration0100 = 3.8f;

	// Max downforce in kg (EP9: 2000kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float DownforceMaxKg = 0.0f;

	// Wheelbase in mm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float WheelbaseMm = 3000.0f;

	// Body type: "hypercar", "sedan", "suv"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FString BodyType = TEXT("sedan");

	// Electric motor torque curve: 0 RPM = peak torque, decays at high RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Electric")
	bool bIsElectric = true;

	// Regenerative braking strength (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Electric")
	float RegenBrakingStrength = 0.3f;

	// Battery capacity in kWh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Electric")
	float BatteryCapacityKwh = 75.0f;

	// Range in km
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Electric")
	float RangeKm = 500.0f;
};

/**
 * Vehicle state manager - centralizes vehicle state data
 * Attach to any vehicle to provide standardized state access
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UVehicleStateManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleStateManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Get current vehicle state
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	const FNIOVehicleState& GetVehicleState() const { return VehicleState; }

	// Get vehicle type
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	ENIOVehicleType GetVehicleType() const { return VehicleType; }

	// Set vehicle type
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	void SetVehicleType(ENIOVehicleType InType) { VehicleType = InType; }

	// Get performance config
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	const FNIOPerformanceConfig& GetPerformanceConfig() const { return PerformanceConfig; }

	// Set performance config
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	void SetPerformanceConfig(const FNIOPerformanceConfig& InConfig) { PerformanceConfig = InConfig; }

	// Get vehicle specs for a given vehicle type (for UI display)
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	static FVehicleSpecs GetVehicleSpecs(ENIOVehicleType VehicleType);

	// Check if vehicle is NIO brand
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	bool IsNIOVehicle() const;

	// Get vehicle display name
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	FString GetVehicleDisplayName() const;

	// Check if vehicle is stuck (nearly stationary with throttle applied)
	UFUNCTION(BlueprintCallable, Category = "Recovery")
	bool IsStuck() const { return bIsStuck; }

	// Check if vehicle is flipped (upside down or severely tilted)
	UFUNCTION(BlueprintCallable, Category = "Recovery")
	bool IsFlipped() const { return bIsFlipped; }

	// Reset vehicle to a safe position and orientation
	UFUNCTION(BlueprintCallable, Category = "Recovery")
	void ResetVehicle();

	// Get the NIO vehicle movement component (for tire physics access)
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	UNIOVehicleMovementComponent* GetNIOMovement() const { return CachedNIOMovement; }

protected:
	virtual void BeginPlay() override;

	// Current vehicle state (updated every tick)
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle State")
	FNIOVehicleState VehicleState;

	// Vehicle type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
	ENIOVehicleType VehicleType = ENIOVehicleType::EP9;

	// Performance configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle State")
	FNIOPerformanceConfig PerformanceConfig;

private:
	// Update state from physics
	void UpdateStateFromPhysics(float DeltaTime);

	// Calculate slip angle for drift detection
	float CalculateSlipAngle() const;

	// Check for stuck/flip conditions and broadcast events
	void CheckStuckAndFlip(float DeltaTime);

	// --- Recovery State ---
	UPROPERTY(BlueprintReadOnly, Category = "Recovery", meta = (AllowPrivateAccess = "true"))
	float StuckTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Recovery", meta = (AllowPrivateAccess = "true"))
	float FlipTimer = 0.0f;

	UPROPERTY()
	FVector LastPosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Recovery", meta = (AllowPrivateAccess = "true"))
	bool bIsStuck = false;

	UPROPERTY(BlueprintReadOnly, Category = "Recovery", meta = (AllowPrivateAccess = "true"))
	bool bIsFlipped = false;

	UPROPERTY(EditAnywhere, Category = "Recovery")
	float StuckThreshold = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Recovery")
	float FlipAngleThreshold = 80.0f;

	// Distance threshold in cm; if moved less than this per tick, vehicle is considered stationary
	UPROPERTY(EditAnywhere, Category = "Recovery")
	float StuckDistanceThreshold = 10.0f;

	// Cached references
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> VehicleRoot;

	// Cached RaceManager for performance (avoid GetAllActorsOfClass every tick)
	UPROPERTY()
	TWeakObjectPtr<ARaceManager> CachedRaceManager;

	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedChaosVehicle;

	UPROPERTY()
	TObjectPtr<UNIOVehicleMovementComponent> CachedNIOMovement;
};
