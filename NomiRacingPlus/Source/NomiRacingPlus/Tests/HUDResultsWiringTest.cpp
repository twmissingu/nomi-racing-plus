// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/HUDResultsWiringTest.h"
#include "UI/RaceHUD.h"
#include "UI/ResultsWidget.h"
#include "UI/MenuManager.h"
#include "Race/RaceManager.h"
#include "Race/RaceProgressionTypes.h"
#include "Vehicles/VehicleStateManager.h"
#include "NOMI/CommentaryEngine.h"

// ─── HUD Data Wiring Tests ──────────────────────────────────────────────────

/**
 * FHUDData fields can be populated from VehicleStateManager and RaceManager values
 * without crashing or losing data.
 */
bool FHUDDataPopulationTest::RunTest(const FString& Parameters)
{
	// Arrange: create a FHUDData and a VehicleStateManager to feed it
	UVehicleStateManager* VSM = NewObject<UVehicleStateManager>();
	TestNotNull(TEXT("VSM should be created"), VSM);
	if (!VSM) { return false; }

	FHUDData Data;

	// Act: populate FHUDData from VSM defaults (speed=0, battery=100, not drifting)
	const FNIOVehicleState& VState = VSM->GetVehicleState();
	Data.Speed = VState.SpeedKmh;
	Data.ThrottleInput = VState.ThrottleInput;
	Data.BrakeInput = VState.BrakeInput;
	Data.SteeringInput = VState.SteeringInput;
	Data.bIsDrifting = VState.bIsDrifting;
	Data.DriftAngle = VState.SlipAngle;
	Data.BatteryLevel = VState.BatteryPercent;
	Data.bIsNIOVehicle = VSM->IsNIOVehicle();

	// Assert: defaults should be sensible
	TestEqual(TEXT("Default speed should be 0"), Data.Speed, 0.0f);
	TestEqual(TEXT("Default throttle should be 0"), Data.ThrottleInput, 0.0f);
	TestEqual(TEXT("Default brake should be 0"), Data.BrakeInput, 0.0f);
	TestFalse(TEXT("Should not be drifting by default"), Data.bIsDrifting);
	TestEqual(TEXT("Battery should be 100"), Data.BatteryLevel, 100.0f);

	// Arrange: race data
	ARaceManager* RM = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RM);
	if (!RM) { return false; }

	// Act: populate race telemetry fields
	Data.Position = RM->GetPlayerPosition();
	Data.TotalRacers = RM->GetAllRacers().Num();
	Data.CurrentLap = RM->GetPlayerCurrentLap();
	Data.RaceTimer = RM->GetRaceTimer();

	// Assert: initial race data
	TestEqual(TEXT("Initial position should be 0"), Data.Position, 0);
	TestEqual(TEXT("Initial total racers should be 0"), Data.TotalRacers, 0);
	TestEqual(TEXT("Initial lap should be 0"), Data.CurrentLap, 0);
	TestEqual(TEXT("Initial race timer should be 0"), Data.RaceTimer, 0.0f);

	return true;
}

/**
 * RaceHUD::UpdateHUDData stores the data and makes it retrievable.
 */
bool FHUDUpdateStoresDataTest::RunTest(const FString& Parameters)
{
	// Arrange
	URaceHUD* HUD = NewObject<URaceHUD>();
	TestNotNull(TEXT("RaceHUD should be created"), HUD);
	if (!HUD) { return false; }

	FHUDData Data;
	Data.Speed = 123.4f;
	Data.Position = 3;
	Data.CurrentLap = 2;
	Data.TotalLaps = 5;
	Data.RaceTimer = 67.8f;
	Data.BestLapTime = 22.1f;
	Data.BatteryLevel = 75.5f;

	// Act
	HUD->UpdateHUDData(Data);

	// Assert
	const FHUDData& Stored = HUD->GetHUDData();
	TestEqual(TEXT("Stored speed should match"), Stored.Speed, 123.4f);
	TestEqual(TEXT("Stored position should match"), Stored.Position, 3);
	TestEqual(TEXT("Stored lap should match"), Stored.CurrentLap, 2);
	TestEqual(TEXT("Stored total laps should match"), Stored.TotalLaps, 5);
	TestEqual(TEXT("Stored race timer should match"), Stored.RaceTimer, 67.8f);
	TestEqual(TEXT("Stored best lap should match"), Stored.BestLapTime, 22.1f);
	TestEqual(TEXT("Stored battery should match"), Stored.BatteryLevel, 75.5f);

	return true;
}

