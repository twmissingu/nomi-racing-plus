// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/TestUtilities.h"

// ============================================================================
// Vehicle Mocking Utilities
// ============================================================================

UVehicleStateManager* NomiTestUtils::CreateMockVehicleStateManager(ENIOVehicleType VehicleType)
{
	UVehicleStateManager* StateManager = NewObject<UVehicleStateManager>();
	if (StateManager)
	{
		StateManager->SetVehicleType(VehicleType);
		StateManager->SetPerformanceConfig(CreatePerformanceConfig(VehicleType));
	}
	return StateManager;
}

FNIOPerformanceConfig NomiTestUtils::CreatePerformanceConfig(ENIOVehicleType VehicleType)
{
	FNIOPerformanceConfig Config;

	switch (VehicleType)
	{
	case ENIOVehicleType::EP9:
		Config.MassKg = 1735.0f;
		Config.PowerKw = 480.0f;
		Config.TorqueNm = 850.0f;
		Config.DriveType = TEXT("AWD_quad_motor");
		Config.TopSpeedKph = 313.0f;
		Config.Acceleration0100 = 2.7f;
		Config.DownforceMaxKg = 2000.0f;
		Config.WheelbaseMm = 2850.0f;
		Config.BodyType = TEXT("hypercar");
		break;

	case ENIOVehicleType::ET7:
		Config.MassKg = 2379.0f;
		Config.PowerKw = 480.0f;
		Config.TorqueNm = 850.0f;
		Config.DriveType = TEXT("AWD_dual_motor");
		Config.TopSpeedKph = 200.0f;
		Config.Acceleration0100 = 3.8f;
		Config.DownforceMaxKg = 0.0f;
		Config.WheelbaseMm = 3060.0f;
		Config.BodyType = TEXT("sedan");
		break;

	case ENIOVehicleType::ES7:
		Config.MassKg = 2399.0f;
		Config.PowerKw = 480.0f;
		Config.TorqueNm = 850.0f;
		Config.DriveType = TEXT("AWD_dual_motor");
		Config.TopSpeedKph = 200.0f;
		Config.Acceleration0100 = 3.9f;
		Config.DownforceMaxKg = 0.0f;
		Config.WheelbaseMm = 2960.0f;
		Config.BodyType = TEXT("suv");
		break;

	case ENIOVehicleType::ET5:
		Config.MassKg = 2010.0f;
		Config.PowerKw = 360.0f;
		Config.TorqueNm = 700.0f;
		Config.DriveType = TEXT("AWD_dual_motor");
		Config.TopSpeedKph = 200.0f;
		Config.Acceleration0100 = 4.3f;
		Config.DownforceMaxKg = 0.0f;
		Config.WheelbaseMm = 2888.0f;
		Config.BodyType = TEXT("sedan");
		break;

	default:
		// Custom: generic sport config
		Config.MassKg = 1800.0f;
		Config.PowerKw = 400.0f;
		Config.TorqueNm = 800.0f;
		Config.TopSpeedKph = 250.0f;
		Config.Acceleration0100 = 3.5f;
		break;
	}

	Config.bIsElectric = true;
	Config.RegenBrakingStrength = 0.3f;
	return Config;
}

FNIOVehicleState NomiTestUtils::CreateVehicleState(
	float SpeedKmh,
	float ThrottleInput,
	float BrakeInput,
	float SteeringInput,
	bool bIsDrifting,
	bool bIsGrounded,
	float BatteryPercent)
{
	FNIOVehicleState State;
	State.SpeedKmh = SpeedKmh;
	State.ThrottleInput = ThrottleInput;
	State.BrakeInput = BrakeInput;
	State.SteeringInput = SteeringInput;
	State.bIsDrifting = bIsDrifting;
	State.bIsGrounded = bIsGrounded;
	State.BatteryPercent = BatteryPercent;
	return State;
}

APawn* NomiTestUtils::CreateMockPawn(const FString& Name)
{
	APawn* Pawn = NewObject<APawn>();
	if (Pawn)
	{
		Pawn->Rename(*Name);
	}
	return Pawn;
}

void NomiTestUtils::CreateMockAIPawns(int32 Count, TArray<APawn*>& OutPawns)
{
	OutPawns.Reserve(Count);
	for (int32 i = 0; i < Count; i++)
	{
		FString Name = FString::Printf(TEXT("AI %d"), i + 1);
		OutPawns.Add(CreateMockPawn(Name));
	}
}

// ============================================================================
// Race Creation Utilities
// ============================================================================

