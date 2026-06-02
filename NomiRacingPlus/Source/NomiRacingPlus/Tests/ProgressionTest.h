// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Race/RaceProgression.h"

/**
 * Progression System Tests
 * Tests for achievements, statistics, championships, and unlockables
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionAchievementTest,
	"NomiRacingPlus.Progression.Achievements",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionStatisticsTest,
	"NomiRacingPlus.Progression.Statistics",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionChampionshipTest,
	"NomiRacingPlus.Progression.Championship",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionUnlockablesTest,
	"NomiRacingPlus.Progression.Unlockables",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSessionRecordingTest,
	"NomiRacingPlus.Progression.SessionRecording",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionStreakTest,
	"NomiRacingPlus.Progression.Streaks",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionTrackStatsTest,
	"NomiRacingPlus.Progression.TrackStats",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionVehicleStatsTest,
	"NomiRacingPlus.Progression.VehicleStats",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