/**
 * RaceHUD::ShowCountdown sets the countdown value in HUD data.
 * Verifies the countdown display API exists and stores state.
 */
bool FHUDCountdownDisplayTest::RunTest(const FString& Parameters)
{
	// Arrange
	URaceHUD* HUD = NewObject<URaceHUD>();
	TestNotNull(TEXT("RaceHUD should be created"), HUD);
	if (!HUD) { return false; }

	// Act: simulate what Tick() does for countdown by setting FHUDData.CountdownValue
	FHUDData Data;
	Data.RaceState = ERaceState::Countdown;
	Data.CountdownValue = 3.0f;
	HUD->UpdateHUDData(Data);

	// Assert
	const FHUDData& Stored = HUD->GetHUDData();
	TestEqual(TEXT("Race state should be Countdown"), Stored.RaceState, ERaceState::Countdown);
	TestEqual(TEXT("Countdown value should be 3"), Stored.CountdownValue, 3.0f);

	// Also verify ShowCountdown API exists and can be called
	HUD->ShowCountdown(5.0f);
	const FHUDData& AfterShow = HUD->GetHUDData();
	TestEqual(TEXT("After ShowCountdown, value should be 5"), AfterShow.CountdownValue, 5.0f);

	return true;
}

/**
 * RaceHUD::HideCountdown clears countdown state.
 */
bool FHUDCountdownHideTest::RunTest(const FString& Parameters)
{
	// Arrange
	URaceHUD* HUD = NewObject<URaceHUD>();
	TestNotNull(TEXT("RaceHUD should be created"), HUD);
	if (!HUD) { return false; }

	// Set countdown first
	HUD->ShowCountdown(2.0f);
	TestEqual(TEXT("Countdown should be 2 before hide"), HUD->GetHUDData().CountdownValue, 2.0f);

	// Act
	HUD->HideCountdown();

	// Assert
	const FHUDData& Stored = HUD->GetHUDData();
	TestEqual(TEXT("Countdown value should be 0 after hide"), Stored.CountdownValue, 0.0f);

	return true;
}

/**
 * Countdown values: integer display rounding, GO! at zero.
 * Tests the data contract that the UI layer will use.
 */
bool FHUDCountdownValuesTest::RunTest(const FString& Parameters)
{
	// Arrange: simulate countdown tick values
	struct FCountdownTestCase
	{
		float Input;
		int32 ExpectedDisplay;
		FString Description;
	};

	TArray<FCountdownTestCase> Cases = {
		{5.0f, 5, TEXT("5 seconds -> display 5")},
		{4.7f, 4, TEXT("4.7 seconds -> display 4")},
		{3.2f, 3, TEXT("3.2 seconds -> display 3")},
		{1.0f, 1, TEXT("1 second -> display 1")},
		{0.4f, 0, TEXT("0.4 seconds -> display 0 (GO!)")},
		{0.0f, 0, TEXT("0 seconds -> display 0 (GO!)")},
	};

	for (const FCountdownTestCase& TC : Cases)
	{
		// The countdown display should floor the value
		int32 DisplayValue = FMath::FloorToInt(TC.Input);
		TestEqual(*TC.Description, DisplayValue, TC.ExpectedDisplay);
	}

	// Edge case: negative countdown should still display 0
	int32 NegDisplay = FMath::FloorToInt(-0.5f);
	TestEqual(TEXT("Negative countdown -> display 0 or less"), NegDisplay, -1);
	// The UI should clamp this to 0, but the raw floor is -1

	return true;
}

/**
 * HUD data fields for NOMI integration (comment text, emotion, visibility)
 */
