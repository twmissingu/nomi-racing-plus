// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TirePhysicsModel.h"
#include "VehicleStateManager.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UNIOVehicleMovementComponent;

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

	// Check if vehicle is NIO brand
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	bool IsNIOVehicle() const;

	// Get vehicle display name
	UFUNCTION(BlueprintCallable, Category = "Vehicle State")
	FString GetVehicleDisplayName() const;

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

	// Cached references
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> VehicleRoot;

	UPROPERTY()
	TObjectPtr<UWorld> CachedWorld;

	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedChaosVehicle;

	UPROPERTY()
	TObjectPtr<UNIOVehicleMovementComponent> CachedNIOMovement;
};