FRaceConfig NomiTestUtils::CreateRaceConfig(
	int32 NumLaps,
	const FString& TrackName,
	int32 MaxAIOpponents,
	float CountdownDuration)
{
	FRaceConfig Config;
	Config.NumLaps = NumLaps;
	Config.TrackName = TrackName;
	Config.MaxAIOpponents = MaxAIOpponents;
	Config.CountdownDuration = CountdownDuration;
	Config.RaceMode = TEXT("StreetGT");
	Config.bAllowCollisions = true;
	Config.CollisionPenalty = 0.0f;
	return Config;
}

FRaceConfig NomiTestUtils::CreateSprintConfig(const FString& TrackName)
{
	return CreateRaceConfig(1, TrackName, 3, 3.0f);
}

FRaceConfig NomiTestUtils::CreateEnduranceConfig(const FString& TrackName)
{
	return CreateRaceConfig(10, TrackName, 7, 5.0f);
}

FRacerData NomiTestUtils::CreateRacerData(
	int32 RacerID,
	const FString& DisplayName,
	bool bIsPlayer,
	int32 Position)
{
	FRacerData Data;
	Data.RacerID = RacerID;
	Data.DisplayName = DisplayName;
	Data.bIsPlayer = bIsPlayer;
	Data.Position = Position;
	Data.CurrentLap = 0;
	Data.CurrentCheckpoint = 0;
	Data.TotalCheckpointsPassed = 0;
	Data.BestLapTime = -1.0f;
	Data.TotalRaceTime = 0.0f;
	Data.bFinished = false;
	Data.TrackProgress = 0.0f;
	return Data;
}

FRaceSessionResult NomiTestUtils::CreateSessionResult(
	const FString& TrackName,
	const FString& VehicleName,
	int32 FinalPosition,
	int32 TotalRacers,
	int32 NumLaps)
{
	FRaceSessionResult Result;
	Result.TrackName = TrackName;
	Result.VehicleName = VehicleName;
	Result.FinalPosition = FinalPosition;
	Result.TotalRacers = TotalRacers;
	Result.NumLaps = NumLaps;

	// Generate synthetic lap times: base ~60s + small variation
	float BaseLapTime = 60.0f;
	float TotalTime = 0.0f;
	Result.LapTimes.Reserve(NumLaps);
	for (int32 i = 0; i < NumLaps; i++)
	{
		float LapTime = BaseLapTime + FMath::FRandRange(-2.0f, 5.0f);
		Result.LapTimes.Add(LapTime);
		TotalTime += LapTime;
	}
	Result.BestLapTime = FMath::Min(Result.LapTimes);
	Result.TotalRaceTime = TotalTime;

	Result.MaxSpeed = FMath::FRandRange(180.0f, 300.0f);
	Result.DriftTime = FMath::FRandRange(0.0f, 15.0f);
	Result.Overtakes = FMath::RandRange(0, 10);
	Result.Collisions = FMath::RandRange(0, 3);
	Result.DistanceDriven = FMath::FRandRange(5000.0f, 50000.0f);
	Result.bCleanRace = (Result.Collisions == 0);
	Result.RaceMode = TEXT("StreetGT");
	Result.Difficulty = 1;
	Result.bChampionshipRace = false;
	Result.Timestamp = FDateTime::Now();
	return Result;
}

FChampionshipData NomiTestUtils::CreateChampionshipData(
	const FString& ID,
	const FString& Name,
	EChampionshipTier Tier,
	int32 NumRaces)
{
	FChampionshipData Data;
	Data.ID = ID;
	Data.Name = Name;
	Data.Tier = Tier;
	Data.CurrentRace = 0;
	Data.bComplete = false;
	Data.bPlayerWon = false;
	Data.FinalPlayerPosition = 0;
	Data.NumAIOpponents = 7;
	Data.Difficulty = static_cast<int32>(Tier);
	Data.StartTime = FDateTime::Now();

	// Generate track names
	Data.Tracks.Reserve(NumRaces);
	for (int32 i = 0; i < NumRaces; i++)
	{
		Data.Tracks.Add(FString::Printf(TEXT("Track_%d"), i + 1));
	}

	// Standard points table (F1-style)
	Data.PointsPerPosition.Add(1, 25);
	Data.PointsPerPosition.Add(2, 18);
	Data.PointsPerPosition.Add(3, 15);
	Data.PointsPerPosition.Add(4, 12);
	Data.PointsPerPosition.Add(5, 10);
	Data.PointsPerPosition.Add(6, 8);
	Data.PointsPerPosition.Add(7, 6);
	Data.PointsPerPosition.Add(8, 4);

	// Initialize standings with player
	FChampionshipStandingEntry PlayerEntry;
	PlayerEntry.Name = TEXT("Player");
	PlayerEntry.bIsPlayer = true;
	PlayerEntry.Points = 0;
	Data.Standings.Add(PlayerEntry);

	// Initialize AI standings
	for (int32 i = 0; i < Data.NumAIOpponents; i++)
	{
		FChampionshipStandingEntry AIEntry;
		AIEntry.Name = FString::Printf(TEXT("AI Driver %d"), i + 1);
		AIEntry.bIsPlayer = false;
		AIEntry.Points = 0;
		Data.Standings.Add(AIEntry);
	}

	return Data;
}