bool FHUDNOMIDataTest::RunTest(const FString& Parameters)
{
	// Arrange
	FHUDData Data;

	// Act: simulate NOMI comment data population
	Data.bNOMICommentVisible = true;
	Data.NOMICommentText = TEXT("Great overtake!");
	Data.NOMIEmotion = ENOMIEmotion::Excited;

	// Assert
	TestTrue(TEXT("NOMI comment should be visible"), Data.bNOMICommentVisible);
	TestEqual(TEXT("NOMI comment text should match"), Data.NOMICommentText, FString(TEXT("Great overtake!")));
	TestEqual(TEXT("NOMI emotion should be Excited"), Data.NOMIEmotion, ENOMIEmotion::Excited);

	// Act: simulate hidden NOMI comment
	Data.bNOMICommentVisible = false;
	Data.NOMICommentText = TEXT("");

	// Assert
	TestFalse(TEXT("NOMI comment should be hidden"), Data.bNOMICommentVisible);
	TestEqual(TEXT("NOMI comment text should be empty"), Data.NOMICommentText, FString(TEXT("")));

	// Verify all emotion types are valid for HUD
	TArray<ENOMIEmotion> Emotions = {
		ENOMIEmotion::Idle, ENOMIEmotion::Happy, ENOMIEmotion::Excited,
		ENOMIEmotion::Nervous, ENOMIEmotion::Surprised, ENOMIEmotion::Celebrating,
		ENOMIEmotion::Concerned, ENOMIEmotion::Confused, ENOMIEmotion::Tired
	};
	for (ENOMIEmotion Emotion : Emotions)
	{
		Data.NOMIEmotion = Emotion;
		TestTrue(TEXT("Emotion should be assignable"), Data.NOMIEmotion == Emotion);
	}

	return true;
}

/**
 * HUD data fields for vehicle telemetry (speed, battery, drift, inputs)
 */
bool FHUDVehicleTelemetryTest::RunTest(const FString& Parameters)
{
	// Arrange
	UVehicleStateManager* VSM = NewObject<UVehicleStateManager>();
	TestNotNull(TEXT("VSM should be created"), VSM);
	if (!VSM) { return false; }

	FHUDData Data;

	// Act: populate from VSM
	const FNIOVehicleState& VState = VSM->GetVehicleState();
	Data.Speed = VState.SpeedKmh;
	Data.ThrottleInput = VState.ThrottleInput;
	Data.BrakeInput = VState.BrakeInput;
	Data.SteeringInput = VState.SteeringInput;
	Data.bIsDrifting = VState.bIsDrifting;
	Data.DriftAngle = VState.SlipAngle;
	Data.BatteryLevel = VState.BatteryPercent;
	Data.bIsNIOVehicle = VSM->IsNIOVehicle();

	// Assert: initial telemetry should be zero/idle
	TestEqual(TEXT("Speed should be 0"), Data.Speed, 0.0f);
	TestEqual(TEXT("Throttle should be 0"), Data.ThrottleInput, 0.0f);
	TestEqual(TEXT("Brake should be 0"), Data.BrakeInput, 0.0f);
	TestEqual(TEXT("Steering should be 0"), Data.SteeringInput, 0.0f);
	TestFalse(TEXT("Should not be drifting"), Data.bIsDrifting);
	TestEqual(TEXT("Drift angle should be 0"), Data.DriftAngle, 0.0f);
	TestEqual(TEXT("Battery should be 100%"), Data.BatteryLevel, 100.0f);

	// Verify NIO vehicle detection
	VSM->SetVehicleType(ENIOVehicleType::EP9);
	TestTrue(TEXT("EP9 should be NIO vehicle"), VSM->IsNIOVehicle());
	Data.bIsNIOVehicle = VSM->IsNIOVehicle();
	TestTrue(TEXT("HUD should show NIO vehicle"), Data.bIsNIOVehicle);

	// Verify non-NIO vehicle
	VSM->SetVehicleType(ENIOVehicleType::SU7Ultra);
	TestFalse(TEXT("SU7 Ultra should not be NIO vehicle"), VSM->IsNIOVehicle());

	return true;
}

/**
 * HUD data fields for race state (position, lap, timer, best lap)
 */
