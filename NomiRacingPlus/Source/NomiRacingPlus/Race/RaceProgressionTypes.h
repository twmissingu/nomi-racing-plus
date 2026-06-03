// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RaceProgressionTypes.generated.h"

/**
 * Achievement types - expanded with tiered and additional achievements
 */
UENUM(BlueprintType)
enum class EAchievement : uint8
{
	// Race milestones
	FirstRace           UMETA(DisplayName = "First Race"),
	FirstWin            UMETA(DisplayName = "First Win"),
	Champion            UMETA(DisplayName = "Champion"),
	ConsistentRacer     UMETA(DisplayName = "Consistent Racer"),
	VeteranRacer        UMETA(DisplayName = "Veteran Racer"),
	LegendaryRacer      UMETA(DisplayName = "Legendary Racer"),

	// Speed achievements
	SpeedDemon          UMETA(DisplayName = "Speed Demon"),
	SpeedKing           UMETA(DisplayName = "Speed King"),

	// Skill achievements
	DriftKing           UMETA(DisplayName = "Drift King"),
	DriftLegend         UMETA(DisplayName = "Drift Legend"),
	PerfectLap          UMETA(DisplayName = "Perfect Lap"),
	PerfectRace         UMETA(DisplayName = "Perfect Race"),
	NoCollision         UMETA(DisplayName = "No Collision"),
	OvertakeMaster      UMETA(DisplayName = "Overtake Master"),
	OvertakeGod         UMETA(DisplayName = "Overtake God"),

	// Collection achievements
	NIOFan              UMETA(DisplayName = "NIO Fan"),
	AllTracks           UMETA(DisplayName = "All Tracks"),
	AllVehicles         UMETA(DisplayName = "All Vehicles"),

	// Difficulty achievements
	HardDifficulty      UMETA(DisplayName = "Hard Difficulty"),
	ExpertDifficulty    UMETA(DisplayName = "Expert Difficulty"),

	// Comeback achievements
	Underdog            UMETA(DisplayName = "Underdog"),
	Comeback            UMETA(DisplayName = "Comeback"),

	// Endurance achievements
	Endurance           UMETA(DisplayName = "Endurance"),
	UltraEndurance      UMETA(DisplayName = "Ultra Endurance"),

	// Championship achievements
	DoubleChampion      UMETA(DisplayName = "Double Champion"),
	TripleChampion      UMETA(DisplayName = "Triple Champion"),

	// Track mastery
	CircuitMaster       UMETA(DisplayName = "Circuit Master"),
	StreetMaster        UMETA(DisplayName = "Street Master"),

	// Special achievements
	OnePercentClub      UMETA(DisplayName = "1% Club"),
	NightRider          UMETA(DisplayName = "Night Rider"),
	SunsetChaser        UMETA(DisplayName = "Sunset Chaser"),
	HundredClub         UMETA(DisplayName = "100 Club")
};

/**
 * Unlockable item types
 */
UENUM(BlueprintType)
enum class EUnlockableType : uint8
{
	Vehicle         UMETA(DisplayName = "Vehicle"),
	Track           UMETA(DisplayName = "Track"),
	PaintJob        UMETA(DisplayName = "Paint Job"),
	WheelDesign     UMETA(DisplayName = "Wheel Design"),
	NOMIVoice       UMETA(DisplayName = "NOMI Voice"),
	GameMode        UMETA(DisplayName = "Game Mode"),
	Championship    UMETA(DisplayName = "Championship"),
	Title           UMETA(DisplayName = "Title"),
	Decal           UMETA(DisplayName = "Decal")
};

