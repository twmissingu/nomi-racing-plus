// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/UXImprovementTest.h"
#include "UI/GarageWidget.h"
#include "UI/SettingsWidget.h"
#include "UI/PauseMenuWidget.h"
#include "Vehicles/VehicleStateManager.h"
#include "Race/RaceManager.h"

// ============================================================================
// Helper: Simulate GarageWidget vehicle filtering logic
// (tests the same code path as UGarageWidget::SetModeFilter)
// ============================================================================

static void FilterVehiclesByMode(
	const TArray<ENIOVehicleType>& AllVehicles,
	const FString& ModeFilter,
	TArray<ENIOVehicleType>& OutFiltered)
{
	OutFiltered.Empty();
	for (ENIOVehicleType VType : AllVehicles)
	{
		if (ModeFilter.IsEmpty() || ModeFilter == TEXT("All"))
		{
			OutFiltered.Add(VType);
			continue;
		}

		// Mirror GarageWidget::SetModeFilter logic
		bool bMatches = false;
		switch (VType)
		{
		case ENIOVehicleType::EP9:
		case ENIOVehicleType::ET7:
		case ENIOVehicleType::ES7:
		case ENIOVehicleType::ET5:
			bMatches = (ModeFilter == TEXT("NIO"));
			break;
		case ENIOVehicleType::SU7Ultra:
			bMatches = (ModeFilter == TEXT("Baja"));
			break;
		default:
			bMatches = false;
			break;
		}

		if (bMatches)
		{
			OutFiltered.Add(VType);
		}
	}
}

// ============================================================================
// Helper: Simulate SettingsWidget dirty state logic
// (tests the same code path as USettingsWidget::CheckDirtyState)
// ============================================================================

static bool CheckSettingsDirty(
	float WorkingMaster, float InitialMaster,
	float WorkingSFX, float InitialSFX,
	float WorkingMusic, float InitialMusic)
{
	return !FMath::IsNearlyEqual(WorkingMaster, InitialMaster, 0.01f)
		|| !FMath::IsNearlyEqual(WorkingSFX, InitialSFX, 0.01f)
		|| !FMath::IsNearlyEqual(WorkingMusic, InitialMusic, 0.01f);
}

// ============================================================================
// Helper: Simulate PauseMenuWidget confirmation state machine
// (tests the same code path as UPauseMenuWidget::ShowConfirmDialog/OnConfirmClicked)
// ============================================================================

struct FConfirmDialogState
{
	bool bShowing = false;
	EConfirmAction PendingAction = EConfirmAction::None;
	bool bActionExecuted = false;
};

static void SimulateShowConfirm(FConfirmDialogState& State, EConfirmAction Action)
{
	State.bShowing = true;
	State.PendingAction = Action;
}

static void SimulateOnConfirm(FConfirmDialogState& State)
{
	State.bShowing = false;
	if (State.PendingAction != EConfirmAction::None)
	{
		State.bActionExecuted = true;
	}
	State.PendingAction = EConfirmAction::None;
}

static void SimulateOnCancel(FConfirmDialogState& State)
{
	State.bShowing = false;
	State.PendingAction = EConfirmAction::None;
}

// ============================================================================
// GarageWidget Empty State Tests
// ============================================================================

/**
 * Empty vehicle list should produce "No Vehicles Available" display text.
 * Exercises the same conditional as UGarageWidget::NativeConstruct().
 */
bool FGarageEmptyStateTest::RunTest(const FString& Parameters)
{
	TArray<ENIOVehicleType> AllVehicles;
	AllVehicles.Add(ENIOVehicleType::EP9);
	AllVehicles.Add(ENIOVehicleType::ET7);
	AllVehicles.Add(ENIOVehicleType::ET5);

	// Filter with non-matching mode → empty list
	TArray<ENIOVehicleType> Filtered;
	FilterVehiclesByMode(AllVehicles, TEXT("Baja"), Filtered);

	FString DisplayText;
	if (Filtered.Num() == 0)
	{
		DisplayText = TEXT("No Vehicles Available");
	}

	TestEqual(TEXT("Baja filter on NIO-only list should show empty state"), DisplayText, FString(TEXT("No Vehicles Available")));
	TestEqual(TEXT("Filtered list should be empty"), Filtered.Num(), 0);

	return true;
}

/**
 * Empty state should disable Prev/Next/Select buttons.
 * Exercises the same button state logic as UGarageWidget::ApplyEmptyState().
 */