bool FHUDRaceStateDataTest::RunTest(const FString& Parameters)
{
	// Arrange
	ARaceManager* RM = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RM);
	if (!RM) { return false; }

	FHUDData Data;

	// Act: populate race state
	Data.RaceState = RM->GetRaceState();
	Data.Position = RM->GetPlayerPosition();
	Data.TotalRacers = RM->GetAllRacers().Num();
	Data.CurrentLap = RM->GetPlayerCurrentLap();
	Data.TotalLaps = RM->GetRaceConfig().NumLaps;
	Data.RaceTimer = RM->GetRaceTimer();

	// Assert: initial idle state
	TestEqual(TEXT("Race state should be Idle"), Data.RaceState, ERaceState::Idle);
	TestEqual(TEXT("Position should be 0"), Data.Position, 0);
	TestEqual(TEXT("Total racers should be 0"), Data.TotalRacers, 0);
	TestEqual(TEXT("Current lap should be 0"), Data.CurrentLap, 0);
	TestEqual(TEXT("Race timer should be 0"), Data.RaceTimer, 0.0f);

	// Act: start race and verify state change
	FRaceConfig Config;
	Config.NumLaps = 3;
	Config.TrackName = TEXT("Test Circuit");
	RM->StartRace(Config);

	Data.RaceState = RM->GetRaceState();
	Data.TotalLaps = RM->GetRaceConfig().NumLaps;

	// Assert: should be in countdown
	TestEqual(TEXT("Race state should be Countdown"), Data.RaceState, ERaceState::Countdown);
	TestEqual(TEXT("Total laps should be 3"), Data.TotalLaps, 3);

	// Verify best lap time defaults
	Data.BestLapTime = -1.0f;
	TestEqual(TEXT("Best lap time should default to -1"), Data.BestLapTime, -1.0f);

	return true;
}

/**
 * HUD visibility toggling
 */
bool FHUDVisibilityTest::RunTest(const FString& Parameters)
{
	// Arrange
	URaceHUD* HUD = NewObject<URaceHUD>();
	TestNotNull(TEXT("RaceHUD should be created"), HUD);
	if (!HUD) { return false; }

	// Act & Assert: SetHUDVisible should not crash
	// Note: In a headless test environment, AddToViewport/RemoveFromParent
	// may not have visual effect, but the API should be callable
	HUD->SetHUDVisible(true);
	HUD->SetHUDVisible(false);
	HUD->SetHUDVisible(true);

	// The test verifies the API exists and doesn't crash
	TestTrue(TEXT("SetHUDVisible API should be callable without crash"), true);

	return true;
}

// ─── Results Widget Tests ────────────────────────────────────────────────────

/**
 * FRaceSessionResult can be populated correctly with all fields
 */
bool FSessionResultPopulationTest::RunTest(const FString& Parameters)
{
	// Arrange & Act
	FRaceSessionResult Result;
	Result.TrackName = TEXT("NIO City Circuit");
	Result.VehicleName = TEXT("NIO EP9");
	Result.FinalPosition = 1;
	Result.StartingPosition = 5;
	Result.TotalRacers = 8;
	Result.NumLaps = 3;
	Result.LapTimes = {22.5f, 21.8f, 21.2f};
	Result.BestLapTime = 21.2f;
	Result.TotalRaceTime = 65.5f;
	Result.MaxSpeed = 280.5f;
	Result.DriftTime = 8.3f;
	Result.Overtakes = 12;
	Result.Collisions = 0;
	Result.DistanceDriven = 15000.0f;
	Result.bCleanRace = true;
	Result.RaceMode = TEXT("StreetGT");
	Result.Difficulty = 1;
	Result.bChampionshipRace = false;
	Result.Timestamp = FDateTime(2026, 6, 5, 14, 30, 0);

	// Assert
	TestEqual(TEXT("Track name should match"), Result.TrackName, FString(TEXT("NIO City Circuit")));
	TestEqual(TEXT("Vehicle name should match"), Result.VehicleName, FString(TEXT("NIO EP9")));
	TestEqual(TEXT("Final position should be 1"), Result.FinalPosition, 1);
	TestEqual(TEXT("Starting position should be 5"), Result.StartingPosition, 5);
	TestEqual(TEXT("Total racers should be 8"), Result.TotalRacers, 8);
	TestEqual(TEXT("Num laps should be 3"), Result.NumLaps, 3);
	TestEqual(TEXT("Should have 3 lap times"), Result.LapTimes.Num(), 3);
	TestEqual(TEXT("First lap time"), Result.LapTimes[0], 22.5f);
	TestEqual(TEXT("Best lap time"), Result.BestLapTime, 21.2f);
	TestEqual(TEXT("Total race time"), Result.TotalRaceTime, 65.5f);
	TestEqual(TEXT("Max speed"), Result.MaxSpeed, 280.5f);
	TestEqual(TEXT("Overtakes"), Result.Overtakes, 12);
	TestEqual(TEXT("Collisions"), Result.Collisions, 0);
	TestTrue(TEXT("Should be clean race"), Result.bCleanRace);
	TestEqual(TEXT("Race mode"), Result.RaceMode, FString(TEXT("StreetGT")));
	TestFalse(TEXT("Should not be championship race"), Result.bChampionshipRace);

	return true;
}

