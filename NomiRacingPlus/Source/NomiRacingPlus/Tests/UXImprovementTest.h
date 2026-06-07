// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// ============================================================================
// GarageWidget Empty State Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGarageEmptyStateTest,
	"NomiRacingPlus.Garage.EmptyState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGarageEmptyStateButtonsDisabledTest,
	"NomiRacingPlus.Garage.EmptyStateButtonsDisabled",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGarageSingleVehicleTest,
	"NomiRacingPlus.Garage.SingleVehicle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGarageNIOFilterTest,
	"NomiRacingPlus.Garage.NIOFilter",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ============================================================================
// SettingsWidget Dirty State Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSettingsDirtyStateTest,
	"NomiRacingPlus.Settings.DirtyState",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSettingsDirtyStateClearOnApplyTest,
	"NomiRacingPlus.Settings.DirtyStateClearOnApply",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSettingsDirtyStateFloatToleranceTest,
	"NomiRacingPlus.Settings.DirtyStateFloatTolerance",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ============================================================================
// PauseMenu Confirmation Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPauseMenuConfirmDialogTest,
	"NomiRacingPlus.PauseMenu.ConfirmDialog",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPauseMenuConfirmCancelTest,
	"NomiRacingPlus.PauseMenu.ConfirmCancel",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPauseMenuConfirmNoActionTest,
	"NomiRacingPlus.PauseMenu.ConfirmNoAction",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPauseMenuDoubleConfirmTest,
	"NomiRacingPlus.PauseMenu.DoubleConfirm",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