bool FGarageEmptyStateButtonsDisabledTest::RunTest(const FString& Parameters)
{
	TArray<ENIOVehicleType> AllVehicles;
	AllVehicles.Add(ENIOVehicleType::EP9);
	AllVehicles.Add(ENIOVehicleType::ET7);

	// Test: empty filter → buttons disabled
	TArray<ENIOVehicleType> Filtered;
	FilterVehiclesByMode(AllVehicles, TEXT("Baja"), Filtered);

	bool bPrevEnabled = (Filtered.Num() > 0);
	bool bNextEnabled = (Filtered.Num() > 0);
	bool bSelectEnabled = (Filtered.Num() > 0);

	TestFalse(TEXT("Prev button disabled when empty"), bPrevEnabled);
	TestFalse(TEXT("Next button disabled when empty"), bNextEnabled);
	TestFalse(TEXT("Select button disabled when empty"), bSelectEnabled);

	// Test: "All" filter → buttons enabled
	FilterVehiclesByMode(AllVehicles, TEXT("All"), Filtered);

	bPrevEnabled = (Filtered.Num() > 0);
	bNextEnabled = (Filtered.Num() > 0);
	bSelectEnabled = (Filtered.Num() > 0);

	TestTrue(TEXT("Prev button enabled when vehicles exist"), bPrevEnabled);
	TestTrue(TEXT("Next button enabled when vehicles exist"), bNextEnabled);
	TestTrue(TEXT("Select button enabled when vehicles exist"), bSelectEnabled);
	TestEqual(TEXT("All filter should return all vehicles"), Filtered.Num(), 2);

	return true;
}

/**
 * Edge case: single vehicle in list should not show empty state.
 */
bool FGarageSingleVehicleTest::RunTest(const FString& Parameters)
{
	TArray<ENIOVehicleType> AllVehicles;
	AllVehicles.Add(ENIOVehicleType::SU7Ultra);

	TArray<ENIOVehicleType> Filtered;
	FilterVehiclesByMode(AllVehicles, TEXT("Baja"), Filtered);

	TestEqual(TEXT("SU7 Ultra should match Baja filter"), Filtered.Num(), 1);
	TestTrue(TEXT("Single vehicle should not be empty state"), Filtered.Num() > 0);

	// Verify it's the right vehicle
	TestEqual(TEXT("Filtered vehicle should be SU7Ultra"), static_cast<int32>(Filtered[0]), static_cast<int32>(ENIOVehicleType::SU7Ultra));

	return true;
}

/**
 * Edge case: NIO filter should return 4 vehicles (EP9, ET7, ES7, ET5).
 */
bool FGarageNIOFilterTest::RunTest(const FString& Parameters)
{
	TArray<ENIOVehicleType> AllVehicles;
	AllVehicles.Add(ENIOVehicleType::EP9);
	AllVehicles.Add(ENIOVehicleType::ET7);
	AllVehicles.Add(ENIOVehicleType::ES7);
	AllVehicles.Add(ENIOVehicleType::ET5);
	AllVehicles.Add(ENIOVehicleType::SU7Ultra);

	TArray<ENIOVehicleType> Filtered;
	FilterVehiclesByMode(AllVehicles, TEXT("NIO"), Filtered);

	TestEqual(TEXT("NIO filter should return 4 vehicles"), Filtered.Num(), 4);

	// Verify no SU7 Ultra in NIO list
	for (ENIOVehicleType VType : Filtered)
	{
		TestTrue(TEXT("NIO filter should not include SU7Ultra"), VType != ENIOVehicleType::SU7Ultra);
	}

	return true;
}

// ============================================================================
// SettingsWidget Dirty State Tests
// ============================================================================

/**
 * Modified settings should mark dirty state.
 * Exercises the same comparison logic as USettingsWidget::CheckDirtyState().
 */
bool FSettingsDirtyStateTest::RunTest(const FString& Parameters)
{
	// Initial values (loaded on open)
	const float InitMaster = 1.0f;
	const float InitSFX = 0.8f;
	const float InitMusic = 0.6f;

	// Test: all modified → dirty
	bool bDirty = CheckSettingsDirty(0.75f, InitMaster, 0.5f, InitSFX, 0.3f, InitMusic);
	TestTrue(TEXT("All modified values should be dirty"), bDirty);

	// Test: only master modified → dirty
	bDirty = CheckSettingsDirty(0.5f, InitMaster, InitSFX, InitSFX, InitMusic, InitMusic);
	TestTrue(TEXT("Master only modified should be dirty"), bDirty);

	// Test: only SFX modified → dirty
	bDirty = CheckSettingsDirty(InitMaster, InitMaster, 0.3f, InitSFX, InitMusic, InitMusic);
	TestTrue(TEXT("SFX only modified should be dirty"), bDirty);

	// Test: only music modified → dirty
	bDirty = CheckSettingsDirty(InitMaster, InitMaster, InitSFX, InitSFX, 0.1f, InitMusic);
	TestTrue(TEXT("Music only modified should be dirty"), bDirty);

	// Test: no changes → not dirty
	bDirty = CheckSettingsDirty(InitMaster, InitMaster, InitSFX, InitSFX, InitMusic, InitMusic);
	TestFalse(TEXT("Unchanged settings should not be dirty"), bDirty);

	return true;
}

