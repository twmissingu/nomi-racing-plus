// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/ProgressionTest.h"
#include "Race/RaceProgression.h"

/**
 * Helper to create a race session result for testing
 */
static FRaceSessionResult CreateTestSessionResult(
	const FString& TrackName,
	const FString& VehicleName,
	int32 FinalPosition,
	int32 TotalRacers,
	float BestLapTime,
	float MaxSpeed,
	int32 Overtakes,
	int32 Collisions,
	bool bCleanRace)
{
	FRaceSessionResult Result;
	Result.TrackName = TrackName;
	Result.VehicleName = VehicleName;
	Result.FinalPosition = FinalPosition;
	Result.TotalRacers = TotalRacers;
	Result.NumLaps = 3;
	Result.BestLapTime = BestLapTime;
	Result.TotalRaceTime = BestLapTime * 3.0f;
	Result.MaxSpeed = MaxSpeed;
	Result.Overtakes = Overtakes;
	Result.Collisions = Collisions;
	Result.DistanceDriven = 12000.0f; // 12km
	Result.bCleanRace = bCleanRace;
	Result.RaceMode = TEXT("StreetGT");
	Result.Difficulty = 1;
	Result.Timestamp = FDateTime::Now();

	// Add some lap times
	Result.LapTimes.Add(BestLapTime);
	Result.LapTimes.Add(BestLapTime + 0.5f);
	Result.LapTimes.Add(BestLapTime + 1.0f);

	return Result;
}

/**
 * Test Achievement System
 */
bool FProgressionAchievementTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: Achievements should be initialized
	const TMap<EAchievement, FAchievementData>& Achievements = Progression->GetAchievements();
	TestTrue(TEXT("Achievements should be initialized"), Achievements.Num() > 0);

	// Test 2: First Race achievement should exist
	FAchievementData FirstRaceData;
	bool bFound = Progression->GetAchievement(EAchievement::FirstRace, FirstRaceData);
	TestTrue(TEXT("First Race achievement should exist"), bFound);
	TestEqual(TEXT("First Race display name"), FirstRaceData.DisplayName, FString(TEXT("First Race")));

	// Test 3: Achievement should start locked
	TestFalse(TEXT("First Race should start locked"), FirstRaceData.bUnlocked);

	// Test 4: Unlock achievement
	bool bUnlocked = Progression->UnlockAchievement(EAchievement::FirstRace);
	TestTrue(TEXT("Should successfully unlock First Race"), bUnlocked);

	// Test 5: Verify unlocked state
	Progression->GetAchievement(EAchievement::FirstRace, FirstRaceData);
	TestTrue(TEXT("First Race should be unlocked"), FirstRaceData.bUnlocked);
	TestEqual(TEXT("Progress should be 1.0"), FirstRaceData.Progress, 1.0f);

	// Test 6: Cannot unlock same achievement twice
	bUnlocked = Progression->UnlockAchievement(EAchievement::FirstRace);
	TestFalse(TEXT("Should not unlock already unlocked achievement"), bUnlocked);

	// Test 7: Update achievement progress
	Progression->UpdateAchievementProgress(EAchievement::ConsistentRacer, 5.0f);
	FAchievementData ConsistentRacerData;
	Progression->GetAchievement(EAchievement::ConsistentRacer, ConsistentRacerData);
	TestTrue(TEXT("Consistent Racer progress should be > 0"), ConsistentRacerData.Progress > 0.0f);
	TestFalse(TEXT("Consistent Racer should not be unlocked at 5/10"), ConsistentRacerData.bUnlocked);

	// Test 8: Full progress should unlock
	Progression->UpdateAchievementProgress(EAchievement::ConsistentRacer, 10.0f);
	Progression->GetAchievement(EAchievement::ConsistentRacer, ConsistentRacerData);
	TestTrue(TEXT("Consistent Racer should be unlocked at 10/10"), ConsistentRacerData.bUnlocked);

	// Test 9: Get unlocked count
	int32 UnlockedCount = Progression->GetUnlockedAchievementCount();
	TestTrue(TEXT("Should have at least 2 unlocked achievements"), UnlockedCount >= 2);

	// Test 10: Total achievement count
	int32 TotalCount = Progression->GetTotalAchievementCount();
	TestTrue(TEXT("Should have many achievements defined"), TotalCount >= 15);

	return true;
}

