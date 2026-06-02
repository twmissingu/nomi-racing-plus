// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RaceProgression.generated.h"

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
};

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChampionshipCompleted, const FChampionshipData&, Championship, bool, bPlayerWon);

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
	UFUNCTION(BlueprintCallable, Category = "Statistics")
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
