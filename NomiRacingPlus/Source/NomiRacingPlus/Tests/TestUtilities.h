// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/TirePhysicsModel.h"
#include "Vehicles/NIOTirePresets.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Race/RaceManager.h"
#include "Race/RaceProgression.h"
#include "Race/ChampionshipManager.h"
#include "AI/AICarController.h"

// ============================================================================
// Vehicle Mocking Utilities
// ============================================================================

namespace NomiTestUtils
{

/**
 * Create a VehicleStateManager configured for a specific NIO vehicle type.
 * Returns a UObject that the caller is responsible for managing (GC-aware).
 */
UVehicleStateManager* CreateMockVehicleStateManager(ENIOVehicleType VehicleType = ENIOVehicleType::EP9);

/**
 * Create a FNIOPerformanceConfig pre-filled with realistic values for a given vehicle type.
 * EP9: hypercar, 480kW, 850Nm, 313 kph, 2.7s 0-100
 * ET7: luxury sedan, 480kW, 850Nm, 200 kph, 3.8s 0-100
 * ES7: SUV, 480kW, 850Nm, 200 kph, 3.9s 0-100
 * ET5: sport sedan, 360kW, 700Nm, 200 kph, 4.3s 0-100
 */
FNIOPerformanceConfig CreatePerformanceConfig(ENIOVehicleType VehicleType);

/**
 * Create a FVehicleState with specific values for testing.
 * All parameters have sensible defaults so callers can override only what matters.
 */
FVehicleState CreateVehicleState(
	float SpeedKmh = 0.0f,
	float ThrottleInput = 0.0f,
	float BrakeInput = 0.0f,
	float SteeringInput = 0.0f,
	bool bIsDrifting = false,
	bool bIsGrounded = true,
	float BatteryPercent = 100.0f
);

/**
 * Create a mock APawn suitable for registration with RaceManager.
 * Uses NewObject<APawn>() since full actor spawning requires a world.
 */
APawn* CreateMockPawn(const FString& Name = TEXT("MockPawn"));

/**
 * Create a set of mock AI pawns, pre-named "AI 1", "AI 2", etc.
 * Returns them in the OutPawns array. Caller manages lifetime.
 */
void CreateMockAIPawns(int32 Count, TArray<APawn*>& OutPawns);

// ============================================================================
// Race Creation Utilities
// ============================================================================

/**
 * Create a FRaceConfig with sensible defaults.
 * Override any field after creation for specific test scenarios.
 */
FRaceConfig CreateRaceConfig(
	int32 NumLaps = 3,
	const FString& TrackName = TEXT("Test Track"),
	int32 MaxAIOpponents = 7,
	float CountdownDuration = 5.0f
);

/**
 * Create a sprint race config: 1 lap, short countdown.
 */
FRaceConfig CreateSprintConfig(const FString& TrackName = TEXT("Sprint Track"));

/**
 * Create an endurance race config: 10 laps, max opponents.
 */
FRaceConfig CreateEnduranceConfig(const FString& TrackName = TEXT("Endurance Track"));

/**
 * Create a FRacerData entry for a racer.
 */
FRacerData CreateRacerData(
	int32 RacerID,
	const FString& DisplayName,
	bool bIsPlayer = false,
	int32 Position = 0
);

/**
 * Create a FRaceSessionResult for testing progression/statistics.
 * Produces a realistic result with configurable position and lap count.
 */
FRaceSessionResult CreateSessionResult(
	const FString& TrackName = TEXT("Test Track"),
	const FString& VehicleName = TEXT("NIO EP9"),
	int32 FinalPosition = 1,
	int32 TotalRacers = 4,
	int32 NumLaps = 3
);

/**
 * Create a FChampionshipData for testing championship flows.
 * Populates tracks, points table, and standings skeleton.
 */
FChampionshipData CreateChampionshipData(
	const FString& ID = TEXT("test_championship"),
	const FString& Name = TEXT("Test Championship"),
	EChampionshipTier Tier = EChampionshipTier::Beginner,
	int32 NumRaces = 3
);

/**
 * Fully set up a RaceManager with player and AI racers, ready for racing.
 * Returns the RaceManager. OutPlayerPawn receives the player pawn pointer.
 * OutAIPawns receives the AI pawn pointers.
 */
ARaceManager* CreateConfiguredRaceManager(
	const FRaceConfig& Config,
	int32 AICount = 3,
	APawn** OutPlayerPawn = nullptr,
	TArray<APawn*>* OutAIPawns = nullptr
);

// ============================================================================
// Game State Validation Utilities
// ============================================================================

/**
 * Validate that a FVehicleState has sensible values.
 * Returns true if all checks pass. Logs failures via the provided FAutomationTestBase.
 */
bool ValidateVehicleState(
	FAutomationTestBase* Test,
	const FVehicleState& State,
	const FString& Context = TEXT("")
);

/**
 * Validate that a VehicleStateManager is correctly initialized.
 * Checks initial defaults: zero speed, grounded, full battery, etc.
 */
bool ValidateVehicleStateManagerDefaults(
	FAutomationTestBase* Test,
	UVehicleStateManager* StateManager,
	const FString& Context = TEXT("")
);

/**
 * Validate that a FNIOPerformanceConfig has physically plausible values.
 * Checks mass > 0, power > 0, top speed > 0, etc.
 */
bool ValidatePerformanceConfig(
	FAutomationTestBase* Test,
	const FNIOPerformanceConfig& Config,
	ENIOVehicleType ExpectedType,
	const FString& Context = TEXT("")
);

/**
 * Validate FRacerData consistency.
 * Checks that player flag, display name, and initial values are correct.
 */
bool ValidateRacerData(
	FAutomationTestBase* Test,
	const FRacerData& Data,
	const FString& ExpectedName,
	bool bExpectedIsPlayer,
	const FString& Context = TEXT("")
);

/**
 * Validate that a RaceManager is in the expected state.
 * Checks race state, racer count, and config.
 */
bool ValidateRaceState(
	FAutomationTestBase* Test,
	ARaceManager* RaceManager,
	ERaceState ExpectedState,
	int32 ExpectedRacerCount = -1,
	const FString& Context = TEXT("")
);

/**
 * Validate that a FRaceSessionResult has consistent data.
 * Checks position bounds, lap times, and non-negative values.
 */
bool ValidateSessionResult(
	FAutomationTestBase* Test,
	const FRaceSessionResult& Result,
	const FString& Context = TEXT("")
);

/**
 * Validate that a FChampionshipData is structurally valid.
 * Checks tracks exist, points table populated, standings consistent.
 */
bool ValidateChampionshipData(
	FAutomationTestBase* Test,
	const FChampionshipData& Data,
	const FString& Context = TEXT("")
);

/**
 * Validate tire state for a single wheel.
 * Checks slip ratio bounds, thermal state plausibility, wear factor range.
 */
bool ValidateTireState(
	FAutomationTestBase* Test,
	const FTireState& State,
	const FString& Context = TEXT("")
);

/**
 * Validate that a FTireModelPreset has physically reasonable values.
 * Checks Pacejka coefficient ranges, dimensions, and thermal thresholds.
 */
bool ValidateTirePreset(
	FAutomationTestBase* Test,
	const FTireModelPreset& Preset,
	const FString& Context = TEXT("")
);

} // namespace NomiTestUtils