ARaceManager* NomiTestUtils::CreateConfiguredRaceManager(
	const FRaceConfig& Config,
	int32 AICount,
	APawn** OutPlayerPawn,
	TArray<APawn*>* OutAIPawns)
{
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	if (!RaceManager)
	{
		return nullptr;
	}

	// Create and register player
	APawn* PlayerPawn = CreateMockPawn(TEXT("Player"));
	RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);

	// Create and register AI opponents
	TArray<APawn*> AIPawns;
	CreateMockAIPawns(AICount, AIPawns);
	for (int32 i = 0; i < AIPawns.Num(); i++)
	{
		FString Name = FString::Printf(TEXT("AI %d"), i + 1);
		RaceManager->RegisterRacer(AIPawns[i], Name, false);
	}

	// Start the race
	RaceManager->StartRace(Config);

	// Output pawns if requested
	if (OutPlayerPawn)
	{
		*OutPlayerPawn = PlayerPawn;
	}
	if (OutAIPawns)
	{
		*OutAIPawns = AIPawns;
	}

	return RaceManager;
}

// ============================================================================
// Game State Validation Utilities
// ============================================================================

bool NomiTestUtils::ValidateVehicleState(
	FAutomationTestBase* Test,
	const FNIOVehicleState& State,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	// Speed must be non-negative
	if (State.SpeedKmh < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sSpeedKmh is negative: %f"), *Prefix, State.SpeedKmh));
		bValid = false;
	}

	// Input values should be in [-1, 1] range
	if (State.ThrottleInput < -1.0f || State.ThrottleInput > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sThrottleInput out of range [-1,1]: %f"), *Prefix, State.ThrottleInput));
		bValid = false;
	}
	if (State.BrakeInput < -1.0f || State.BrakeInput > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sBrakeInput out of range [-1,1]: %f"), *Prefix, State.BrakeInput));
		bValid = false;
	}
	if (State.SteeringInput < -1.0f || State.SteeringInput > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sSteeringInput out of range [-1,1]: %f"), *Prefix, State.SteeringInput));
		bValid = false;
	}

	// Battery should be 0-100
	if (State.BatteryPercent < 0.0f || State.BatteryPercent > 100.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sBatteryPercent out of range [0,100]: %f"), *Prefix, State.BatteryPercent));
		bValid = false;
	}

	// Tire wear should be 0-1
	if (State.AverageTireWear < 0.0f || State.AverageTireWear > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sAverageTireWear out of range [0,1]: %f"), *Prefix, State.AverageTireWear));
		bValid = false;
	}

	// Tire temperature should be physically plausible (-40 to 200 C)
	if (State.AverageTireTemperature < -40.0f || State.AverageTireTemperature > 200.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sAverageTireTemperature implausible: %f"), *Prefix, State.AverageTireTemperature));
		bValid = false;
	}

	return bValid;
}

bool NomiTestUtils::ValidateVehicleStateManagerDefaults(
	FAutomationTestBase* Test,
	UVehicleStateManager* StateManager,
	const FString& Context)
{
	if (!StateManager)
	{
		Test->AddError(FString::Printf(TEXT("[%s] StateManager is null"), *Context.IsEmpty() ? TEXT("StateManagerDefaults") : *Context));
		return false;
	}

	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	const FNIOVehicleState& State = StateManager->GetVehicleState();

	if (State.SpeedKmh != 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial speed should be 0, got %f"), *Prefix, State.SpeedKmh));
		bValid = false;
	}
	if (State.ThrottleInput != 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial throttle should be 0, got %f"), *Prefix, State.ThrottleInput));
		bValid = false;
	}
	if (State.BrakeInput != 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial brake should be 0, got %f"), *Prefix, State.BrakeInput));
		bValid = false;
	}
	if (State.bIsDrifting)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial drift should be false"), *Prefix));
		bValid = false;
	}
	if (!State.bIsGrounded)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial grounded should be true"), *Prefix));
		bValid = false;
	}
	if (State.BatteryPercent != 100.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial battery should be 100, got %f"), *Prefix, State.BatteryPercent));
		bValid = false;
	}
	if (State.AverageTireWear != 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sInitial tire wear should be 1.0, got %f"), *Prefix, State.AverageTireWear));
		bValid = false;
	}

	return bValid;
}