/**
 * Test Statistics System
 */
bool FProgressionStatisticsTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: Initial statistics should be zero
	const FPlayerStatistics& InitialStats = Progression->GetStatistics();
	TestEqual(TEXT("Initial total races should be 0"), InitialStats.TotalRaces, 0);
	TestEqual(TEXT("Initial total wins should be 0"), InitialStats.TotalWins, 0);

	// Test 2: Record a winning session
	FRaceSessionResult WinResult = CreateTestSessionResult(
		TEXT("NIOCityCircuit"), TEXT("EP9"), 1, 8, 45.0f, 280.0f, 10, 0, true);
	Progression->RecordRaceSession(WinResult);

	// Test 3: Verify statistics updated
	const FPlayerStatistics& Stats = Progression->GetStatistics();
	TestEqual(TEXT("Total races should be 1"), Stats.TotalRaces, 1);
	TestEqual(TEXT("Total wins should be 1"), Stats.TotalWins, 1);
	TestEqual(TEXT("Total podiums should be 1"), Stats.TotalPodiums, 1);
	TestEqual(TEXT("Total clean races should be 1"), Stats.TotalCleanRaces, 1);
	TestEqual(TEXT("Max speed should match"), Stats.MaxSpeed, 280.0f);

	// Test 4: Record a losing session
	FRaceSessionResult LoseResult = CreateTestSessionResult(
		TEXT("ShanghaiPudong"), TEXT("ET7"), 5, 8, 52.0f, 220.0f, 3, 2, false);
	Progression->RecordRaceSession(LoseResult);

	// Test 5: Verify cumulative statistics
	const FPlayerStatistics& CumulativeStats = Progression->GetStatistics();
	TestEqual(TEXT("Total races should be 2"), CumulativeStats.TotalRaces, 2);
	TestEqual(TEXT("Total wins should still be 1"), CumulativeStats.TotalWins, 1);
	TestEqual(TEXT("Total podiums should still be 1"), CumulativeStats.TotalPodiums, 1);
	TestEqual(TEXT("Total clean races should still be 1"), CumulativeStats.TotalCleanRaces, 1);
	TestEqual(TEXT("Total collisions should be 2"), CumulativeStats.TotalCollisions, 2);

	// Test 6: Best lap time tracking
	TestEqual(TEXT("Best lap time should be 45.0"), CumulativeStats.BestLapTime, 45.0f);
	TestEqual(TEXT("Best lap track"), CumulativeStats.BestLapTrack, FString(TEXT("NIOCityCircuit")));

	// Test 7: Completed tracks tracking
	TestTrue(TEXT("Should have NIOCityCircuit track"), CumulativeStats.CompletedTracks.Contains(TEXT("NIOCityCircuit")));
	TestTrue(TEXT("Should have ShanghaiPudong track"), CumulativeStats.CompletedTracks.Contains(TEXT("ShanghaiPudong")));

	// Test 8: Used vehicles tracking
	TestTrue(TEXT("Should have EP9 vehicle"), CumulativeStats.UsedVehicles.Contains(TEXT("EP9")));
	TestTrue(TEXT("Should have ET7 vehicle"), CumulativeStats.UsedVehicles.Contains(TEXT("ET7")));

	return true;
}

/**
 * Test Championship System
 */
