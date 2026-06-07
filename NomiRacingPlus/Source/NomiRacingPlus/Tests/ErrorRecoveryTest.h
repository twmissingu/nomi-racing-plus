// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// ErrorRecoveryWidget Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorRecoveryActionEnumTest,
	"NomiRacingPlus.ErrorRecovery.ActionEnum",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorRecoveryDelegateTest,
	"NomiRacingPlus.ErrorRecovery.Delegate",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorRecoveryDialogStateTest,
	"NomiRacingPlus.ErrorRecovery.DialogState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// AudioManager Volume Persistence Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudioManagerVolumePersistenceTest,
	"NomiRacingPlus.AudioManager.VolumePersistence",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAudioManagerVolumeClampTest,
	"NomiRacingPlus.AudioManager.VolumeClamp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