bool NomiTestUtils::ValidatePerformanceConfig(
	FAutomationTestBase* Test,
	const FNIOPerformanceConfig& Config,
	ENIOVehicleType ExpectedType,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	if (Config.MassKg <= 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sMassKg must be positive, got %f"), *Prefix, Config.MassKg));
		bValid = false;
	}
	if (Config.PowerKw <= 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sPowerKw must be positive, got %f"), *Prefix, Config.PowerKw));
		bValid = false;
	}
	if (Config.TorqueNm <= 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTorqueNm must be positive, got %f"), *Prefix, Config.TorqueNm));
		bValid = false;
	}
	if (Config.TopSpeedKph <= 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTopSpeedKph must be positive, got %f"), *Prefix, Config.TopSpeedKph));
		bValid = false;
	}
	if (Config.Acceleration0100 <= 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sAcceleration0100 must be positive, got %f"), *Prefix, Config.Acceleration0100));
		bValid = false;
	}
	if (!Config.bIsElectric)
	{
		Test->AddError(FString::Printf(TEXT("%sNIO vehicles must be electric"), *Prefix));
		bValid = false;
	}

	// Vehicle-specific range checks
	switch (ExpectedType)
	{
	case ENIOVehicleType::EP9:
		if (Config.TopSpeedKph < 300.0f)
		{
			Test->AddError(FString::Printf(TEXT("%sEP9 top speed should be >= 300 kph, got %f"), *Prefix, Config.TopSpeedKph));
			bValid = false;
		}
		if (Config.DownforceMaxKg <= 0.0f)
		{
			Test->AddError(FString::Printf(TEXT("%sEP9 should have downforce, got %f"), *Prefix, Config.DownforceMaxKg));
			bValid = false;
		}
		break;

	case ENIOVehicleType::ET7:
	case ENIOVehicleType::ES7:
	case ENIOVehicleType::ET5:
		if (Config.TopSpeedKph < 150.0f)
		{
			Test->AddError(FString::Printf(TEXT("%sVehicle top speed should be >= 150 kph, got %f"), *Prefix, Config.TopSpeedKph));
			bValid = false;
		}
		break;

	default:
		break;
	}

	return bValid;
}

bool NomiTestUtils::ValidateRacerData(
	FAutomationTestBase* Test,
	const FRacerData& Data,
	const FString& ExpectedName,
	bool bExpectedIsPlayer,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	if (Data.DisplayName != ExpectedName)
	{
		Test->AddError(FString::Printf(TEXT("%sDisplayName mismatch: expected '%s', got '%s'"),
			*Prefix, *ExpectedName, *Data.DisplayName));
		bValid = false;
	}
	if (Data.bIsPlayer != bExpectedIsPlayer)
	{
		Test->AddError(FString::Printf(TEXT("%sbIsPlayer mismatch: expected %s, got %s"),
			*Prefix, bExpectedIsPlayer ? TEXT("true") : TEXT("false"), Data.bIsPlayer ? TEXT("true") : TEXT("false")));
		bValid = false;
	}
	if (Data.CurrentLap < 0)
	{
		Test->AddError(FString::Printf(TEXT("%sCurrentLap is negative: %d"), *Prefix, Data.CurrentLap));
		bValid = false;
	}
	if (Data.TotalRaceTime < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTotalRaceTime is negative: %f"), *Prefix, Data.TotalRaceTime));
		bValid = false;
	}
	if (Data.TrackProgress < 0.0f || Data.TrackProgress > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTrackProgress out of range [0,1]: %f"), *Prefix, Data.TrackProgress));
		bValid = false;
	}

	return bValid;
}