/**
 * Achievement data
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAchievementData
{
	GENERATED_BODY()

	// Achievement type
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	EAchievement Achievement = EAchievement::FirstRace;

	// Display name
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FString DisplayName;

	// Description
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FString Description;

	// Icon path
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FString IconPath;

	// Is unlocked?
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	bool bUnlocked = false;

	// Unlock timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FDateTime UnlockTime;

	// Progress (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	float Progress = 0.0f;

	// Target value for unlock
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	float TargetValue = 1.0f;
};

/**
 * Per-track statistics
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTrackStatistics
{
	GENERATED_BODY()

	// Track name
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	FString TrackName;

	// Times raced on this track
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	int32 RacesCompleted = 0;

	// Wins on this track
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	int32 Wins = 0;

	// Best lap time on this track
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	float BestLapTime = -1.0f;

	// Best lap vehicle
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	FString BestLapVehicle;

	// Total laps on this track
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	int32 TotalLaps = 0;

	// Best position on this track
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	int32 BestPosition = 0;

	// Best race time
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	float BestRaceTime = -1.0f;

	// Total distance on this track (meters)
	UPROPERTY(BlueprintReadWrite, Category = "TrackStats")
	float TotalDistance = 0.0f;
};

/**
 * Per-vehicle statistics
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FVehicleStatistics
{
	GENERATED_BODY()

	// Vehicle name
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	FString VehicleName;

	// Times used
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	int32 RacesCompleted = 0;

	// Wins with this vehicle
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	int32 Wins = 0;

	// Best lap time with this vehicle
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	float BestLapTime = -1.0f;

	// Best lap track
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	FString BestLapTrack;

	// Total distance with this vehicle (meters)
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	float TotalDistance = 0.0f;

	// Max speed with this vehicle
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	float MaxSpeed = 0.0f;

	// Total drift time with this vehicle
	UPROPERTY(BlueprintReadWrite, Category = "VehicleStats")
	float TotalDriftTime = 0.0f;
};

/**
 * Race session result - captured after each race for statistics processing
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FRaceSessionResult
{
	GENERATED_BODY()

	// Track name
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString TrackName;

	// Vehicle used
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString VehicleName;

	// Final position (1-based)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 FinalPosition = 0;

	// Starting position (1-based, 0 = unknown)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 StartingPosition = 0;

	// Total racers in session
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 TotalRacers = 0;

	// Number of laps
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 NumLaps = 0;

	// All lap times
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	TArray<float> LapTimes;

	// Best lap time
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float BestLapTime = -1.0f;

	// Total race time
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float TotalRaceTime = 0.0f;

	// Max speed achieved (km/h)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float MaxSpeed = 0.0f;

	// Total drift time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float DriftTime = 0.0f;

	// Total overtakes
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 Overtakes = 0;

	// Number of collisions
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 Collisions = 0;

	// Distance driven (meters)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float DistanceDriven = 0.0f;

	// Was a clean race (no collisions)?
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bCleanRace = false;

	// Race mode used
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString RaceMode;

	// Difficulty level (0=Easy, 1=Normal, 2=Hard, 3=Expert)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 Difficulty = 1;

	// Whether this was part of a championship
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bChampionshipRace = false;

	// Championship name (if applicable)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FString ChampionshipName;

	// Timestamp
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FDateTime Timestamp;
};

/**
 * Player statistics
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPlayerStatistics
{
	GENERATED_BODY()

	// Total races started
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalRaces = 0;

	// Total wins
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalWins = 0;

	// Total podiums (top 3)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalPodiums = 0;

	// Total laps completed
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalLaps = 0;

	// Total distance driven (meters)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	float TotalDistance = 0.0f;

	// Total drift time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	float TotalDriftTime = 0.0f;

	// Total overtakes
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalOvertakes = 0;

	// Best lap time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	float BestLapTime = -1.0f;

	// Best lap track
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FString BestLapTrack;

	// Best lap vehicle
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FString BestLapVehicle;

	// Max speed achieved (km/h)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	float MaxSpeed = 0.0f;

	// Tracks completed
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	TSet<FString> CompletedTracks;

	// Vehicles used
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	TSet<FString> UsedVehicles;

	// Championship wins
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 ChampionshipWins = 0;

	// Total play time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	float TotalPlayTime = 0.0f;

	// Total clean races (no collisions)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalCleanRaces = 0;

	// Total collisions
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 TotalCollisions = 0;

	// Per-track statistics
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	TMap<FString, FTrackStatistics> TrackStats;

	// Per-vehicle statistics
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	TMap<FString, FVehicleStatistics> VehicleStats;

	// Recent session results (last N sessions)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	TArray<FRaceSessionResult> RecentSessions;

	// Consecutive wins streak
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 CurrentWinStreak = 0;

	// Best win streak
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 BestWinStreak = 0;

	// Total podium streak
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 CurrentPodiumStreak = 0;

	// Best podium streak (career best)
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	int32 BestPodiumStreak = 0;
};

/**
 * Unlockable item
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FUnlockableItem
{
	GENERATED_BODY()

	// Item ID
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	FString ID;

	// Item type
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	EUnlockableType Type = EUnlockableType::PaintJob;

	// Display name
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	FString DisplayName;

	// Description
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	FString Description;

	// Is unlocked?
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	bool bUnlocked = false;

	// Unlock condition text (human-readable)
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	FString UnlockCondition;

	// Achievement required (if unlock is achievement-based)
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	EAchievement RequiredAchievement = EAchievement::FirstRace;

	// Whether this unlock is achievement-based
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	bool bRequiresAchievement = false;

	// Stat-based unlock: required number of wins
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	int32 RequiredWins = 0;

	// Stat-based unlock: required number of races
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	int32 RequiredRaces = 0;

	// Stat-based unlock: required championship wins
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	int32 RequiredChampionshipWins = 0;

	// Rarity tier (0=Common, 1=Uncommon, 2=Rare, 3=Epic, 4=Legendary)
	UPROPERTY(BlueprintReadWrite, Category = "Unlock")
	int32 Rarity = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FAchievementData&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnlocked, const FUnlockableItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceSessionCompleted, const FRaceSessionResult&, SessionResult);