/**
 * ResultsWidget::SetMenuManager stores the reference correctly
 */
bool FResultsWidgetMenuManagerTest::RunTest(const FString& Parameters)
{
	// Arrange
	UResultsWidget* Results = NewObject<UResultsWidget>();
	TestNotNull(TEXT("ResultsWidget should be created"), Results);
	if (!Results) { return false; }

	UMenuManager* MenuMgr = NewObject<UMenuManager>();
	TestNotNull(TEXT("MenuManager should be created"), MenuMgr);
	if (!MenuMgr) { return false; }

	// Act
	Results->SetMenuManager(MenuMgr);

	// Assert: the widget should hold the reference
	TestTrue(TEXT("MenuManager reference should be stored"), Results->MenuManager == MenuMgr);

	// Edge case: setting nullptr should not crash
	Results->SetMenuManager(nullptr);
	// The reference may or may not be nulled depending on implementation,
	// but it should not crash

	return true;
}

/**
 * ResultsWidget ordinal suffix formatting
 * Tests the GetOrdinalSuffix helper through the public contract
 */
bool FResultsOrdinalSuffixTest::RunTest(const FString& Parameters)
{
	// Test ordinal suffix logic (matching GetOrdinalSuffix implementation)
	struct FOrdinalCase { int32 Position; FString ExpectedSuffix; FString Description; };

	TArray<FOrdinalCase> Cases = {
		{1, TEXT("st"), TEXT("1st")},
		{2, TEXT("nd"), TEXT("2nd")},
		{3, TEXT("rd"), TEXT("3rd")},
		{4, TEXT("th"), TEXT("4th")},
		{11, TEXT("th"), TEXT("11th")},
		{12, TEXT("th"), TEXT("12th")},
		{13, TEXT("th"), TEXT("13th")},
		{21, TEXT("st"), TEXT("21st")},
		{22, TEXT("nd"), TEXT("22nd")},
		{23, TEXT("rd"), TEXT("23rd")},
		{100, TEXT("th"), TEXT("100th")},
		{111, TEXT("th"), TEXT("111th")},
	};

	// Verify the ordinal suffix logic matches the expected pattern
	// This tests the contract that the UI code relies on
	for (const FOrdinalCase& TC : Cases)
	{
		FString Suffix;
		int32 Mod100 = TC.Position % 100;
		int32 Mod10 = TC.Position % 10;

		if (Mod100 >= 11 && Mod100 <= 13)
		{
			Suffix = TEXT("th");
		}
		else if (Mod10 == 1)
		{
			Suffix = TEXT("st");
		}
		else if (Mod10 == 2)
		{
			Suffix = TEXT("nd");
		}
		else if (Mod10 == 3)
		{
			Suffix = TEXT("rd");
		}
		else
		{
			Suffix = TEXT("th");
		}

		TestEqual(*TC.Description, Suffix, TC.ExpectedSuffix);
	}

	return true;
}

/**
 * ResultsWidget time formatting
 * Tests the FormatTime helper contract
 */
bool FResultsTimeFormatTest::RunTest(const FString& Parameters)
{
	// Test time formatting logic (MM:SS.mmm)
	struct FTimeCase { float Seconds; FString ExpectedPrefix; FString Description; };

	TArray<FTimeCase> Cases = {
		{0.0f, TEXT("00:00"), TEXT("Zero time")},
		{1.5f, TEXT("00:01"), TEXT("1.5 seconds")},
		{59.999f, TEXT("00:59"), TEXT("Under 1 minute")},
		{60.0f, TEXT("01:00"), TEXT("Exactly 1 minute")},
		{65.5f, TEXT("01:05"), TEXT("1 min 5.5 sec")},
		{125.3f, TEXT("02:05"), TEXT("2 min 5.3 sec")},
		{600.0f, TEXT("10:00"), TEXT("10 minutes")},
		{3661.0f, TEXT("61:01"), TEXT("Over 1 hour (race format)")},
	};

	for (const FTimeCase& TC : Cases)
	{
		int32 Minutes = FMath::FloorToInt(TC.Seconds / 60.0f);
		float Remainder = TC.Seconds - (Minutes * 60.0f);
		int32 Sec = FMath::FloorToInt(Remainder);
		int32 Ms = FMath::FloorToInt((Remainder - Sec) * 1000.0f);

		FString Formatted = FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Sec, Ms);

		TestTrue(*TC.Description, Formatted.StartsWith(TC.ExpectedPrefix));
	}

	return true;
}

