// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISensorSystem.h"
#include "AIOvertakeEvaluator.h"
#include "AIDefensiveEvaluator.h"
#include "AISlipstreamSystem.h"
#include "AIRubberBandScaler.h"
#include "AIBehaviorTree.generated.h"

/**
 * AI behavior states
 */
UENUM(BlueprintType)
enum class EAIBehaviorState : uint8
{
	Idle           UMETA(DisplayName = "Idle"),
	Accelerating   UMETA(DisplayName = "Accelerating"),
	Braking        UMETA(DisplayName = "Braking"),
	Cornering      UMETA(DisplayName = "Cornering"),
	Overtaking     UMETA(DisplayName = "Overtaking"),
	Defending      UMETA(DisplayName = "Defending"),
	Drafting       UMETA(DisplayName = "Drafting"),
	Recovering     UMETA(DisplayName = "Recovering"),
	Stuck          UMETA(DisplayName = "Stuck"),
	Finished       UMETA(DisplayName = "Finished")
};

/**
 * AI decision factors
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDecisionFactors
{
	GENERATED_BODY()

	// Distance to next waypoint
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float DistanceToWaypoint = 0.0f;

	// Distance to vehicle ahead
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float DistanceToVehicleAhead = 0.0f;

	// Distance to vehicle behind
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float DistanceToVehicleBehind = 0.0f;

	// Current speed
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float CurrentSpeed = 0.0f;

	// Recommended speed for current section
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float RecommendedSpeed = 0.0f;

	// Is on straight?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsOnStraight = false;

	// Is in corner?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsInCorner = false;

	// Corner sharpness (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float CornerSharpness = 0.0f;

	// Track position (0-1, 0 = start, 1 = end)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float TrackPosition = 0.0f;

	// Race position
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 RacePosition = 0;

	// Total racers
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 TotalRacers = 0;

	// Is player ahead?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsPlayerAhead = false;

	// Slipstream available?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bSlipstreamAvailable = false;

	// Slipstream strength (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float SlipstreamStrength = 0.0f;
};

/**
 * AI overtaking decision
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIOvertakeDecision
{
	GENERATED_BODY()

	// Should overtake?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bShouldOvertake = false;

	// Overtake direction (-1 = left, 1 = right)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float OvertakeDirection = 0.0f;

	// Overtake urgency (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float OvertakeUrgency = 0.0f;

	// Overtake type
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	FString OvertakeType;
};

/**
 * AI defensive decision
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDefensiveDecision
{
	GENERATED_BODY()

	// Should defend?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bShouldDefend = false;

	// Defense direction (-1 = left, 1 = right)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float DefenseDirection = 0.0f;

	// Block line
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bBlockInside = false;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bBlockOutside = false;
};

/**
 * AI slipstream configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAISlipstreamConfig
{
	GENERATED_BODY()

	// Enable slipstream?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	bool bEnabled = true;

	// Slipstream detection distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float DetectionDistance = 2000.0f; // 20 meters

	// Slipstream cone angle (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float ConeAngle = 30.0f;

	// Max speed boost from slipstream
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float MaxSpeedBoost = 0.15f;

	// Drafting distance (optimal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slipstream")
	float OptimalDraftingDistance = 500.0f; // 5 meters
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIStateChanged, EAIBehaviorState, NewState);

/**
 * AI Behavior Tree
 * Advanced AI decision making with overtaking, defending, slipstream, and rubber banding
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAIBehaviorTree : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIBehaviorTree();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Decision Making

	// Update AI decisions
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateDecisions(const FAIDecisionFactors& Factors);

	// Get current behavior state
	UFUNCTION(BlueprintCallable, Category = "AI")
	EAIBehaviorState GetBehaviorState() const { return CurrentState; }

	// Get throttle input
	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetThrottleInput() const { return ThrottleInput; }

	// Get brake input
	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetBrakeInput() const { return BrakeInput; }

	// Get steering input
	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetSteeringInput() const { return SteeringInput; }

	// Get overtake decision
	UFUNCTION(BlueprintCallable, Category = "AI")
	const FAIOvertakeDecision& GetOvertakeDecision() const { return OvertakeDecision; }

	// Get defensive decision
	UFUNCTION(BlueprintCallable, Category = "AI")
	const FAIDefensiveDecision& GetDefensiveDecision() const { return DefensiveDecision; }

	// Configuration

	// Set rubber band config
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRubberBandConfig(const FAIRubberBandConfig& Config) { RubberBandConfig = Config; }

	// Set slipstream config
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetSlipstreamConfig(const FAISlipstreamConfig& Config) { SlipstreamConfig = Config; }

	// Set difficulty (0-1)
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDifficulty(float InDifficulty) { Difficulty = FMath::Clamp(InDifficulty, 0.0f, 1.0f); }

	// Events

	// State changed delegate
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIStateChanged OnAIStateChanged;

protected:
	// Current behavior state
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EAIBehaviorState CurrentState = EAIBehaviorState::Idle;

	// Current inputs
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float ThrottleInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float BrakeInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float SteeringInput = 0.0f;

	// Current decisions
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	FAIOvertakeDecision OvertakeDecision;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	FAIDefensiveDecision DefensiveDecision;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	FAIRubberBandConfig RubberBandConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	FAISlipstreamConfig SlipstreamConfig;

	// Difficulty (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float Difficulty = 0.5f;

	// Get subsystem references

public:
	UFUNCTION(BlueprintCallable, Category = "AI|Subsystem")
	UAISensorSystem* GetSensorSystem() const { return SensorSystem; }

	UFUNCTION(BlueprintCallable, Category = "AI|Subsystem")
	UAIOvertakeEvaluator* GetOvertakeEvaluator() const { return OvertakeEvaluator; }

	UFUNCTION(BlueprintCallable, Category = "AI|Subsystem")
	UAIDefensiveEvaluator* GetDefensiveEvaluator() const { return DefensiveEvaluator; }

	UFUNCTION(BlueprintCallable, Category = "AI|Subsystem")
	UAISlipstreamSystem* GetSlipstreamSystem() const { return SlipstreamSystem; }

	UFUNCTION(BlueprintCallable, Category = "AI|Subsystem")
	UAIRubberBandScaler* GetRubberBandScaler() const { return RubberBandScaler; }

	// Get combined speed multiplier (base * rubber band * slipstream)
	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetCombinedSpeedMultiplier() const;

private:
	// Behavior evaluation
	void EvaluateBehavior(const FAIDecisionFactors& Factors);

	// Calculate throttle/brake
	void CalculateThrottleBrake(const FAIDecisionFactors& Factors);

	// Calculate steering
	void CalculateSteering(const FAIDecisionFactors& Factors);

	// Evaluate overtaking using overtake evaluator
	void EvaluateOvertake(const FAIDecisionFactors& Factors);

	// Evaluate defending using defensive evaluator
	void EvaluateDefend(const FAIDecisionFactors& Factors);

	// Evaluate slipstream using slipstream system
	void EvaluateSlipstream(const FAIDecisionFactors& Factors);

	// Apply rubber banding using rubber band scaler
	void ApplyRubberBanding(const FAIDecisionFactors& Factors);

	// Integrate subsystem results into final inputs
	void IntegrateSubsystemResults();

	// State transitions
	void TransitionToState(EAIBehaviorState NewState);

	// Subsystem references (created in BeginPlay)
	UPROPERTY()
	TObjectPtr<UAISensorSystem> SensorSystem;

	UPROPERTY()
	TObjectPtr<UAIOvertakeEvaluator> OvertakeEvaluator;

	UPROPERTY()
	TObjectPtr<UAIDefensiveEvaluator> DefensiveEvaluator;

	UPROPERTY()
	TObjectPtr<UAISlipstreamSystem> SlipstreamSystem;

	UPROPERTY()
	TObjectPtr<UAIRubberBandScaler> RubberBandScaler;

	// Cached overtake and defensive results
	FOvertakeOpportunity CurrentOvertakeOpportunity;
	FAIDefensiveAction CurrentDefensiveAction;

	// Timers
	float OvertakeTimer = 0.0f;
	float DefenseTimer = 0.0f;
	float StuckTimer = 0.0f;

	// Previous state for recovery
	EAIBehaviorState PreviousState = EAIBehaviorState::Idle;
};