bool NomiTestUtils::ValidateRaceState(
	FAutomationTestBase* Test,
	ARaceManager* RaceManager,
	ERaceState ExpectedState,
	int32 ExpectedRacerCount,
	const FString& Context)
{
	if (!RaceManager)
	{
		Test->AddError(FString::Printf(TEXT("[%s] RaceManager is null"), *Context.IsEmpty() ? TEXT("RaceState") : *Context));
		return false;
	}

	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	ERaceState ActualState = RaceManager->GetRaceState();
	if (ActualState != ExpectedState)
	{
		Test->AddError(FString::Printf(TEXT("%sRace state mismatch: expected %d, got %d"),
			*Prefix, static_cast<int32>(ExpectedState), static_cast<int32>(ActualState)));
		bValid = false;
	}

	if (ExpectedRacerCount >= 0)
	{
		int32 ActualCount = RaceManager->GetAllRacers().Num();
		if (ActualCount != ExpectedRacerCount)
		{
			Test->AddError(FString::Printf(TEXT("%sRacer count mismatch: expected %d, got %d"),
				*Prefix, ExpectedRacerCount, ActualCount));
			bValid = false;
		}
	}

	return bValid;
}

bool NomiTestUtils::ValidateSessionResult(
	FAutomationTestBase* Test,
	const FRaceSessionResult& Result,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	if (Result.FinalPosition < 1 || Result.FinalPosition > Result.TotalRacers)
	{
		Test->AddError(FString::Printf(TEXT("%sFinalPosition %d out of range [1, %d]"),
			*Prefix, Result.FinalPosition, Result.TotalRacers));
		bValid = false;
	}
	if (Result.NumLaps <= 0)
	{
		Test->AddError(FString::Printf(TEXT("%sNumLaps must be positive, got %d"), *Prefix, Result.NumLaps));
		bValid = false;
	}
	if (Result.LapTimes.Num() != Result.NumLaps)
	{
		Test->AddError(FString::Printf(TEXT("%sLapTimes count %d != NumLaps %d"),
			*Prefix, Result.LapTimes.Num(), Result.NumLaps));
		bValid = false;
	}
	if (Result.TotalRaceTime < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTotalRaceTime is negative: %f"), *Prefix, Result.TotalRaceTime));
		bValid = false;
	}
	if (Result.MaxSpeed < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sMaxSpeed is negative: %f"), *Prefix, Result.MaxSpeed));
		bValid = false;
	}
	if (Result.DistanceDriven < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sDistanceDriven is negative: %f"), *Prefix, Result.DistanceDriven));
		bValid = false;
	}
	if (Result.Overtakes < 0)
	{
		Test->AddError(FString::Printf(TEXT("%sOvertakes is negative: %d"), *Prefix, Result.Overtakes));
		bValid = false;
	}
	if (Result.Collisions < 0)
	{
		Test->AddError(FString::Printf(TEXT("%sCollisions is negative: %d"), *Prefix, Result.Collisions));
		bValid = false;
	}
	if (Result.bCleanRace && Result.Collisions > 0)
	{
		Test->AddError(FString::Printf(TEXT("%sbCleanRace is true but Collisions is %d"), *Prefix, Result.Collisions));
		bValid = false;
	}

	// Best lap time should be the minimum of lap times (if laps exist)
	if (Result.LapTimes.Num() > 0 && Result.BestLapTime > 0.0f)
	{
		float ActualMin = FMath::Min(Result.LapTimes);
		if (!FMath::IsNearlyEqual(Result.BestLapTime, ActualMin, 0.01f))
		{
			Test->AddError(FString::Printf(TEXT("%sBestLapTime %f != min of lap times %f"),
				*Prefix, Result.BestLapTime, ActualMin));
			bValid = false;
		}
	}

	return bValid;
}

bool NomiTestUtils::ValidateChampionshipData(
	FAutomationTestBase* Test,
	const FChampionshipData& Data,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	if (Data.ID.IsEmpty())
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship ID is empty"), *Prefix));
		bValid = false;
	}
	if (Data.Name.IsEmpty())
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship Name is empty"), *Prefix));
		bValid = false;
	}
	if (Data.Tracks.Num() == 0)
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship has no tracks"), *Prefix));
		bValid = false;
	}
	if (Data.PointsPerPosition.Num() == 0)
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship has no points table"), *Prefix));
		bValid = false;
	}
	if (Data.Standings.Num() == 0)
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship has no standings"), *Prefix));
		bValid = false;
	}
	if (Data.CurrentRace < 0 || Data.CurrentRace > Data.Tracks.Num())
	{
		Test->AddError(FString::Printf(TEXT("%sCurrentRace %d out of range [0, %d]"),
			*Prefix, Data.CurrentRace, Data.Tracks.Num()));
		bValid = false;
	}

	// Verify at least one player in standings
	bool bHasPlayer = false;
	for (const FChampionshipStandingEntry& Entry : Data.Standings)
	{
		if (Entry.bIsPlayer)
		{
			bHasPlayer = true;
			break;
		}
	}
	if (!bHasPlayer)
	{
		Test->AddError(FString::Printf(TEXT("%sChampionship standings have no player entry"), *Prefix));
		bValid = false;
	}

	return bValid;
}