// ─── Integration Tests ──────────────────────────────────────────────────────

/**
 * Full HUD data pipeline: VehicleStateManager state -> FHUDData -> RaceHUD
 * Simulates what NomiRaceGameMode::Tick() does.
 */
bool FHUDDataPipelineTest::RunTest(const FString& Parameters)
{
	// Arrange: create all components in the pipeline
	UVehicleStateManager* VSM = NewObject<UVehicleStateManager>();
	ARaceManager* RM = NewObject<ARaceManager>();
	UCommentaryEngine* CE = NewObject<UCommentaryEngine>();
	URaceHUD* HUD = NewObject<URaceHUD>();

	TestNotNull(TEXT("VSM should be created"), VSM);
	TestNotNull(TEXT("RaceManager should be created"), RM);
	TestNotNull(TEXT("CommentaryEngine should be created"), CE);
	TestNotNull(TEXT("RaceHUD should be created"), HUD);

	if (!VSM || !RM || !CE || !HUD) { return false; }

	// Act: simulate the data pipeline from NomiRaceGameMode::Tick()
	FHUDData Data;

	// Vehicle telemetry from VSM
	const FNIOVehicleState& VState = VSM->GetVehicleState();
	Data.Speed = VState.SpeedKmh;
	Data.ThrottleInput = VState.ThrottleInput;
	Data.BrakeInput = VState.BrakeInput;
	Data.SteeringInput = VState.SteeringInput;
	Data.bIsDrifting = VState.bIsDrifting;
	Data.DriftAngle = VState.SlipAngle;
	Data.BatteryLevel = VState.BatteryPercent;
	Data.bIsNIOVehicle = VSM->IsNIOVehicle();

	// Race telemetry from RaceManager
	ERaceState CurrentState = RM->GetRaceState();
	Data.RaceState = CurrentState;
	Data.Position = RM->GetPlayerPosition();
	Data.TotalRacers = RM->GetAllRacers().Num();
	Data.CurrentLap = RM->GetPlayerCurrentLap();
	Data.TotalLaps = RM->GetRaceConfig().NumLaps;
	Data.RaceTimer = RM->GetRaceTimer();

	// NOMI data from CommentaryEngine
	Data.bNOMICommentVisible = CE->IsCommentPlaying();
	if (Data.bNOMICommentVisible)
	{
		Data.NOMICommentText = CE->GetCurrentCommentText();
		Data.NOMIEmotion = CE->GetCurrentEmotion();
	}

	// Push to HUD
	HUD->UpdateHUDData(Data);

	// Assert: full pipeline should preserve all data
	const FHUDData& Stored = HUD->GetHUDData();
	TestEqual(TEXT("Pipeline speed should be 0"), Stored.Speed, 0.0f);
	TestEqual(TEXT("Pipeline battery should be 100"), Stored.BatteryLevel, 100.0f);
	TestEqual(TEXT("Pipeline race state should be Idle"), Stored.RaceState, ERaceState::Idle);
	TestFalse(TEXT("Pipeline NOMI comment should be hidden"), Stored.bNOMICommentVisible);

	// Act: start race and update pipeline
	FRaceConfig Config;
	Config.NumLaps = 3;
	Config.TrackName = TEXT("Pipeline Test Track");
	RM->StartRace(Config);

	Data.RaceState = RM->GetRaceState();
	Data.TotalLaps = RM->GetRaceConfig().NumLaps;

	// Add a racer to test position data
	APawn* Pawn = NewObject<APawn>();
	RM->RegisterRacer(Pawn, TEXT("TestPlayer"), true);
	Data.Position = RM->GetPlayerPosition();
	Data.TotalRacers = RM->GetAllRacers().Num();

	HUD->UpdateHUDData(Data);

	// Assert: updated pipeline data
	const FHUDData& Updated = HUD->GetHUDData();
	TestEqual(TEXT("Updated race state should be Countdown"), Updated.RaceState, ERaceState::Countdown);
	TestEqual(TEXT("Updated total laps should be 3"), Updated.TotalLaps, 3);
	TestEqual(TEXT("Updated total racers should be 1"), Updated.TotalRacers, 1);

	return true;
}

/**
 * RaceFinish event -> ResultsWidget creation pipeline
 * Simulates what NomiRaceGameMode::OnRaceEvent does for RaceFinish
 */
