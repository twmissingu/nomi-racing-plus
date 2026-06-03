// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Race/ChampionshipTypes.h"
#include "Race/RaceProgressionTypes.h"
#include "RaceProgression.generated.h"

/**
 * Race Progression System
 * Manages achievements, statistics, championships, and unlockables
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API URaceProgression : public UActorComponent
{
	GENERATED_BODY()

public:
	URaceProgression();

	virtual void BeginPlay() override;

	// Achievement System

	// Get all achievements
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	const TMap<EAchievement, FAchievementData>& GetAchievements() const { return Achievements; }

	// Get specific achievement
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool GetAchievement(EAchievement Achievement, FAchievementData& OutData) const;

	// Unlock achievement
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool UnlockAchievement(EAchievement Achievement);

	// Update achievement progress
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void UpdateAchievementProgress(EAchievement Achievement, float Progress);

	// Get number of unlocked achievements
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	int32 GetUnlockedAchievementCount() const;

	// Get total achievement count
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	int32 GetTotalAchievementCount() const { return Achievements.Num(); }

	// Statistics

	// Get player statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	const FPlayerStatistics& GetStatistics() const { return Statistics; }

	// Update statistics after race (legacy - use RecordRaceSession instead)
	UFUNCTION(BlueprintCallable, Category = "Statistics", meta = (DeprecatedFunction, DeprecationMessage = "Use RecordRaceSession instead"))
	void UpdateStatistics(const FPlayerStatistics& RaceStats);

	// Record a complete race session result - primary entry point for post-race processing
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void RecordRaceSession(const FRaceSessionResult& SessionResult);

	// Get per-track statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	bool GetTrackStatistics(const FString& TrackName, FTrackStatistics& OutStats) const;

	// Get per-vehicle statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	bool GetVehicleStatistics(const FString& VehicleName, FVehicleStatistics& OutStats) const;

	// Get recent session history
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	const TArray<FRaceSessionResult>& GetRecentSessions() const { return Statistics.RecentSessions; }

	// Championship

	// Start championship
	UFUNCTION(BlueprintCallable, Category = "Championship")
	void StartChampionship(const FChampionshipData& Championship);

	// Get current championship
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool GetCurrentChampionship(FChampionshipData& OutChampionship) const;

	// Is there an active championship?
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool HasActiveChampionship() const { return bHasActiveChampionship; }

	// Update championship results after a single race
	UFUNCTION(BlueprintCallable, Category = "Championship")
	void UpdateChampionshipResults(int32 PlayerPosition, const TMap<FString, int32>& AIPositions);

	// Get championship history
	UFUNCTION(BlueprintCallable, Category = "Championship")
	const TArray<FChampionshipHistoryEntry>& GetChampionshipHistory() const { return ChampionshipHistory; }

	// Get championship win count
	UFUNCTION(BlueprintCallable, Category = "Championship")
	int32 GetChampionshipWinCount() const;

	// Unlockables

	// Get all unlockables
	UFUNCTION(BlueprintCallable, Category = "Unlockables")
	const TMap<FString, FUnlockableItem>& GetUnlockables() const { return Unlockables; }

	// Get unlockables by type
	UFUNCTION(BlueprintCallable, Category = "Unlockables")
	TArray<FUnlockableItem> GetUnlockablesByType(EUnlockableType Type) const;

	// Unlock item
	UFUNCTION(BlueprintCallable, Category = "Unlockables")
	bool UnlockItem(const FString& ItemID);

	// Is item unlocked?
	UFUNCTION(BlueprintCallable, Category = "Unlockables")
	bool IsItemUnlocked(const FString& ItemID) const;

	// Get unlock progress (percentage of items unlocked)
	UFUNCTION(BlueprintCallable, Category = "Unlockables")
	float GetUnlockProgress() const;

	// Manual initialization (for components created outside actor lifecycle)
	// Call this after NewObject when BeginPlay() won't be called
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitializeSystems();

	// Save/Load

	// Save progression
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveProgression();

	// Load progression
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadProgression();

	// Events

	// Achievement unlocked delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	// Item unlocked delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemUnlocked OnItemUnlocked;

	// Race session completed delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceSessionCompleted OnRaceSessionCompleted;

	// Championship completed delegate
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChampionshipCompleted OnChampionshipCompleted;

protected:
	// Achievements
	UPROPERTY(BlueprintReadWrite, Category = "Achievements")
	TMap<EAchievement, FAchievementData> Achievements;

	// Statistics
	UPROPERTY(BlueprintReadWrite, Category = "Statistics")
	FPlayerStatistics Statistics;

	// Current championship
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FChampionshipData CurrentChampionship;

	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	bool bHasActiveChampionship = false;

	// Championship history
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TArray<FChampionshipHistoryEntry> ChampionshipHistory;

	// Unlockables
	UPROPERTY(BlueprintReadWrite, Category = "Unlockables")
	TMap<FString, FUnlockableItem> Unlockables;

	// Maximum recent sessions to keep
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 MaxRecentSessions = 50;

private:
	// Initialize achievements
	void InitializeAchievements();

	// Initialize unlockables
	void InitializeUnlockables();

	// Process a race session result into statistics
	void ProcessSessionToStatistics(const FRaceSessionResult& SessionResult);

	// Update per-track statistics from session
	void UpdateTrackStatistics(const FRaceSessionResult& SessionResult);

	// Update per-vehicle statistics from session
	void UpdateVehicleStatistics(const FRaceSessionResult& SessionResult);

	// Update streaks
	void UpdateStreaks(const FRaceSessionResult& SessionResult);

	// Check and unlock achievements based on current statistics
	void CheckAchievements();

	// Check stat-based unlockables
	void CheckStatBasedUnlocks();

	// Save file path
	FString GetSavePath() const;

	// Championship history save path
	FString GetChampionshipHistoryPath() const;
};
