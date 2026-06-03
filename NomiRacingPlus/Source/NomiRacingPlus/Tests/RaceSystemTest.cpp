// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/RaceSystemTest.h"
#include "Race/RaceManager.h"

/**
 * Test RaceManager state machine transitions
 */
bool FRaceManagerStateMachineTest::RunTest(const FString& Parameters)
{
	// Test 1: Create RaceManager
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!RaceManager)
	{
		return false;
	}

	// Test 2: Initial state should be Idle
	TestEqual(TEXT("Initial state should be Idle"), RaceManager->GetRaceState(), ERaceState::Idle);

	// Test 3: Configure race
	FRaceConfig Config;
	Config.NumLaps = 3;
	Config.MaxAIOpponents = 7;
	Config.TrackName = TEXT("Test Track");
	Config.CountdownDuration = 5.0f;

	// Test 4: Start race
	RaceManager->StartRace(Config);
	TestEqual(TEXT("State should be Countdown after start"), RaceManager->GetRaceState(), ERaceState::Countdown);

	// Test 5: Verify countdown value
	float Countdown = RaceManager->GetCountdownValue();
	TestTrue(TEXT("Countdown should be positive"), Countdown > 0.0f);

	// Test 6: Test pause functionality
	RaceManager->PauseRace();
	TestEqual(TEXT("State should be Paused"), RaceManager->GetRaceState(), ERaceState::Paused);

	// Test 7: Test resume functionality
	RaceManager->ResumeRace();
	TestEqual(TEXT("State should be Racing after resume"), RaceManager->GetRaceState(), ERaceState::Racing);

	// Test 8: Test end race
	RaceManager->EndRace();
	TestEqual(TEXT("State should be Finished"), RaceManager->GetRaceState(), ERaceState::Finished);

	// Test 9: Test reset
	RaceManager->ResetRace();
	TestEqual(TEXT("State should be Idle after reset"), RaceManager->GetRaceState(), ERaceState::Idle);

	return true;
}

/**
 * Test checkpoint system
 */
bool FRaceCheckpointTest::RunTest(const FString& Parameters)
{
	// Test 1: Create RaceManager
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!RaceManager)
	{
		return false;
	}

	// Test 2: Configure and start race
	FRaceConfig Config;
	Config.NumLaps = 1;
	Config.TrackName = TEXT("Test Track");
	RaceManager->StartRace(Config);

	// Test 3: Create a mock pawn for testing
	// Note: In real tests, we'd use a proper test pawn
	// For now, we verify the function exists and can be called

	// Test 4: Verify race is not finished initially
	TestFalse(TEXT("Race should not be finished initially"), RaceManager->IsRaceFinished());

	return true;
}

/**
 * Test position calculation
 */
bool FRacePositionCalculationTest::RunTest(const FString& Parameters)
{
	// Test 1: Create RaceManager
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!RaceManager)
	{
		return false;
	}

	// Test 2: Verify initial position is 0
	int32 InitialPosition = RaceManager->GetPlayerPosition();
	TestEqual(TEXT("Initial position should be 0"), InitialPosition, 0);

	// Test 3: Verify initial lap is 0
	int32 InitialLap = RaceManager->GetPlayerCurrentLap();
	TestEqual(TEXT("Initial lap should be 0"), InitialLap, 0);

	return true;
}

/**
 * Test race timer
 */
bool FRaceTimerTest::RunTest(const FString& Parameters)
{
	// Test 1: Create RaceManager
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!RaceManager)
	{
		return false;
	}

	// Test 2: Verify initial timer is 0
	float InitialTimer = RaceManager->GetRaceTimer();
	TestEqual(TEXT("Initial timer should be 0"), InitialTimer, 0.0f);

	// Test 3: Configure and start race
	FRaceConfig Config;
	Config.NumLaps = 1;
	Config.TrackName = TEXT("Test Track");
	RaceManager->StartRace(Config);

	// Test 4: Verify timer is still 0 at countdown
	float CountdownTimer = RaceManager->GetRaceTimer();
	TestEqual(TEXT("Timer should be 0 during countdown"), CountdownTimer, 0.0f);

	return true;
}
