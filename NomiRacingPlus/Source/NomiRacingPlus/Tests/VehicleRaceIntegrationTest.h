// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Race/RaceManager.h"
#include "Vehicles/VehicleStateManager.h"

/**
 * Vehicle-Race Interaction Integration Tests
 * Tests the full lifecycle of vehicles interacting with the race system:
 * registration, checkpoint progression, position tracking, lap completion,
 * and race event broadcasting.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleRegistrationIntegrationTest,
	"NomiRacingPlus.Integration.VehicleRace.Registration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleCheckpointProgressionTest,
	"NomiRacingPlus.Integration.VehicleRace.CheckpointProgression",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehiclePositionTrackingTest,
	"NomiRacingPlus.Integration.VehicleRace.PositionTracking",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleLapCompletionTest,
	"NomiRacingPlus.Integration.VehicleRace.LapCompletion",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleRaceEventBroadcastTest,
	"NomiRacingPlus.Integration.VehicleRace.EventBroadcast",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMultiVehicleRaceLifecycleTest,
	"NomiRacingPlus.Integration.VehicleRace.MultiVehicleLifecycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleStateManagerRaceDataTest,
	"NomiRacingPlus.Integration.VehicleRace.StateManagerData",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
