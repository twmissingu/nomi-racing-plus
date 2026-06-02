// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICarController.generated.h"

class UAIBehaviorTree;
class UVehicleStateManager;
class UNIOVehicleMovementComponent;

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
 * AI state machine
 */
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Racing      UMETA(DisplayName = "Racing"),
	Overtaking  UMETA(DisplayName = "Overtaking"),
	Defending   UMETA(DisplayName = "Defending"),
	Recovering  UMETA(DisplayName = "Recovering"),
	Finished    UMETA(DisplayName = "Finished")
};

/**
 * AI difficulty settings
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIDifficultySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ThrottleSmoothness = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float BrakeSmoothness = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SteeringSmoothness = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float BrakingDistanceMultiplier = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float OvertakeAggressiveness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float CorneringSpeedFactor = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PathDeviation = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ReactionTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RubberBandStrength = 0.3f;
};

/**
 * AI waypoint for path following
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAIWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float RecommendedSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bIsCorner = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float CornerSharpness = 0.0f;
};

/**
 * AI Car Controller - manages AI driving behavior
 */
UCLASS()
class NOMIRACINGPLUS_API AAICarController : public AAIController
{
	GENERATED_BODY()

public:
	AAICarController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// Set AI difficulty
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetDifficulty(EAIDifficulty InDifficulty);

	// Set waypoints for AI to follow
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetWaypoints(const TArray<FAIWaypoint>& InWaypoints);

	// Start racing
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartRacing();

	// Stop racing
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopRacing();

	// Set target speed multiplier
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetSpeedMultiplier(float Multiplier);

	// Get current difficulty
	UFUNCTION(BlueprintCallable, Category = "AI")
	EAIDifficulty GetDifficulty() const { return Difficulty; }

	// Get current AI state
	UFUNCTION(BlueprintCallable, Category = "AI")
	EAIState GetAIState() const { return AIState; }

protected:
	// Current difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EAIDifficulty Difficulty = EAIDifficulty::Normal;

	// Current AI state
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EAIState AIState = EAIState::Idle;

	// Difficulty settings per level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TMap<EAIDifficulty, FAIDifficultySettings> DifficultySettings;

	// Waypoints
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	TArray<FAIWaypoint> Waypoints;

	// Current waypoint index
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int32 CurrentWaypointIndex = 0;

	// Is currently racing
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsRacing = false;

	// Target speed multiplier (for rubber banding)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float TargetSpeedMultiplier = 1.0f;

private:
	// Update AI decision making
	void UpdateAIDecision(float DeltaTime);

	// Calculate steering toward target
	float CalculateSteering(const FVector& TargetLocation);

	// Calculate throttle and brake
	void CalculateThrottleBrake(float& OutThrottle, float& OutBrake, float DeltaTime);

	// Check for obstacles ahead
	bool CheckForObstacles(FVector& OutAvoidanceDirection);

	// Update overtake logic
	void UpdateOvertakeLogic(float DeltaTime);

	// Apply AI inputs to vehicle
	void ApplyAIInputs(float Throttle, float Brake, float Steering, float DeltaTime);

	// Get current difficulty settings
	const FAIDifficultySettings& GetCurrentSettings() const;

	// Update rubber band context
	void UpdateRubberBandContext();

	// Generate default waypoints for AI to follow
	void GenerateDefaultWaypoints();

	// Cached references
	UPROPERTY()
	TObjectPtr<APawn> ControlledVehicle;

	UPROPERTY()
	TObjectPtr<UWorld> CachedWorld;

	UPROPERTY()
	TObjectPtr<UVehicleStateManager> CachedStateManager;

	UPROPERTY()
	TObjectPtr<UNIOVehicleMovementComponent> CachedMovementComponent;

	UPROPERTY()
	TObjectPtr<UAIBehaviorTree> BehaviorTree;

	// Input smoothing
	float SmoothThrottle = 0.0f;
	float SmoothBrake = 0.0f;
	float SmoothSteering = 0.0f;

	// Reaction timer
	float ReactionTimer = 0.0f;

	// Overtake timer
	float OvertakeTimer = 0.0f;
};
