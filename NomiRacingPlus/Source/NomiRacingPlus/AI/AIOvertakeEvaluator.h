// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISensorSystem.h"
#include "AIBehaviorTree.h"
#include "AIOvertakeEvaluator.generated.h"

/**
 * Overtake strategy types
 */
UENUM(BlueprintType)
enum class EOvertakeStrategy : uint8
{
	None             UMETA(DisplayName = "None"),
	InsidePass       UMETA(DisplayName = "Inside Pass"),
	OutsidePass      UMETA(DisplayName = "Outside Pass"),
	SlingshotPass     UMETA(DisplayName = "Slingshot Pass"),
	LateBrakePass    UMETA(DisplayName = "Late Brake Pass"),
	DraftAndPass     UMETA(DisplayName = "Draft and Pass")
};

/**
 * Overtake opportunity evaluation result
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FOvertakeOpportunity
{
	GENERATED_BODY()

	// Is there a viable overtake opportunity?
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	bool bViable = false;

	// Recommended strategy
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	EOvertakeStrategy Strategy = EOvertakeStrategy::None;

	// Target vehicle to overtake
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	TObjectPtr<APawn> TargetVehicle = nullptr;

	// Direction to move (-1 = left, 1 = right)
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	float Direction = 0.0f;

	// Confidence in this opportunity (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	float Confidence = 0.0f;

	// Estimated time to complete overtake (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	float EstimatedTime = 0.0f;

	// Risk level (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	float Risk = 0.0f;

	// Speed advantage needed (km/h)
	UPROPERTY(BlueprintReadOnly, Category = "Overtake")
	float SpeedAdvantageNeeded = 0.0f;
};

/**
 * Overtake evaluator personality modifiers
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FOvertakePersonality
{
	GENERATED_BODY()

	// How often the AI attempts overtakes (0-2, 1 = normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float FrequencyMultiplier = 1.0f;

	// Risk tolerance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float RiskTolerance = 0.5f;

	// Patience - how long to wait for a good opportunity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float Patience = 0.5f;

	// Prefer inside or outside passes (-1 = inside, 1 = outside, 0 = no preference)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float SidePreference = 0.0f;

	// Use slipstream for overtaking (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float DraftUsage = 0.5f;
};

/**
 * AI Overtake Evaluator
 * Analyzes sensor data to determine when and how to overtake other vehicles
 * Considers speed differentials, gap analysis, track position, and risk
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAIOvertakeEvaluator : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIOvertakeEvaluator();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Evaluate overtake opportunity based on current sensor data
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	FOvertakeOpportunity Evaluate(const FAISensorData& SensorData, float CurrentSpeedKmh);

	// Set personality modifiers
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	void SetPersonality(const FOvertakePersonality& InPersonality) { Personality = InPersonality; }

	// Get personality
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	const FOvertakePersonality& GetPersonality() const { return Personality; }

	// Cancel current overtake
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	void CancelOvertake();

	// Is currently executing an overtake?
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	bool IsOvertaking() const { return bIsOvertaking; }

	// Get current overtake progress (0-1)
	UFUNCTION(BlueprintCallable, Category = "AI|Overtake")
	float GetOvertakeProgress() const { return OvertakeProgress; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Overtake")
	FOvertakePersonality Personality;

	// Minimum gap width to attempt overtake (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Overtake")
	float MinGapWidth = 300.0f;

	// Minimum speed advantage to attempt overtake (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Overtake")
	float MinSpeedAdvantage = 5.0f;

	// Cooldown between overtake attempts (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Overtake")
	float OvertakeCooldown = 3.0f;

private:
	// Analyze gap between target vehicle and track edge
	float AnalyzeGapWidth(const FAIDetectedVehicle& Target, bool bInside) const;

	// Select best overtake strategy
	EOvertakeStrategy SelectStrategy(const FAIDetectedVehicle& Target, float CurrentSpeed, float GapWidthInside, float GapWidthOutside) const;

	// Calculate risk for an overtake attempt
	float CalculateRisk(const FAIDetectedVehicle& Target, EOvertakeStrategy Strategy, float GapWidth) const;

	// Check if late braking is viable
	bool CanLateBrake(const FAIDetectedVehicle& Target, float CurrentSpeed) const;

	// Calculate confidence based on all factors
	float CalculateConfidence(float SpeedAdvantage, float GapWidth, float Risk, EOvertakeStrategy Strategy) const;

	// Overtake state
	bool bIsOvertaking = false;
	float OvertakeProgress = 0.0f;
	float OvertakeCooldownTimer = 0.0f;
	float OvertakeTimer = 0.0f;
	EOvertakeStrategy ActiveStrategy = EOvertakeStrategy::None;

	// Cached owner pawn
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
};