bool FProgressionChampionshipTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: No active championship initially
	TestFalse(TEXT("Should not have active championship initially"), Progression->HasActiveChampionship());

	// Test 2: Create and start a championship
	FChampionshipData Championship;
	Championship.ID = TEXT("Test_Championship");
	Championship.Name = TEXT("Test Championship");
	Championship.Tier = EChampionshipTier::Beginner;
	Championship.Tracks = { TEXT("Track1"), TEXT("Track2"), TEXT("Track3") };
	Championship.PointsPerPosition = {
		{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
		{6, 8}, {7, 6}, {8, 4}
	};
	Championship.NumAIOpponents = 5;
	Championship.Difficulty = 0;

	// Add player to standings
	FChampionshipStandingEntry PlayerEntry;
	PlayerEntry.Name = TEXT("Player");
	PlayerEntry.bIsPlayer = true;
	Championship.Standings.Add(PlayerEntry);

	// Add AI to standings
	for (int32 i = 0; i < 5; i++)
	{
		FChampionshipStandingEntry AIEntry;
		AIEntry.Name = FString::Printf(TEXT("AI_%d"), i + 1);
		AIEntry.bIsPlayer = false;
		Championship.Standings.Add(AIEntry);
	}

	Progression->StartChampionship(Championship);
	TestTrue(TEXT("Should have active championship"), Progression->HasActiveChampionship());

	// Test 3: Get current championship
	FChampionshipData CurrentChamp;
	bool bHasChamp = Progression->GetCurrentChampionship(CurrentChamp);
	TestTrue(TEXT("Should get current championship"), bHasChamp);
	TestEqual(TEXT("Championship name should match"), CurrentChamp.Name, FString(TEXT("Test Championship")));

	// Test 4: Simulate Race 1 result (pre-calculated by ChampionshipManager)
	// Player got 25 pts (P1), AI_1 got 18 pts (P2), AI_2 15, AI_3 12, AI_4 10, AI_5 8
	FChampionshipData AfterRace1 = Championship;
	AfterRace1.PlayerPoints = 25;
	AfterRace1.AIOpponentPoints.Add(TEXT("AI_1"), 18);
	AfterRace1.AIOpponentPoints.Add(TEXT("AI_2"), 15);
	AfterRace1.AIOpponentPoints.Add(TEXT("AI_3"), 12);
	AfterRace1.AIOpponentPoints.Add(TEXT("AI_4"), 10);
	AfterRace1.AIOpponentPoints.Add(TEXT("AI_5"), 8);
	AfterRace1.CurrentRace = 1;
	AfterRace1.bComplete = false;
	FChampionshipStandingEntry P1;
	P1.Name = TEXT("Player"); P1.bIsPlayer = true; P1.Points = 25; P1.Positions.Add(1); P1.Wins = 1; P1.Podiums = 1;
	FChampionshipStandingEntry A1;
	A1.Name = TEXT("AI_1"); A1.bIsPlayer = false; A1.Points = 18; A1.Positions.Add(2); A1.Podiums = 1;
	FChampionshipStandingEntry A2;
	A2.Name = TEXT("AI_2"); A2.bIsPlayer = false; A2.Points = 15; A2.Positions.Add(3); A2.Podiums = 1;
	FChampionshipStandingEntry A3;
	A3.Name = TEXT("AI_3"); A3.bIsPlayer = false; A3.Points = 12; A3.Positions.Add(4);
	FChampionshipStandingEntry A4;
	A4.Name = TEXT("AI_4"); A4.bIsPlayer = false; A4.Points = 10; A4.Positions.Add(5);
	FChampionshipStandingEntry A5;
	A5.Name = TEXT("AI_5"); A5.bIsPlayer = false; A5.Points = 8; A5.Positions.Add(6);
	AfterRace1.Standings = { P1, A1, A2, A3, A4, A5 };

	Progression->UpdateChampionshipResults(AfterRace1);

	// Verify championship not yet complete
	Progression->GetCurrentChampionship(CurrentChamp);
	TestFalse(TEXT("Championship should not be complete after 1 race"), CurrentChamp.bComplete);
	TestEqual(TEXT("Should be on race 2 (CurrentRace=1)"), CurrentChamp.CurrentRace, 1);

	// Test 5: Simulate Race 2 (player P2, AI_1 P1)
	FChampionshipData AfterRace2 = AfterRace1;
	AfterRace2.PlayerPoints = 43; // 25 + 18
	AfterRace2.AIOpponentPoints[TEXT("AI_1")] = 43; // 18 + 25
	AfterRace2.CurrentRace = 2;
	AfterRace2.Standings[0].Points = 43; AfterRace2.Standings[0].Positions.Add(2); AfterRace2.Standings[0].Podiums = 2;
	AfterRace2.Standings[1].Points = 43; AfterRace2.Standings[1].Positions.Add(1); AfterRace2.Standings[1].Wins = 1; AfterRace2.Standings[1].Podiums = 1;
	Progression->UpdateChampionshipResults(AfterRace2);

	// Test 6: Simulate Race 3 (player P1, AI_1 P2) — player wins championship
	FChampionshipData AfterRace3 = AfterRace2;
	AfterRace3.PlayerPoints = 68; // 43 + 25
	AfterRace3.AIOpponentPoints[TEXT("AI_1")] = 61; // 43 + 18
	AfterRace3.CurrentRace = 3;
	AfterRace3.bComplete = true;
	AfterRace3.bPlayerWon = true;
	AfterRace3.FinalPlayerPosition = 1;
	AfterRace3.CompletionTime = FDateTime::Now();
	AfterRace3.Standings[0].Points = 68; AfterRace3.Standings[0].Positions.Add(1);
	AfterRace3.Standings[0].Wins = 2; AfterRace3.Standings[0].Podiums = 3;
	AfterRace3.Standings[1].Points = 61; AfterRace3.Standings[1].Positions.Add(2);
	AfterRace3.Standings[1].Podiums = 2;
	Progression->UpdateChampionshipResults(AfterRace3);

	// Test 7: Championship should be complete
	Progression->GetCurrentChampionship(CurrentChamp);
	TestTrue(TEXT("Championship should be complete"), CurrentChamp.bComplete);

	// Test 8: Player should have won (68 vs AI_1 61)
	TestTrue(TEXT("Player should have won championship"), CurrentChamp.bPlayerWon);

	// Test 9: Championship wins count
	TestEqual(TEXT("Should have 1 championship win"), Progression->GetChampionshipWinCount(), 1);

	// Test 10: Championship history
	const TArray<FChampionshipHistoryEntry>& History = Progression->GetChampionshipHistory();
	TestEqual(TEXT("Should have 1 history entry"), History.Num(), 1);

	return true;
}