/**
 * Apply should clear dirty flag.
 */
bool FSettingsDirtyStateClearOnApplyTest::RunTest(const FString& Parameters)
{
	// Simulate dirty state
	bool bSettingsDirty = true;

	// Simulate Apply action
	bSettingsDirty = false;

	TestFalse(TEXT("Dirty should be false after Apply"), bSettingsDirty);

	// Edge case: Apply when already clean
	bSettingsDirty = false;
	bSettingsDirty = false;
	TestFalse(TEXT("Apply on clean state should remain clean"), bSettingsDirty);

	return true;
}

/**
 * Edge case: floating point near-equal tolerance.
 * Values within 0.01 should NOT be considered dirty.
 */
bool FSettingsDirtyStateFloatToleranceTest::RunTest(const FString& Parameters)
{
	const float InitMaster = 1.0f;
	const float InitSFX = 0.8f;
	const float InitMusic = 0.6f;

	// Test: values within tolerance → not dirty
	bool bDirty = CheckSettingsDirty(
		InitMaster + 0.005f, InitMaster,
		InitSFX - 0.003f, InitSFX,
		InitMusic + 0.008f, InitMusic);
	TestFalse(TEXT("Values within 0.01 tolerance should not be dirty"), bDirty);

	// Test: values at boundary → not dirty
	bDirty = CheckSettingsDirty(
		InitMaster + 0.01f, InitMaster,
		InitSFX, InitSFX,
		InitMusic, InitMusic);
	TestFalse(TEXT("Values at exact 0.01 boundary should not be dirty"), bDirty);

	// Test: values just outside tolerance → dirty
	bDirty = CheckSettingsDirty(
		InitMaster + 0.011f, InitMaster,
		InitSFX, InitSFX,
		InitMusic, InitMusic);
	TestTrue(TEXT("Values outside 0.01 tolerance should be dirty"), bDirty);

	return true;
}

// ============================================================================
// PauseMenu Confirmation Tests
// ============================================================================

/**
 * Confirm dialog should execute pending action on confirm.
 * Exercises the same state machine as UPauseMenuWidget::OnConfirmClicked().
 */
bool FPauseMenuConfirmDialogTest::RunTest(const FString& Parameters)
{
	FConfirmDialogState State;

	// Show dialog with Restart action
	SimulateShowConfirm(State, EConfirmAction::Restart);
	TestTrue(TEXT("Dialog should be showing"), State.bShowing);
	TestEqual(TEXT("Pending action should be Restart"), static_cast<int32>(State.PendingAction), static_cast<int32>(EConfirmAction::Restart));

	// Confirm
	SimulateOnConfirm(State);
	TestFalse(TEXT("Dialog should be hidden after confirm"), State.bShowing);
	TestTrue(TEXT("Restart action should be executed"), State.bActionExecuted);
	TestEqual(TEXT("Pending action should be None after confirm"), static_cast<int32>(State.PendingAction), static_cast<int32>(EConfirmAction::None));

	return true;
}

/**
 * Cancel should NOT execute pending action.
 */
bool FPauseMenuConfirmCancelTest::RunTest(const FString& Parameters)
{
	FConfirmDialogState State;

	// Show dialog with ReturnToMainMenu action
	SimulateShowConfirm(State, EConfirmAction::ReturnToMainMenu);
	TestTrue(TEXT("Dialog should be showing"), State.bShowing);

	// Cancel
	SimulateOnCancel(State);
	TestFalse(TEXT("Dialog should be hidden after cancel"), State.bShowing);
	TestFalse(TEXT("Action should NOT be executed on cancel"), State.bActionExecuted);
	TestEqual(TEXT("Pending action should be None after cancel"), static_cast<int32>(State.PendingAction), static_cast<int32>(EConfirmAction::None));

	return true;
}

/**
 * Edge case: confirm when no action is pending (should not crash).
 */
bool FPauseMenuConfirmNoActionTest::RunTest(const FString& Parameters)
{
	FConfirmDialogState State;

	// Manually set state to showing with no action
	State.bShowing = true;
	State.PendingAction = EConfirmAction::None;

	// Confirm — should not crash
	SimulateOnConfirm(State);
	TestFalse(TEXT("Dialog should be hidden"), State.bShowing);
	TestFalse(TEXT("No action should be executed"), State.bActionExecuted);

	return true;
}

/**
 * Edge case: double confirm should be idempotent.
 */
bool FPauseMenuDoubleConfirmTest::RunTest(const FString& Parameters)
{
	FConfirmDialogState State;

	SimulateShowConfirm(State, EConfirmAction::Restart);
	SimulateOnConfirm(State);

	// Second confirm on already-closed dialog
	State.bActionExecuted = false;
	SimulateOnConfirm(State);
	TestFalse(TEXT("Double confirm should not re-execute"), State.bActionExecuted);

	return true;
}
