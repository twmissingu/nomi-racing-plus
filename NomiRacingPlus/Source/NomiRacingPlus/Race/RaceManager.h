// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaceManager.generated.h"

/**
 * Race event types for the event system
 */
UENUM(BlueprintType)
enum class ERaceEvent : uint8
{
	Overtake           UMETA(DisplayName = "Overtake"),
	Overtaken          UMETA(DisplayName = "Overtaken"),
	DriftStart         UMETA(DisplayName = "Drift Start"),
	DriftEnd           UMETA(DisplayName = "Drift End"),
	LapComplete        UMETA(DisplayName = "Lap Complete"),
	FastestLap         UMETA(DisplayName = "Fastest Lap"),
	HighSpeed          UMETA(DisplayName = "High Speed"),
	Collision          UMETA(DisplayName = "Collision"),
	FirstPlace         UMETA(DisplayName = "First Place"),
	LastPlace          UMETA(DisplayName = "Last Place"),
	RaceStart          UMETA(DisplayName = "Race Start"),
	RaceFinish         UMETA(DisplayName = "Race Finish"),
	CountdownStart     UMETA(DisplayName = "Countdown Start"),
	CheckpointPassed   UMETA(DisplayName = "Checkpoint Passed"),
	VehicleStuck       UMETA(DisplayName = "Vehicle Stuck"),
	VehicleFlipped     UMETA(DisplayName = "Vehicle Flipped")
};

/**
 * Race state machine states
 */
UENUM(BlueprintType)
enum class ERaceState : uint8
{
	Idle               UMETA(DisplayName = "Idle"),
	Loading            UMETA(DisplayName = "Loading"),
	Countdown          UMETA(DisplayName = "Countdown"),
	Racing             UMETA(DisplayName = "Racing"),
	Paused             UMETA(DisplayName = "Paused"),
	Finished           UMETA(DisplayName = "Finished"),
	PostRace           UMETA(DisplayName = "Post Race")
};

