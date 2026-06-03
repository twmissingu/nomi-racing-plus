// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Race/ChampionshipTypes.h"
#include "ChampionshipManager.generated.h"

class URaceProgression;

/**
 * Predefined championship series definitions
 */

/**
 * Championship Manager
 * Manages predefined championship series, multi-race championship flow,
 * AI opponent generation, rewards, and season progression
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API AChampionshipManager : public AActor
{
	GENERATED_BODY()

public:
	AChampionshipManager();

	virtual void BeginPlay() override;

	// Championship Series

	// Get all available championship series for a given tier
	UFUNCTION(BlueprintCallable, Category = "Championship")
	TArray<FChampionshipData> GetChampionshipsForTier(EChampionshipTier Tier) const;

	// Get all available championships (unlocked)
	UFUNCTION(BlueprintCallable, Category = "Championship")
	TArray<FChampionshipData> GetAllAvailableChampionships() const;

	// Start a predefined championship by ID
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool StartChampionshipByID(const FString& ChampionshipID);

	// Start a championship from data
	UFUNCTION(BlueprintCallable, Category = "Championship")
	void StartChampionship(const FChampionshipData& Championship);

	// Championship Flow

	// Get current championship data
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool GetCurrentChampionship(FChampionshipData& OutChampionship) const;

	// Is there an active championship?
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool HasActiveChampionship() const;

	// Get the track for the current championship race
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool GetCurrentRaceTrack(FString& OutTrackName) const;

	// Get the race number within the championship (1-based)
	UFUNCTION(BlueprintCallable, Category = "Championship")
	int32 GetCurrentRaceNumber() const;

	// Get total races in current championship
	UFUNCTION(BlueprintCallable, Category = "Championship")
	int32 GetTotalRacesInChampionship() const;

	// Record results after a championship race
	UFUNCTION(BlueprintCallable, Category = "Championship")
	void RecordChampionshipRaceResult(int32 PlayerPosition, const TMap<FString, int32>& AIPositions);

	// Is the championship complete?
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool IsChampionshipComplete() const;

	// Standings

	// Get current championship standings
	UFUNCTION(BlueprintCallable, Category = "Championship")
	bool GetStandings(TArray<FChampionshipStandingEntry>& OutStandings) const;

	// Get player's current position in standings
	UFUNCTION(BlueprintCallable, Category = "Championship")
	int32 GetPlayerStandingPosition() const;

	// AI Opponents

	// Generate AI opponents for a championship
	UFUNCTION(BlueprintCallable, Category = "Championship")
	TArray<FString> GenerateAIOpponents(int32 Count, EChampionshipTier Tier) const;

	// Get AI names for current championship
	UFUNCTION(BlueprintCallable, Category = "Championship")
	const TArray<FString>& GetAIOpponentNames() const { return AIOpponentNames; }

	// Points System

	// Get default points table for a tier
	UFUNCTION(BlueprintCallable, Category = "Championship")
	TMap<int32, int32> GetDefaultPointsTable(EChampionshipTier Tier) const;

	// Rewards

	// Get reward for completing a championship at a given position
	UFUNCTION(BlueprintCallable, Category = "Championship")
	int32 GetChampionshipReward(EChampionshipTier Tier, int32 Position) const;

	// History

	// Get championship history from progression system
	UFUNCTION(BlueprintCallable, Category = "Championship")
	const TArray<FChampionshipHistoryEntry>& GetChampionshipHistory() const;

	// Reference to progression component (set externally or found on game instance)
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TObjectPtr<URaceProgression> ProgressionComponent;

protected:
	// All predefined championship templates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Championship")
	TArray<FChampionshipData> ChampionshipTemplates;

	// Current active championship
	UPROPERTY(BlueprintReadOnly, Category = "Championship")
	FChampionshipData ActiveChampionship;

	// Is there an active championship?
	UPROPERTY(BlueprintReadOnly, Category = "Championship")
	bool bHasActiveChampionship = false;

	// AI opponent names for current championship
	UPROPERTY(BlueprintReadOnly, Category = "Championship")
	TArray<FString> AIOpponentNames;

private:
	// Initialize predefined championship templates
	void InitializeChampionshipTemplates();

	// Initialize AI name pool
	void InitializeAINames();

	// Generate standings for a new championship
	void InitializeStandings(FChampionshipData& Championship);

	// AI name pool
	UPROPERTY()
	TArray<FString> AINamesPool;
};
