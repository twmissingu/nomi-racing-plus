// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISensorSystem.h"
#include "AISlipstreamSystem.generated.h"

/**
 * Slipstream zone data
 * Represents the low-pressure wake zone behind a vehicle
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FSlipstreamZone
{
	GENERATED_BODY()

	// Vehicle creating the wake
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	TObjectPtr<APawn> SourceVehicle = nullptr;

	// Center of the wake zone (world space)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	FVector WakeCenter = FVector::ZeroVector;

	// Wake length (cm)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float WakeLength = 0.0f;

	// Wake width at widest point (cm)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float WakeWidth = 0.0f;

	// Current strength at optimal position (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float MaxStrength = 0.0f;

	// Speed of the source vehicle (km/h)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float SourceSpeed = 0.0f;
};

/**
 * Slipstream effect on vehicle
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FSlipstreamEffect
{
	GENERATED_BODY()

	// Is the vehicle currently in a slipstream zone?
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	bool bInSlipstream = false;

	// Current slipstream strength (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float Strength = 0.0f;

	// Speed boost factor (multiplier, e.g., 1.1 = 10% boost)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float SpeedBoostFactor = 1.0f;

	// Drag reduction factor (0-1, how much drag is reduced)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float DragReduction = 0.0f;

	// Source vehicle providing the slipstream
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	TObjectPtr<APawn> SourceVehicle = nullptr;

	// Optimal steering correction to stay in wake (-1 to 1)
	UPROPERTY(BlueprintReadOnly, Category = "Slipstream")
	float SteeringCorrection = 0.0f;
};

/**
 * Slipstream configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FSlipstreamConfig
{
	GENERATED_BODY()

	// Enable slipstream system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	bool bEnabled = true;

	// Maximum slipstream distance (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float MaxDistance = 2500.0f;

	// Wake cone half-angle (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float WakeConeAngle = 20.0f;

	// Maximum speed boost (multiplier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float MaxSpeedBoost = 1.12f;

	// Maximum drag reduction (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float MaxDragReduction = 0.35f;

	// Wake persistence time after leaving zone (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float WakeFadeTime = 0.5f;

	// Speed-dependent wake scaling: higher speed = longer wake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float SpeedWakeScale = 0.01f;

	// Lateral stability in slipstream (0-1, higher = easier to stay in wake)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float LateralStability = 0.7f;
};

/**
 * AI Slipstream System
 * Simulates aerodynamic wake zones behind vehicles
 * Provides drag reduction and speed boost for vehicles in the wake
 * Also provides AI behavior hints for drafting decisions
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAISlipstreamSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAISlipstreamSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Get current slipstream effect on this vehicle
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	const FSlipstreamEffect& GetCurrentEffect() const { return CurrentEffect; }

	// Get slipstream configuration
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	const FSlipstreamConfig& GetConfig() const { return Config; }

	// Set slipstream configuration
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	void SetConfig(const FSlipstreamConfig& InConfig) { Config = InConfig; }

	// Calculate slipstream effect from sensor data
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	void UpdateFromSensorData(const FAISensorData& SensorData);

	// Get the recommended steering to maintain draft position
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	float GetDraftSteering() const { return CurrentEffect.SteeringCorrection; }

	// Get the speed boost multiplier
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	float GetSpeedBoost() const { return CurrentEffect.SpeedBoostFactor; }

	// Is currently drafting?
	UFUNCTION(BlueprintCallable, Category = "AI|Slipstream")
	bool IsDrafting() const { return CurrentEffect.bInSlipstream; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Slipstream")
	FSlipstreamConfig Config;

private:
	// Calculate slipstream effect for a given vehicle position relative to a wake
	float CalculateWakeStrength(const FVector& VehiclePos, const FSlipstreamZone& Zone) const;

	// Calculate steering correction to stay centered in wake
	float CalculateSteeringCorrection(const FVector& VehiclePos, const FSlipstreamZone& Zone) const;

	// Build a slipstream zone from a detected vehicle
	FSlipstreamZone BuildWakeZone(const FAIDetectedVehicle& Vehicle) const;

	// Current effect
	FSlipstreamEffect CurrentEffect;

	// Active slipstream zone
	FSlipstreamZone ActiveZone;

	// Fade timer for when leaving slipstream zone
	float FadeTimer = 0.0f;
	bool bWasInSlipstream = false;

	// Cached owner pawn
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
};
