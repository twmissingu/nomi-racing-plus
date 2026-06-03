// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Race/RaceManager.h"

/**
 * Race System Tests
 * Tests for race state machine, checkpoints, and timing
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRaceManagerStateMachineTest,
	"NomiRacingPlus.Race.StateMachine",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRaceCheckpointTest,
	"NomiRacingPlus.Race.Checkpoints",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRacePositionCalculationTest,
	"NomiRacingPlus.Race.Positions",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRaceTimerTest,
	"NomiRacingPlus.Race.Timer",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