/**
 * Test Unlockable System
 */
bool FProgressionUnlockablesTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: Unlockables should be initialized
	const TMap<FString, FUnlockableItem>& Unlockables = Progression->GetUnlockables();
	TestTrue(TEXT("Unlockables should be initialized"), Unlockables.Num() > 0);

	// Test 2: All items start locked
	for (const auto& Pair : Unlockables)
	{
		TestFalse(FString::Printf(TEXT("%s should start locked"), *Pair.Key), Pair.Value.bUnlocked);
	}

	// Test 3: Unlock a specific item
	bool bUnlocked = Progression->UnlockItem(TEXT("EP9_Paint_Silver"));
	TestTrue(TEXT("Should successfully unlock EP9_Paint_Silver"), bUnlocked);
	TestTrue(TEXT("EP9_Paint_Silver should be unlocked"), Progression->IsItemUnlocked(TEXT("EP9_Paint_Silver")));

	// Test 4: Cannot unlock same item twice
	bUnlocked = Progression->UnlockItem(TEXT("EP9_Paint_Silver"));
	TestFalse(TEXT("Should not unlock already unlocked item"), bUnlocked);

	// Test 5: Invalid item ID
	bUnlocked = Progression->UnlockItem(TEXT("NonExistentItem"));
	TestFalse(TEXT("Should not unlock non-existent item"), bUnlocked);

	// Test 6: Get unlockables by type
	TArray<FUnlockableItem> PaintJobs = Progression->GetUnlockablesByType(EUnlockableType::PaintJob);
	TestTrue(TEXT("Should have paint job unlockables"), PaintJobs.Num() > 0);

	TArray<FUnlockableItem> Wheels = Progression->GetUnlockablesByType(EUnlockableType::WheelDesign);
	TestTrue(TEXT("Should have wheel unlockables"), Wheels.Num() > 0);

	// Test 7: Unlock progress
	float Progress = Progression->GetUnlockProgress();
	TestTrue(TEXT("Unlock progress should be > 0 after unlocking"), Progress > 0.0f);

	return true;
}

/**
 * Test Session Recording
 */
bool FProgressionSessionRecordingTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: Record multiple sessions
	for (int32 i = 0; i < 5; i++)
	{
		FRaceSessionResult Result = CreateTestSessionResult(
			TEXT("NIOCityCircuit"), TEXT("EP9"),
			i + 1, 8, 45.0f + i, 280.0f - i * 5, 10 - i, i, i == 0);
		Progression->RecordRaceSession(Result);
	}

	// Test 2: Verify session count
	const TArray<FRaceSessionResult>& Sessions = Progression->GetRecentSessions();
	TestEqual(TEXT("Should have 5 recent sessions"), Sessions.Num(), 5);

	// Test 3: Verify latest session
	const FRaceSessionResult& LatestSession = Sessions.Last();
	TestEqual(TEXT("Latest session position should be 5"), LatestSession.FinalPosition, 5);

	// Test 4: Verify statistics accumulated correctly
	const FPlayerStatistics& Stats = Progression->GetStatistics();
	TestEqual(TEXT("Total races should be 5"), Stats.TotalRaces, 5);
	TestEqual(TEXT("Total wins should be 1"), Stats.TotalWins, 1);
	TestEqual(TEXT("Total podiums should be 3"), Stats.TotalPodiums, 3); // positions 1, 2, 3
	TestEqual(TEXT("Total clean races should be 1"), Stats.TotalCleanRaces, 1);

	return true;
}

/**
 * Test Streak Tracking
 */
bool FProgressionStreakTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Test 1: Win streak
	for (int32 i = 0; i < 3; i++)
	{
		FRaceSessionResult Result = CreateTestSessionResult(
			TEXT("NIOCityCircuit"), TEXT("EP9"), 1, 8, 45.0f, 280.0f, 10, 0, true);
		Progression->RecordRaceSession(Result);
	}

	const FPlayerStatistics& Stats = Progression->GetStatistics();
	TestEqual(TEXT("Win streak should be 3"), Stats.CurrentWinStreak, 3);
	TestEqual(TEXT("Best win streak should be 3"), Stats.BestWinStreak, 3);

	// Test 2: Break the streak
	FRaceSessionResult LossResult = CreateTestSessionResult(
		TEXT("ShanghaiPudong"), TEXT("ET7"), 5, 8, 52.0f, 220.0f, 3, 1, false);
	Progression->RecordRaceSession(LossResult);

	const FPlayerStatistics& StatsAfterLoss = Progression->GetStatistics();
	TestEqual(TEXT("Win streak should reset to 0"), StatsAfterLoss.CurrentWinStreak, 0);
	TestEqual(TEXT("Best win streak should still be 3"), StatsAfterLoss.BestWinStreak, 3);

	// Test 3: Build a new streak
	for (int32 i = 0; i < 5; i++)
	{
		FRaceSessionResult Result = CreateTestSessionResult(
			TEXT("NIOCityCircuit"), TEXT("EP9"), 1, 8, 45.0f, 280.0f, 10, 0, true);
		Progression->RecordRaceSession(Result);
	}

	const FPlayerStatistics& StatsNewStreak = Progression->GetStatistics();
	TestEqual(TEXT("Win streak should be 5"), StatsNewStreak.CurrentWinStreak, 5);
	TestEqual(TEXT("Best win streak should be 5"), StatsNewStreak.BestWinStreak, 5);

	// Test 4: Podium streak
	const FPlayerStatistics& FinalStats = Progression->GetStatistics();
	TestTrue(TEXT("Podium streak should be positive"), FinalStats.CurrentPodiumStreak > 0);

	return true;
}

/**
 * Test Per-Track Statistics
 */
