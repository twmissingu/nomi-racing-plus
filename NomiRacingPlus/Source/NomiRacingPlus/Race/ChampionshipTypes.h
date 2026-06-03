// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChampionshipTypes.generated.h"

/**
 * Championship difficulty tier
 */
UENUM(BlueprintType)
enum class EChampionshipTier : uint8
{
	Beginner        UMETA(DisplayName = "Beginner"),
	Intermediate    UMETA(DisplayName = "Intermediate"),
	Pro             UMETA(DisplayName = "Pro"),
	Elite           UMETA(DisplayName = "Elite"),
	Legend          UMETA(DisplayName = "Legend")
};

/**
 * Championship standings entry for a single participant
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FChampionshipStandingEntry
{
	GENERATED_BODY()

	// Participant name
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	FString Name;

	// Is this the player?
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	bool bIsPlayer = false;

	// Total points
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	int32 Points = 0;

	// Finishing positions per race
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	TArray<int32> Positions;

	// Wins count
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	int32 Wins = 0;

	// Podiums count
	UPROPERTY(BlueprintReadWrite, Category = "Standing")
	int32 Podiums = 0;
};

/**
 * Championship data
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FChampionshipData
{
	GENERATED_BODY()

	// Championship unique ID
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FString ID;

	// Championship name
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FString Name;

	// Championship description
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FString Description;

	// Championship tier
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	EChampionshipTier Tier = EChampionshipTier::Beginner;

	// Tracks in championship
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TArray<FString> Tracks;

	// Current race index
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	int32 CurrentRace = 0;

	// Points per position (1st=25, 2nd=18, etc.)
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TMap<int32, int32> PointsPerPosition;

	// Player points
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	int32 PlayerPoints = 0;

	// AI opponent points
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TMap<FString, int32> AIOpponentPoints;

	// Full standings
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	TArray<FChampionshipStandingEntry> Standings;

	// Is championship complete?
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	bool bComplete = false;

	// Did the player win?
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	bool bPlayerWon = false;

	// Final player position in standings
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	int32 FinalPlayerPosition = 0;

	// Number of AI opponents
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	int32 NumAIOpponents = 7;

	// Difficulty (0=Easy, 1=Normal, 2=Hard, 3=Expert)
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	int32 Difficulty = 1;

	// Start timestamp
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FDateTime StartTime;

	// Completion timestamp
	UPROPERTY(BlueprintReadWrite, Category = "Championship")
	FDateTime CompletionTime;
};

/**
 * Championship history entry
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FChampionshipHistoryEntry
{
	GENERATED_BODY()

	// Championship data
	UPROPERTY(BlueprintReadWrite, Category = "History")
	FChampionshipData Championship;

	// Was the player the winner?
	UPROPERTY(BlueprintReadWrite, Category = "History")
	bool bPlayerWon = false;

	// Player final position
	UPROPERTY(BlueprintReadWrite, Category = "History")
	int32 PlayerPosition = 0;

	// Completion timestamp
	UPROPERTY(BlueprintReadWrite, Category = "History")
	FDateTime CompletionTime;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChampionshipCompleted, const FChampionshipData&, Championship, bool, bPlayerWon);
