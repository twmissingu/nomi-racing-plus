// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Race/RaceManager.h"
#include "Race/RaceProgression.h"
#include "Race/ChampionshipManager.h"
#include "Vehicles/VehicleStateManager.h"
#include "AI/AICarController.h"
#include "NOMI/CommentaryEngine.h"
#include "NOMI/NOMIController.h"
#include "NomiRaceGameMode.generated.h"

class URaceHUD;
class UResultsWidget;

/**
 * Game mode for NIO Racing Plus
 * Manages race flow, player, and AI vehicles
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANomiRaceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANomiRaceGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Race Management

	// Start a new race
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void StartNewRace(const FRaceConfig& Config);

	// Get race manager
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	ARaceManager* GetRaceManager() const { return RaceManager; }

	// Player Management

	// Get player vehicle
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	APawn* GetPlayerVehicle() const;

	// Set player vehicle type
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void SetPlayerVehicleType(ENIOVehicleType VehicleType);

	// AI Management

	// Spawn AI opponents
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void SpawnAIOpponents(int32 Count);

	// Set AI difficulty
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void SetAIDifficulty(EAIDifficulty Difficulty);

	// Championship Integration

	// Start a championship
	UFUNCTION(BlueprintCallable, Category = "Game Mode|Championship")
	bool StartChampionship(const FString& ChampionshipID);

	// Get championship manager
	UFUNCTION(BlueprintCallable, Category = "Game Mode|Championship")
	AChampionshipManager* GetChampionshipManager() const { return ChampionshipManager; }

	// Is currently in a championship?
	UFUNCTION(BlueprintCallable, Category = "Game Mode|Championship")
	bool IsInChampionship() const;

	// Get the current championship race track
	UFUNCTION(BlueprintCallable, Category = "Game Mode|Championship")
	bool GetCurrentChampionshipTrack(FString& OutTrackName) const;

protected:
	// Race manager reference
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	TObjectPtr<ARaceManager> RaceManager;

	// Player vehicle type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
	ENIOVehicleType PlayerVehicleType = ENIOVehicleType::EP9;

	// AI vehicle types to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
	TArray<ENIOVehicleType> AIVehicleTypes;

	// AI difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
	EAIDifficulty AIDifficulty = EAIDifficulty::Normal;

	// Default race config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
	FRaceConfig DefaultRaceConfig;

	// Championship manager reference
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode|Championship")
	TObjectPtr<AChampionshipManager> ChampionshipManager;

	// Race progression reference
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode|Progression")
	TObjectPtr<URaceProgression> RaceProgression;

	// NOMI commentary engine component
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode|NOMI")
	TObjectPtr<UCommentaryEngine> CommentaryEngine;

	// NOMI controller (visual representation)
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode|NOMI")
	TObjectPtr<ANOMIController> NOMIController;

	// Race HUD widget
	UPROPERTY()
	TObjectPtr<URaceHUD> RaceHUDWidget;

	// Auto-start race delay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Mode")
	float RaceStartDelay = 3.0f;

	// Has the race started?
	bool bRaceStarted = false;

	// Player's starting position (tracked for achievements like Underdog)
	int32 PlayerStartingPosition = 0;

	// Cached vehicle state manager for the player pawn
	UPROPERTY()
	TObjectPtr<UVehicleStateManager> CachedPlayerVSM;

private:
	// Initialize race manager
	void InitializeRaceManager();

	// Spawn player vehicle
	void SpawnPlayerVehicle();

	// Get spawn class for a vehicle type (shared between player and AI)
	TSubclassOf<APawn> GetVehicleSpawnClass(ENIOVehicleType VehicleType) const;

	// Handle race events
	UFUNCTION()
	void OnRaceEvent(ERaceEvent Event, const FRacerData& RacerData);

	// Forward race event to NOMI commentary engine
	void ForwardEventToNOMI(ERaceEvent Event, const FRacerData& RacerData);
};
