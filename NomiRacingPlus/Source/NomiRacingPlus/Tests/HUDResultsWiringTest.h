// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UI/RaceHUD.h"
#include "UI/ResultsWidget.h"
#include "Race/RaceManager.h"
#include "Vehicles/VehicleStateManager.h"
#include "Race/RaceProgressionTypes.h"

// ─── HUD Data Wiring Tests ──────────────────────────────────────────────────

/**
 * Test that FHUDData can be populated from VehicleStateManager and RaceManager data
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDDataPopulationTest,
	"NomiRacingPlus.HUDResults.HUDDataPopulation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test that RaceHUD::UpdateHUDData stores the data correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDUpdateStoresDataTest,
	"NomiRacingPlus.HUDResults.HUDUpdateStoresData",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test that RaceHUD::ShowCountdown sets countdown state and value
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDCountdownDisplayTest,
	"NomiRacingPlus.HUDResults.CountdownDisplay",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test that RaceHUD::HideCountdown transitions to Racing state
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDCountdownHideTest,
	"NomiRacingPlus.HUDResults.CountdownHide",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test countdown values: integer display, "GO!" at zero
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDCountdownValuesTest,
	"NomiRacingPlus.HUDResults.CountdownValues",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test HUD data fields for NOMI integration (comment text, emotion, visibility)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDNOMIDataTest,
	"NomiRacingPlus.HUDResults.NOMIData",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test HUD data fields for vehicle telemetry (speed, battery, drift, inputs)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDVehicleTelemetryTest,
	"NomiRacingPlus.HUDResults.VehicleTelemetry",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test HUD data fields for race state (position, lap, timer, best lap)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDRaceStateDataTest,
	"NomiRacingPlus.HUDResults.RaceStateData",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test HUD visibility toggling
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDVisibilityTest,
	"NomiRacingPlus.HUDResults.HUDVisibility",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ─── Results Widget Tests ────────────────────────────────────────────────────

/**
 * Test that FRaceSessionResult can be populated correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSessionResultPopulationTest,
	"NomiRacingPlus.HUDResults.SessionResultPopulation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test that ResultsWidget::SetResults stores MenuManager reference
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResultsWidgetMenuManagerTest,
	"NomiRacingPlus.HUDResults.ResultsWidgetMenuManager",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test ResultsWidget ordinal suffix formatting
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResultsOrdinalSuffixTest,
	"NomiRacingPlus.HUDResults.OrdinalSuffix",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test ResultsWidget time formatting
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FResultsTimeFormatTest,
	"NomiRacingPlus.HUDResults.TimeFormat",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ─── Integration Tests ──────────────────────────────────────────────────────

/**
 * Test full HUD data pipeline: VehicleStateManager state -> FHUDData -> RaceHUD
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDDataPipelineTest,
	"NomiRacingPlus.Integration.HUDResults.DataPipeline",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test RaceFinish event -> ResultsWidget creation pipeline
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRaceFinishResultsPipelineTest,
	"NomiRacingPlus.Integration.HUDResults.RaceFinishPipeline",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

/**
 * Test Baja mode HUD data (distance to finish, progress)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDBajaModeTest,
	"NomiRacingPlus.HUDResults.BajaMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
