// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISensorSystem.generated.h"

/**
 * Detected vehicle data from sensor raycasts
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDetectedVehicle
{
	GENERATED_BODY()

	// Detected vehicle pawn
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	TObjectPtr<APawn> Vehicle = nullptr;

	// Distance to detected vehicle (cm)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	float Distance = 0.0f;

	// Relative velocity (positive = closing in)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	float RelativeSpeed = 0.0f;

	// Lateral offset from AI forward (-1 = left, 1 = right)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	float LateralOffset = 0.0f;

	// Is the vehicle ahead of us?
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bIsAhead = false;

	// Is the vehicle behind us?
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bIsBehind = false;

	// Angle to vehicle relative to forward (degrees)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	float AngleDeg = 0.0f;
};

/**
 * Sensor configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAISensorConfig
{
	GENERATED_BODY()

	// Forward detection range (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float ForwardRange = 3000.0f;

	// Rear detection range (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float RearRange = 2000.0f;

	// Side detection range (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float SideRange = 1500.0f;

	// Number of forward rays for sweep detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	int32 ForwardRayCount = 5;

	// Forward sweep half-angle (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float ForwardSweepAngle = 25.0f;

	// Detection cone angle for slipstream (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float SlipstreamConeAngle = 30.0f;

	// Slipstream max distance (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float SlipstreamMaxDistance = 2000.0f;

	// Update rate (seconds between full scans)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float UpdateInterval = 0.1f;
};

/**
 * Sensor scan result
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAISensorData
{
	GENERATED_BODY()

	// Vehicle directly ahead
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	FAIDetectedVehicle VehicleAhead;

	// Vehicle directly behind
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	FAIDetectedVehicle VehicleBehind;

	// Nearest vehicle to the left
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	FAIDetectedVehicle VehicleLeft;

	// Nearest vehicle to the right
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	FAIDetectedVehicle VehicleRight;

	// Vehicle ahead in slipstream zone
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	FAIDetectedVehicle SlipstreamTarget;

	// Is any vehicle detected ahead?
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bHasVehicleAhead = false;

	// Is any vehicle detected behind?
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bHasVehicleBehind = false;

	// Is slipstream available?
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bSlipstreamAvailable = false;

	// Slipstream strength (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	float SlipstreamStrength = 0.0f;

	// All detected vehicles (for overtake evaluation)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	TArray<FAIDetectedVehicle> NearbyVehicles;

	// Track boundary detection
	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bTrackEdgeLeft = false;

	UPROPERTY(BlueprintReadOnly, Category = "Sensor")
	bool bTrackEdgeRight = false;
};

/**
 * AI Sensor System
 * Multi-ray perception system for detecting nearby vehicles and track boundaries
 * Provides data to behavior tree, overtake evaluator, and slipstream system
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAISensorSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAISensorSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Get latest sensor data
	UFUNCTION(BlueprintCallable, Category = "AI|Sensor")
	const FAISensorData& GetSensorData() const { return SensorData; }

	// Set sensor configuration
	UFUNCTION(BlueprintCallable, Category = "AI|Sensor")
	void SetSensorConfig(const FAISensorConfig& InConfig) { Config = InConfig; }

	// Force an immediate scan
	UFUNCTION(BlueprintCallable, Category = "AI|Sensor")
	void ForceScan();

	// Get slipstream strength for a specific direction
	UFUNCTION(BlueprintCallable, Category = "AI|Sensor")
	float GetSlipstreamStrengthAt(const FVector& Direction) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sensor")
	FAISensorConfig Config;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Sensor")
	FAISensorData SensorData;

private:
	// Perform a full sensor scan
	void PerformScan(float DeltaTime);

	// Scan forward for vehicles
	void ScanForward();

	// Scan rear for vehicles
	void ScanRear();

	// Scan sides for vehicles
	void ScanSides();

	// Detect slipstream zones
	void DetectSlipstream();

	// Scan for track boundaries
	void ScanTrackBoundaries();

	// Calculate slipstream strength based on distance and alignment
	float CalculateSlipstreamStrength(const FAIDetectedVehicle& Vehicle) const;

	// Helper to trace and detect a vehicle
	bool TraceForVehicle(const FVector& Start, const FVector& Direction, float Range, FAIDetectedVehicle& OutVehicle) const;

	// Populate a detected vehicle struct from a hit result
	void PopulateDetectedVehicle(APawn* OtherVehicle, float Distance, float AngleDeg, bool bAhead, FAIDetectedVehicle& OutVehicle) const;

	// Scan timer
	float ScanTimer = 0.0f;

	// Cached owner pawn
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	// Actors to ignore in raycasts
	UPROPERTY()
	TArray<TObjectPtr<AActor>> IgnoredActors;
};