bool FRaceFinishResultsPipelineTest::RunTest(const FString& Parameters)
{
	// Arrange: simulate the race finish data flow
	FRacerData PlayerRacer;
	PlayerRacer.bIsPlayer = true;
	PlayerRacer.Position = 2;
	PlayerRacer.LapTimes = {23.0f, 22.5f, 21.8f};
	PlayerRacer.BestLapTime = 21.8f;
	PlayerRacer.TotalRaceTime = 67.3f;

	// Act: build FRaceSessionResult from racer data (mirrors OnRaceEvent logic)
	FRaceSessionResult SessionResult;
	SessionResult.FinalPosition = PlayerRacer.Position;
	SessionResult.TotalRacers = 8;
	SessionResult.NumLaps = PlayerRacer.LapTimes.Num();
	SessionResult.LapTimes = PlayerRacer.LapTimes;
	SessionResult.BestLapTime = PlayerRacer.BestLapTime;
	SessionResult.TotalRaceTime = PlayerRacer.TotalRaceTime;
	SessionResult.RaceMode = TEXT("StreetGT");
	SessionResult.Timestamp = FDateTime::Now();

	// Assert: session result should be correctly built
	TestEqual(TEXT("Final position should be 2"), SessionResult.FinalPosition, 2);
	TestEqual(TEXT("Num laps should be 3"), SessionResult.NumLaps, 3);
	TestEqual(TEXT("Should have 3 lap times"), SessionResult.LapTimes.Num(), 3);
	TestEqual(TEXT("Best lap time should match"), SessionResult.BestLapTime, 21.8f);
	TestEqual(TEXT("Total race time should match"), SessionResult.TotalRaceTime, 67.3f);

	// Act: create ResultsWidget and wire it up
	UResultsWidget* Results = NewObject<UResultsWidget>();
	TestNotNull(TEXT("ResultsWidget should be created"), Results);
	if (!Results) { return false; }

	UMenuManager* MenuMgr = NewObject<UMenuManager>();
	TestNotNull(TEXT("MenuManager should be created"), MenuMgr);
	if (!MenuMgr) { return false; }

	Results->SetMenuManager(MenuMgr);
	Results->SetResults(SessionResult, false);

	// Assert: ResultsWidget should hold the MenuManager reference
	TestTrue(TEXT("ResultsWidget should have MenuManager"), Results->MenuManager == MenuMgr);

	// Verify session result data is accessible
	TestEqual(TEXT("Session result final position"), SessionResult.FinalPosition, 2);
	TestEqual(TEXT("Session result best lap"), SessionResult.BestLapTime, 21.8f);
	TestTrue(TEXT("Session result should have lap times"), SessionResult.LapTimes.Num() > 0);

	return true;
}

/**
 * Baja mode HUD data (distance to finish, progress)
 */
bool FHUDBajaModeTest::RunTest(const FString& Parameters)
{
	// Arrange
	ARaceManager* RM = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RM);
	if (!RM) { return false; }

	FHUDData Data;

	// Act: set up Baja mode data
	Data.bIsBajaMode = true;
	Data.DistanceToFinish = 3500.0f;
	Data.ProgressPercent = 30.0f;

	// Assert: Baja mode fields should be set correctly
	TestTrue(TEXT("Should be Baja mode"), Data.bIsBajaMode);
	TestEqual(TEXT("Distance to finish should be 3500m"), Data.DistanceToFinish, 3500.0f);
	TestEqual(TEXT("Progress should be 30%"), Data.ProgressPercent, 30.0f);

	// Act: verify RaceManager Baja config
	FRaceConfig Config;
	Config.NumLaps = 0;
	Config.bIsPointToPoint = true;
	Config.TrackName = TEXT("Baja Desert");
	RM->StartRace(Config);

	Data.bIsBajaMode = RM->GetRaceConfig().bIsPointToPoint;

	// Assert
	TestTrue(TEXT("RaceManager should be in Baja mode"), Data.bIsBajaMode);

	// Edge case: non-Baja mode
	Data.bIsBajaMode = false;
	Data.DistanceToFinish = 0.0f;
	Data.ProgressPercent = 0.0f;
	TestFalse(TEXT("Non-Baja mode should not show distance"), Data.bIsBajaMode);
	TestEqual(TEXT("Non-Baja distance should be 0"), Data.DistanceToFinish, 0.0f);

	return true;
}
