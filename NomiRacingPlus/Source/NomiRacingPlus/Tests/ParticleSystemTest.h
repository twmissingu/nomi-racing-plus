// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// ParticleSystem Quality Test — verify quality level mapping
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FParticleSystemQualityTest,
	"NomiRacingPlus.ParticleSystem.Quality",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ParticleSystem Multiplier Test — verify spawn/size multiplier values
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FParticleSystemMultiplierTest,
	"NomiRacingPlus.ParticleSystem.Multiplier",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ParticleSystem Threshold Test — verify effect trigger thresholds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FParticleSystemThresholdTest,
	"NomiRacingPlus.ParticleSystem.Threshold",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// TrackSelectWidget Empty State Test — verify button disable/enable logic
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTrackSelectEmptyStateTest,
	"NomiRacingPlus.TrackSelect.EmptyState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ErrorToastWidget Severity Prefix Test — verify user-friendly prefix symbols
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FToastSeverityPrefixTest,
	"NomiRacingPlus.Toast.SeverityPrefix",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