bool FProgressionTrackStatsTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Record sessions on different tracks
	FRaceSessionResult Track1Result = CreateTestSessionResult(
		TEXT("NIOCityCircuit"), TEXT("EP9"), 1, 8, 45.0f, 280.0f, 10, 0, true);
	Progression->RecordRaceSession(Track1Result);

	FRaceSessionResult Track2Result = CreateTestSessionResult(
		TEXT("ShanghaiPudong"), TEXT("ET7"), 3, 8, 52.0f, 220.0f, 5, 2, false);
	Progression->RecordRaceSession(Track2Result);

	// Record another session on Track1
	FRaceSessionResult Track1Result2 = CreateTestSessionResult(
		TEXT("NIOCityCircuit"), TEXT("EP9"), 2, 8, 44.0f, 285.0f, 8, 0, true);
	Progression->RecordRaceSession(Track1Result2);

	// Test per-track statistics
	FTrackStatistics Track1Stats;
	bool bFound = Progression->GetTrackStatistics(TEXT("NIOCityCircuit"), Track1Stats);
	TestTrue(TEXT("Should find NIOCityCircuit stats"), bFound);
	TestEqual(TEXT("NIOCityCircuit races should be 2"), Track1Stats.RacesCompleted, 2);
	TestEqual(TEXT("NIOCityCircuit wins should be 1"), Track1Stats.Wins, 1);
	TestEqual(TEXT("NIOCityCircuit best lap should be 44.0"), Track1Stats.BestLapTime, 44.0f);
	TestEqual(TEXT("NIOCityCircuit best position should be 1"), Track1Stats.BestPosition, 1);

	FTrackStatistics Track2Stats;
	bFound = Progression->GetTrackStatistics(TEXT("ShanghaiPudong"), Track2Stats);
	TestTrue(TEXT("Should find ShanghaiPudong stats"), bFound);
	TestEqual(TEXT("ShanghaiPudong races should be 1"), Track2Stats.RacesCompleted, 1);
	TestEqual(TEXT("ShanghaiPudong wins should be 0"), Track2Stats.Wins, 0);
	TestEqual(TEXT("ShanghaiPudong best position should be 3"), Track2Stats.BestPosition, 3);

	// Non-existent track
	FTrackStatistics MissingStats;
	bFound = Progression->GetTrackStatistics(TEXT("MissingTrack"), MissingStats);
	TestFalse(TEXT("Should not find missing track stats"), bFound);

	return true;
}

/**
 * Test Per-Vehicle Statistics
 */
bool FProgressionVehicleStatsTest::RunTest(const FString& Parameters)
{
	URaceProgression* Progression = NewObject<URaceProgression>();
	TestNotNull(TEXT("Progression should be created"), Progression);

	if (!Progression) return false;

	// Record sessions with different vehicles
	FRaceSessionResult EP9Result = CreateTestSessionResult(
		TEXT("NIOCityCircuit"), TEXT("EP9"), 1, 8, 45.0f, 300.0f, 10, 0, true);
	Progression->RecordRaceSession(EP9Result);

	FRaceSessionResult ET7Result = CreateTestSessionResult(
		TEXT("ShanghaiPudong"), TEXT("ET7"), 3, 8, 52.0f, 240.0f, 5, 2, false);
	Progression->RecordRaceSession(ET7Result);

	// Another EP9 session
	FRaceSessionResult EP9Result2 = CreateTestSessionResult(
		TEXT("SpeedwayOval"), TEXT("EP9"), 2, 8, 43.0f, 310.0f, 8, 1, false);
	Progression->RecordRaceSession(EP9Result2);

	// Test per-vehicle statistics
	FVehicleStatistics EP9Stats;
	bool bFound = Progression->GetVehicleStatistics(TEXT("EP9"), EP9Stats);
	TestTrue(TEXT("Should find EP9 stats"), bFound);
	TestEqual(TEXT("EP9 races should be 2"), EP9Stats.RacesCompleted, 2);
	TestEqual(TEXT("EP9 wins should be 1"), EP9Stats.Wins, 1);
	TestEqual(TEXT("EP9 best lap should be 43.0"), EP9Stats.BestLapTime, 43.0f);
	TestEqual(TEXT("EP9 max speed should be 310.0"), EP9Stats.MaxSpeed, 310.0f);

	FVehicleStatistics ET7Stats;
	bFound = Progression->GetVehicleStatistics(TEXT("ET7"), ET7Stats);
	TestTrue(TEXT("Should find ET7 stats"), bFound);
	TestEqual(TEXT("ET7 races should be 1"), ET7Stats.RacesCompleted, 1);
	TestEqual(TEXT("ET7 wins should be 0"), ET7Stats.Wins, 0);

	// Non-existent vehicle
	FVehicleStatistics MissingStats;
	bFound = Progression->GetVehicleStatistics(TEXT("MissingVehicle"), MissingStats);
	TestFalse(TEXT("Should not find missing vehicle stats"), bFound);

	return true;
}
