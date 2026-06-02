// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIBehaviorTree.h"
#include "AICarController.generated.h"

/**
 * AI difficulty levels
 */
UENUM(BlueprintType)
enum class EAIDifficulty : uint8
{
	Easy       UMETA(DisplayName = "Easy"),
	Normal     UMETA(DisplayName = "Normal"),
	Hard       UMETA(DisplayName = "Hard"),
	Expert     UMETA(DisplayName = "Expert")
};

/**
 * AI difficulty settings
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDifficultySettings
{
	GENERATED_BODY()

	// Speed multiplier (0.8 = 80% of max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Speed")
	float SpeedMultiplier = 1.0f;

	// Throttle smoothness (0-1, higher = smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Control")
	float ThrottleSmoothness = 0.8f;

	// Brake smoothness (0-1, higher = smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Control")
	float BrakeSmoothness = 0.7f;

	// Steering smoothness (0-1, higher = smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Control")
	float SteeringSmoothness = 0.6f;

	// Braking distance multiplier (1.0 = normal, 1.5 = brake earlier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Braking")
	float BrakingDistanceMultiplier = 1.0f;

	// Overtake aggressiveness (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Behavior")
	float OvertakeAggressiveness = 0.5f;

	// Cornering speed factor (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Cornering")
	float CorneringSpeedFactor = 0.8f;

	// Path deviation (0-1, higher = more random path)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Path")
	float PathDeviation = 0.1f;

	// Reaction time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|Reaction")
	float ReactionTime = 0.3f;

	// Rubber band effect strength (0 = none, 1 = strong)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty|RubberBand")
	float RubberBandStrength = 0.3f;
};

/**
 * AI state machine states
 */
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle         UMETA(DisplayName = "Idle"),
	Racing       UMETA(DisplayName = "Racing"),
	Overtaking   UMETA(DisplayName = "Overtaking"),
	Braking      UMETA(DisplayName = "Braking"),
	Recovering   UMETA(DisplayName = "Recovering"),
	Finished     UMETA(DisplayName = "Finished")
};

/**
 * AI waypoint data
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIWaypoint
{
	GENERATED_BODY()

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector Location = FVector::ZeroVector;

	// Recommended speed at this point (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float RecommendedSpeed = 100.0f;

	// Is this a corner?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bIsCorner = false;

	// Corner sharpness (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float CornerSharpness = 0.0f;
};

/**
 * AI Car Controller - controls AI opponent vehicles
 * Implements path following, overtaking, and difficulty scaling
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API AAICarController : public AAIController
{
	GENERATED_BODY()

public:
	AAICarController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// AI Control

	// Set AI difficulty
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetDifficulty(EAIDifficulty InDifficulty);

	// Get current difficulty
	UFUNCTION(BlueprintCallable, Category = "AI")
	EAIDifficulty GetDifficulty() const { return Difficulty; }

	// Set waypoints for AI to follow
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetWaypoints(const TArray<FAIWaypoint>& InWaypoints);

	// Start racing
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartRacing();

	// Stop racing
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopRacing();

	// Get current AI state
	UFUNCTION(BlueprintCallable, Category = "AI")
	EAIState GetAIState() const { return AIState; }

	// Set target speed (for rubber banding)
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetSpeedMultiplier(float Multiplier);

protected:
	// Current difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EAIDifficulty Difficulty = EAIDifficulty::Normal;

	// Difficulty settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TMap<EAIDifficulty, FAIDifficultySettings> DifficultySettings;

	// Current waypoints
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TArray<FAIWaypoint> Waypoints;

	// Current waypoint index
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 CurrentWaypointIndex = 0;

	// Current AI state
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EAIState AIState = EAIState::Idle;

	// Is racing?
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsRacing = false;

	// Target speed multiplier (for rubber banding)
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float TargetSpeedMultiplier = 1.0f;

private:
	// AI decision making
	void UpdateAIDecision(float DeltaTime);

	// Calculate steering toward waypoint
	float CalculateSteering(const FVector& TargetLocation);

	// Calculate throttle/brake
	void CalculateThrottleBrake(float& OutThrottle, float& OutBrake, float DeltaTime);

	// Check for obstacles
	bool CheckForObstacles(FVector& OutAvoidanceDirection);

	// Overtake logic (legacy - now handled by behavior tree)
	void UpdateOvertakeLogic(float DeltaTime);

	// Apply AI inputs to vehicle
	void ApplyAIInputs(float Throttle, float Brake, float Steering, float DeltaTime);

	// Get current difficulty settings
	const FAIDifficultySettings& GetCurrentSettings() const;

	// Update rubber band scaler with race context
	void UpdateRubberBandContext();

	// Behavior tree component
	UPROPERTY()
	TObjectPtr<UAIBehaviorTree> BehaviorTree;

	// Cached component references (avoid FindComponentByClass every frame)
	UPROPERTY()
	TObjectPtr<UVehicleStateManager> CachedStateManager;

	UPROPERTY()
	TObjectPtr<UNIOVehicleMovementComponent> CachedMovementComponent;

	// Smooth input values
	float SmoothThrottle = 0.0f;
	float SmoothBrake = 0.0f;
	float SmoothSteering = 0.0f;

	// Overtake timer
	float OvertakeTimer = 0.0f;

	// Reaction timer
	float ReactionTimer = 0.0f;

	// Cached vehicle reference
	UPROPERTY()
	TObjectPtr<APawn> ControlledVehicle;

	// Cached world reference
	UPROPERTY()
	TObjectPtr<UWorld> CachedWorld;
};