/**
 * Race configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FRaceConfig
{
	GENERATED_BODY()

	// Number of laps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 NumLaps = 3;

	// Maximum number of AI opponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 MaxAIOpponents = 7;

	// Track name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString TrackName;

	// Race mode: "StreetGT", "NIO", "Baja"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString RaceMode = TEXT("StreetGT");

	// Countdown duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float CountdownDuration = 5.0f;

	// Allow collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bAllowCollisions = true;

	// Collision penalty (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float CollisionPenalty = 0.0f;

	// Point-to-point race (no laps, Baja mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsPointToPoint = false;
};

/**
 * Racer data for tracking each participant
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FRacerData
{
	GENERATED_BODY()

	// Racer ID (0 = player, 1+ = AI)
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 RacerID = 0;

	// Display name
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	FString DisplayName;

	// Is this the player?
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bIsPlayer = false;

	// Current lap (0 = not started, 1+ = racing)
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentLap = 0;

	// Current checkpoint index
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentCheckpoint = 0;

	// Total checkpoints passed
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 TotalCheckpointsPassed = 0;

	// Current position (1-based)
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 Position = 0;

	// Lap times
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TArray<float> LapTimes;

	// Best lap time
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float BestLapTime = -1.0f;

	// Total race time
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TotalRaceTime = 0.0f;

	// Is finished?
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bFinished = false;

	// Vehicle reference
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TObjectPtr<APawn> VehiclePawn;

	// Progress through track (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TrackProgress = 0.0f;
};

/**
 * Race event delegate
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRaceEvent, ERaceEvent, const FRacerData&);

/**
 * Race Manager - controls race flow, timing, and positions
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ARaceManager : public AActor
{
	GENERATED_BODY()

public:
	ARaceManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Race Control

	// Start a new race with configuration
	UFUNCTION(BlueprintCallable, Category = "Race")
	void StartRace(const FRaceConfig& Config);

	// Pause the race
	UFUNCTION(BlueprintCallable, Category = "Race")
	void PauseRace();

	// Resume the race
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ResumeRace();

	// End the race
	UFUNCTION(BlueprintCallable, Category = "Race")
	void EndRace();

	// Reset race to idle
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ResetRace();

	// Racer Management

	// Register a racer
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void RegisterRacer(APawn* VehiclePawn, const FString& DisplayName, bool bIsPlayer = false);

	// Unregister a racer
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void UnregisterRacer(APawn* VehiclePawn);

	// Get racer data
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	bool GetRacerData(APawn* VehiclePawn, FRacerData& OutData) const;

	// Get all racer data
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	const TArray<FRacerData>& GetAllRacers() const { return Racers; }

	// Checkpoint System

	// Racer passes a checkpoint
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void RacerPassCheckpoint(APawn* VehiclePawn, int32 CheckpointIndex);

	// Racer crosses finish line
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void RacerCrossFinishLine(APawn* VehiclePawn);

	// Getters

	// Get current race state
	UFUNCTION(BlueprintCallable, Category = "Race")
	ERaceState GetRaceState() const { return RaceState; }

	// Get race config
	UFUNCTION(BlueprintCallable, Category = "Race")
	const FRaceConfig& GetRaceConfig() const { return RaceConfig; }

	// Get current countdown value
	UFUNCTION(BlueprintCallable, Category = "Race")
	float GetCountdownValue() const { return CountdownTimer; }

	// Get player position
	UFUNCTION(BlueprintCallable, Category = "Race")
	int32 GetPlayerPosition() const;

	// Get player lap
	UFUNCTION(BlueprintCallable, Category = "Race")
	int32 GetPlayerCurrentLap() const;

	// Get race timer
	UFUNCTION(BlueprintCallable, Category = "Race")
	float GetRaceTimer() const { return RaceTimer; }

	// Is race finished?
	UFUNCTION(BlueprintCallable, Category = "Race")
	bool IsRaceFinished() const;

	// Are all racers finished?
	bool AreAllRacersFinished() const;

	// Baja Mode

	// Get remaining distance to finish for a racer (meters)
	UFUNCTION(BlueprintCallable, Category = "Race|Baja")
	float GetDistanceToFinish(int32 RacerIndex) const;

	// Get progress percentage for a racer (0-100)
	UFUNCTION(BlueprintCallable, Category = "Race|Baja")
	float GetProgressPercent(int32 RacerIndex) const;

	// Event system

	// Race event delegate (C++ only, not BlueprintAssignable to support AddLambda in tests)
	FOnRaceEvent OnRaceEvent;

protected:
	// Current race state
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	ERaceState RaceState = ERaceState::Idle;

	// Race configuration
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	FRaceConfig RaceConfig;

	// All racers
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	TArray<FRacerData> Racers;

	// Countdown timer
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	float CountdownTimer = 0.0f;

	// Race timer
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	float RaceTimer = 0.0f;

	// Total checkpoints per lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 CheckpointsPerLap = 10;

	// Baja mode flag
	UPROPERTY(BlueprintReadOnly, Category = "Race|Baja")
	bool bIsBajaMode = false;

	// Total track distance in meters (for Baja mode)
	UPROPERTY(EditAnywhere, Category = "Race|Baja")
	float TotalTrackDistance = 5000.0f;

	// World positions of each checkpoint (for distance calculation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Baja")
	TArray<FVector> CheckpointPositions;

private:
	// Update countdown
	void UpdateCountdown(float DeltaTime);

	// Update race timer
	void UpdateRaceTimer(float DeltaTime);

	// Update positions
	void UpdatePositions();

	// Calculate racer progress
	float CalculateRacerProgress(const FRacerData& Racer) const;

	// Find racer by pawn
	int32 FindRacerIndex(APawn* VehiclePawn) const;

	// Broadcast race event
	void BroadcastRaceEvent(ERaceEvent Event, const FRacerData& RacerData);
};
