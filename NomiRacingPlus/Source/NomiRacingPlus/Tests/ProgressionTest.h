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
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionStatisticsTest,
	"NomiRacingPlus.Progression.Statistics",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionChampionshipTest,
	"NomiRacingPlus.Progression.Championship",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionUnlockablesTest,
	"NomiRacingPlus.Progression.Unlockables",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSessionRecordingTest,
	"NomiRacingPlus.Progression.SessionRecording",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionStreakTest,
	"NomiRacingPlus.Progression.Streaks",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionTrackStatsTest,
	"NomiRacingPlus.Progression.TrackStats",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionVehicleStatsTest,
	"NomiRacingPlus.Progression.VehicleStats",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