bool NomiTestUtils::ValidateTireState(
	FAutomationTestBase* Test,
	const FTireState& State,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	if (State.SlipRatio < -1.0f || State.SlipRatio > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sSlipRatio out of range [-1,1]: %f"), *Prefix, State.SlipRatio));
		bValid = false;
	}
	if (State.Thermal.WearFactor < 0.0f || State.Thermal.WearFactor > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sWearFactor out of range [0,1]: %f"), *Prefix, State.Thermal.WearFactor));
		bValid = false;
	}
	if (State.Thermal.CoreTemperature < -40.0f || State.Thermal.CoreTemperature > 200.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sCoreTemperature implausible: %f"), *Prefix, State.Thermal.CoreTemperature));
		bValid = false;
	}
	if (State.Thermal.ThermalGripMultiplier < 0.0f || State.Thermal.ThermalGripMultiplier > 2.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sThermalGripMultiplier out of range [0,2]: %f"), *Prefix, State.Thermal.ThermalGripMultiplier));
		bValid = false;
	}
	if (State.WheelLoad < 0.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sWheelLoad is negative: %f"), *Prefix, State.WheelLoad));
		bValid = false;
	}

	return bValid;
}

bool NomiTestUtils::ValidateTirePreset(
	FAutomationTestBase* Test,
	const FTireModelPreset& Preset,
	const FString& Context)
{
	FString Prefix = Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT("[%s] "), *Context);
	bool bValid = true;

	// Pacejka coefficient ranges
	if (Preset.Pacejka.Longitudinal.B < 0.1f || Preset.Pacejka.Longitudinal.B > 50.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sLongitudinal.B out of range: %f"), *Prefix, Preset.Pacejka.Longitudinal.B));
		bValid = false;
	}
	if (Preset.Pacejka.Lateral.B < 0.1f || Preset.Pacejka.Lateral.B > 50.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sLateral.B out of range: %f"), *Prefix, Preset.Pacejka.Lateral.B));
		bValid = false;
	}
	if (Preset.Pacejka.CombinedSlipFactor < 0.0f || Preset.Pacejka.CombinedSlipFactor > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sCombinedSlipFactor out of range [0,1]: %f"), *Prefix, Preset.Pacejka.CombinedSlipFactor));
		bValid = false;
	}

	// Dimensions
	if (Preset.TireWidthMm < 100.0f || Preset.TireWidthMm > 400.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sTireWidthMm out of range [100,400]: %f"), *Prefix, Preset.TireWidthMm));
		bValid = false;
	}
	if (Preset.AspectRatio < 20.0f || Preset.AspectRatio > 80.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sAspectRatio out of range [20,80]: %f"), *Prefix, Preset.AspectRatio));
		bValid = false;
	}
	if (Preset.RimDiameterInches < 14.0f || Preset.RimDiameterInches > 24.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sRimDiameterInches out of range [14,24]: %f"), *Prefix, Preset.RimDiameterInches));
		bValid = false;
	}

	// Thermal thresholds ordering
	if (Preset.ColdThreshold >= Preset.OptimalTemp)
	{
		Test->AddError(FString::Printf(TEXT("%sColdThreshold (%f) should be < OptimalTemp (%f)"),
			*Prefix, Preset.ColdThreshold, Preset.OptimalTemp));
		bValid = false;
	}
	if (Preset.OptimalTemp >= Preset.OverheatThreshold)
	{
		Test->AddError(FString::Printf(TEXT("%sOptimalTemp (%f) should be < OverheatThreshold (%f)"),
			*Prefix, Preset.OptimalTemp, Preset.OverheatThreshold));
		bValid = false;
	}

	// Wear rate
	if (Preset.WearRate < 0.0f || Preset.WearRate > 1.0f)
	{
		Test->AddError(FString::Printf(TEXT("%sWearRate out of range [0,1]: %f"), *Prefix, Preset.WearRate));
		bValid = false;
	}

	return bValid;
}
