// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIRubberBandScaler.generated.h"

/**
 * Rubber band mode
 */
UENUM(BlueprintType)
enum class ERubberBandMode : uint8
{
	Disabled          UMETA(DisplayName = "Disabled"),
	DistanceBased     UMETA(DisplayName = "Distance Based"),
	PositionBased     UMETA(DisplayName = "Position Based"),
	Hybrid            UMETA(DisplayName = "Hybrid")
};

/**
 * Rubber band configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIRubberBandConfig
{
	GENERATED_BODY()

	// Rubber band mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	ERubberBandMode Mode = ERubberBandMode::Hybrid;

	// Speed boost when behind player (per distance unit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float CatchUpSpeedBoost = 0.15f;

	// Speed reduction when ahead of player (per distance unit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float SlowDownWhenAhead = 0.05f;

	// Maximum speed boost allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float MaxBoost = 0.25f;

	// Maximum speed reduction allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float MaxReduction = 0.15f;

	// Distance threshold to start rubber banding (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float ActivationDistance = 3000.0f;

	// Distance at which max rubber band effect is applied (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float MaxEffectDistance = 10000.0f;

	// Position-based boost per position behind player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float PositionBoostPerPlace = 0.03f;

	// Position-based reduction per position ahead of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float PositionReductionPerPlace = 0.015f;

	// Smoothing factor for rubber band transitions (higher = smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float SmoothingFactor = 2.0f;

	// Brake point adjustment when behind (reduces braking distance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float BrakePointAdjustment = 0.1f;

	// Cornering speed boost when behind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float CorneringBoostWhenBehind = 0.05f;

	// Minimum race progress before rubber banding activates (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
	float MinProgressToActivate = 0.1f;
};

/**
 * Rubber band state
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIRubberBandState
{
	GENERATED_BODY()

	// Current speed multiplier from rubber banding
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	float SpeedMultiplier = 1.0f;

	// Current brake point adjustment
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	float BrakePointOffset = 0.0f;

	// Current cornering speed bonus
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	float CorneringBonus = 0.0f;

	// Is rubber banding currently active?
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	bool bActive = false;

	// Current effect strength (-1 to 1, negative = behind, positive = ahead)
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	float EffectStrength = 0.0f;

	// Distance to player (cm)
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	float DistanceToPlayer = 0.0f;

	// Position relative to player (negative = behind, positive = ahead)
	UPROPERTY(BlueprintReadOnly, Category = "RubberBand")
	int32 PositionDelta = 0;
};

/**
 * AI Rubber Band Scaler
 * Dynamically adjusts AI performance based on distance and position relative to player
 * Creates exciting races where AI stays competitive without being unfair
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAIRubberBandScaler : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIRubberBandScaler();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Update rubber band state based on player distance and race position
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	void UpdateState(float DistanceToPlayer, int32 AIRacePosition, int32 PlayerRacePosition, float RaceProgress);

	// Get current rubber band state
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	const FAIRubberBandState& GetState() const { return State; }

	// Get speed multiplier (includes rubber band effect)
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	float GetSpeedMultiplier() const { return State.SpeedMultiplier; }

	// Get brake point adjustment
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	float GetBrakePointAdjustment() const { return State.BrakePointOffset; }

	// Get cornering bonus
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	float GetCorneringBonus() const { return State.CorneringBonus; }

	// Set configuration
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	void SetConfig(const FAIRubberBandConfig& InConfig) { Config = InConfig; }

	// Get configuration
	UFUNCTION(BlueprintCallable, Category = "AI|RubberBand")
	const FAIRubberBandConfig& GetConfig() const { return Config; }

	// Is rubber banding active?
	bool IsActive() const { return State.bActive; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|RubberBand")
	FAIRubberBandConfig Config;

private:
	// Calculate distance-based effect
	float CalculateDistanceEffect(float DistanceToPlayer) const;

	// Calculate position-based effect
	float CalculatePositionEffect(int32 AIPosition, int32 PlayerPosition) const;

	// Apply smoothing to prevent abrupt changes
	float SmoothEffect(float CurrentEffect, float TargetEffect, float DeltaTime) const;

	// Current state
	FAIRubberBandState State;

	// Smoothed effect for gradual transitions
	float SmoothedEffect = 0.0f;
};
