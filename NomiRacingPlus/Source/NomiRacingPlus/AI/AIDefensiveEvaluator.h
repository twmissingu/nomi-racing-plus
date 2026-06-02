// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISensorSystem.h"
#include "AIDefensiveEvaluator.generated.h"

/**
 * Defensive strategy types
 */
UENUM(BlueprintType)
enum class EDefensiveStrategy : uint8
{
	None              UMETA(DisplayName = "None"),
	BlockInside       UMETA(DisplayName = "Block Inside"),
	BlockOutside      UMETA(DisplayName = "Block Outside"),
	MirrorMovement    UMETA(DisplayName = "Mirror Movement"),
	HoldLine          UMETA(DisplayName = "Hold Line"),
	SqueezeOff        UMETA(DisplayName = "Squeeze Off"),
	DefensiveBrake    UMETA(DisplayName = "Defensive Brake")
};

/**
 * Defensive action result
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDefensiveAction
{
	GENERATED_BODY()

	// Should take defensive action?
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	bool bShouldDefend = false;

	// Selected strategy
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	EDefensiveStrategy Strategy = EDefensiveStrategy::None;

	// Target vehicle to defend against
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	TObjectPtr<APawn> ThreatVehicle = nullptr;

	// Direction to move for defense (-1 = left, 1 = right)
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	float DefenseDirection = 0.0f;

	// Urgency of defense (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	float Urgency = 0.0f;

	// Should adjust speed defensively?
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	bool bAdjustSpeed = false;

	// Target speed adjustment factor (0.8-1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	float SpeedAdjustment = 1.0f;
};

/**
 * Defensive personality modifiers
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDefensivePersonality
{
	GENERATED_BODY()

	// How aggressively the AI defends (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float Aggressiveness = 0.5f;

	// How early the AI starts defending (0-1, higher = earlier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float Awareness = 0.5f;

	// Tendency to block specific side (-1 = always block inside, 1 = always block outside)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float BlockPreference = 0.0f;

	// Willingness to squeeze opponents off track (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float SqueezeTendency = 0.3f;

	// Use defensive braking (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float BrakeDefensiveness = 0.2f;
};

/**
 * AI Defensive Evaluator
 * Analyzes sensor data to determine when and how to defend position
 * Considers threat level, track position, and available blocking options
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAIDefensiveEvaluator : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIDefensiveEvaluator();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Evaluate defensive action based on sensor data
	UFUNCTION(BlueprintCallable, Category = "AI|Defense")
	FAIDefensiveAction Evaluate(const FAISensorData& SensorData, float CurrentSpeedKmh, int32 RacePosition);

	// Set defensive personality
	UFUNCTION(BlueprintCallable, Category = "AI|Defense")
	void SetPersonality(const FAIDefensivePersonality& InPersonality) { Personality = InPersonality; }

	// Get personality
	UFUNCTION(BlueprintCallable, Category = "AI|Defense")
	const FAIDefensivePersonality& GetPersonality() const { return Personality; }

	// Stop defending
	UFUNCTION(BlueprintCallable, Category = "AI|Defense")
	void StopDefending();

	// Is currently defending?
	UFUNCTION(BlueprintCallable, Category = "AI|Defense")
	bool IsDefending() const { return bIsDefending; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Defense")
	FAIDefensivePersonality Personality;

	// Distance at which to start defending (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Defense")
	float DefenseActivationDistance = 1500.0f;

	// Minimum threat level to activate defense (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Defense")
	float MinThreatLevel = 0.3f;

private:
	// Calculate threat level from a vehicle behind
	float CalculateThreatLevel(const FAIDetectedVehicle& Attacker, float CurrentSpeed) const;

	// Select defensive strategy
	EDefensiveStrategy SelectStrategy(const FAIDetectedVehicle& Attacker, float ThreatLevel) const;

	// Determine which side the attacker is approaching from
	float GetAttackSide(const FAIDetectedVehicle& Attacker) const;

	// Check if blocking a specific side is viable
	bool CanBlockSide(float Direction, bool bTrackEdgeLeft, bool bTrackEdgeRight) const;

	// Defensive state
	bool bIsDefending = false;
	float DefenseTimer = 0.0f;
	EDefensiveStrategy ActiveStrategy = EDefensiveStrategy::None;

	// Previous attacker for tracking movement
	UPROPERTY()
	TObjectPtr<APawn> PreviousAttacker = nullptr;

	float PreviousAttackerLateral = 0.0f;

	// Cached owner pawn
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
};
