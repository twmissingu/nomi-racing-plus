// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/GarageWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/MenuManager.h"
#include "Vehicles/VehicleStateManager.h"

void UGarageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrevVehicleButton)
	{
		PrevVehicleButton->OnClicked.AddDynamic(this, &UGarageWidget::OnPrevClicked);
	}

	if (NextVehicleButton)
	{
		NextVehicleButton->OnClicked.AddDynamic(this, &UGarageWidget::OnNextClicked);
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UGarageWidget::OnSelectClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UGarageWidget::OnBackClicked);
	}

	if (AvailableVehicles.Num() > 0)
	{
		CurrentVehicleIndex = 0;
		UpdateVehicleDisplay();
		ApplyEmptyState(false);
	}
	else
	{
		ApplyEmptyState(true);
	}
}

void UGarageWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UGarageWidget::SetModeFilter(const FString& Mode)
{
	AvailableVehicles.Empty();

	// Mode descriptions for player guidance
	FString ModeName;
	FString ModeDescription;

	if (Mode == TEXT("NIO"))
	{
		ModeName = TEXT("NIO Championship");
		ModeDescription = TEXT("NIO-only championship featuring EP9, ET7, ES7, ET5, and SU7 Ultra. Compete on urban circuits with battery swap availability and NIO branding.");
		AvailableVehicles.Add(ENIOVehicleType::EP9);
		AvailableVehicles.Add(ENIOVehicleType::ET7);
		AvailableVehicles.Add(ENIOVehicleType::ES7);
		AvailableVehicles.Add(ENIOVehicleType::ET5);
		AvailableVehicles.Add(ENIOVehicleType::SU7Ultra);
	}
	else if (Mode == TEXT("Baja"))
	{
		ModeName = TEXT("Baja Rally");
		ModeDescription = TEXT("Off-road desert rally — ES7 only. Single point-to-point stage through sand dunes and canyons. Extreme difficulty with dust visibility and unpredictable terrain.");
		AvailableVehicles.Add(ENIOVehicleType::ES7);
	}
	else
	{
		ModeName = TEXT("Street GT");
		ModeDescription = TEXT("All vehicles welcome. Race on urban circuits, ovals, and mountain passes. Full collision enabled. The classic racing experience.");
		AvailableVehicles.Add(ENIOVehicleType::EP9);
		AvailableVehicles.Add(ENIOVehicleType::ET7);
		AvailableVehicles.Add(ENIOVehicleType::ES7);
		AvailableVehicles.Add(ENIOVehicleType::ET5);
		AvailableVehicles.Add(ENIOVehicleType::SU7Ultra);
	}

	// Display mode information
	if (ModeNameText)
	{
		ModeNameText->SetText(FText::FromString(ModeName));
	}

	if (ModeDescriptionText)
	{
		ModeDescriptionText->SetText(FText::FromString(ModeDescription));
	}

	// Restore saved selection index from MenuContext
	int32 SavedIndex = 0;
	if (MenuManager)
	{
		SavedIndex = MenuManager->GetMenuContext().VehicleIndex;
	}
	CurrentVehicleIndex = FMath::Clamp(SavedIndex, 0, FMath::Max(0, AvailableVehicles.Num() - 1));

	// Handle empty state after filtering
	if (AvailableVehicles.Num() == 0)
	{
		ApplyEmptyState(true);
	}
	else
	{
		// Re-enable buttons if vehicles are available
		ApplyEmptyState(false);
		UpdateVehicleDisplay();
	}
}

void UGarageWidget::ApplyEmptyState(bool bEmpty)
{
	if (bEmpty)
	{
		if (VehicleNameText)
		{
			VehicleNameText->SetText(FText::FromString(TEXT("No Vehicles Available")));
		}
		if (PowerText) PowerText->SetText(FText::GetEmpty());
		if (TorqueText) TorqueText->SetText(FText::GetEmpty());
		if (AccelText) AccelText->SetText(FText::GetEmpty());
		if (TopSpeedText) TopSpeedText->SetText(FText::GetEmpty());
		if (DriveTypeText) DriveTypeText->SetText(FText::GetEmpty());

		if (PrevVehicleButton) PrevVehicleButton->SetIsEnabled(false);
		if (NextVehicleButton) NextVehicleButton->SetIsEnabled(false);
		if (SelectButton) SelectButton->SetIsEnabled(false);
	}
	else
	{
		if (PrevVehicleButton) PrevVehicleButton->SetIsEnabled(true);
		if (NextVehicleButton) NextVehicleButton->SetIsEnabled(true);
		if (SelectButton) SelectButton->SetIsEnabled(true);
	}
}

void UGarageWidget::OnPrevClicked()
{
	if (AvailableVehicles.Num() == 0)
	{
		return;
	}

	CurrentVehicleIndex = (CurrentVehicleIndex - 1 + AvailableVehicles.Num()) % AvailableVehicles.Num();
	UpdateVehicleDisplay();
}

void UGarageWidget::OnNextClicked()
{
	if (AvailableVehicles.Num() == 0)
	{
		return;
	}

	CurrentVehicleIndex = (CurrentVehicleIndex + 1) % AvailableVehicles.Num();
	UpdateVehicleDisplay();
}

void UGarageWidget::OnSelectClicked()
{
	if (!MenuManager || AvailableVehicles.Num() == 0)
	{
		return;
	}

	// Save current selection to MenuContext for state preservation
	FMenuContext Ctx = MenuManager->GetMenuContext();
	Ctx.VehicleIndex = CurrentVehicleIndex;
	MenuManager->SetMenuContext(Ctx);
	MenuManager->SetVehicleType(AvailableVehicles[CurrentVehicleIndex]);
	MenuManager->ShowTrackSelect();
}

void UGarageWidget::OnBackClicked()
{
	if (!MenuManager)
	{
		return;
	}

	// Save current selection to MenuContext before navigating back
	FMenuContext Ctx = MenuManager->GetMenuContext();
	Ctx.VehicleIndex = CurrentVehicleIndex;
	MenuManager->SetMenuContext(Ctx);
	MenuManager->ReturnToPrevious();
}

void UGarageWidget::UpdateVehicleDisplay()
{
	if (AvailableVehicles.Num() == 0)
	{
		return;
	}

	const ENIOVehicleType VehicleType = AvailableVehicles[CurrentVehicleIndex];
	const FVehicleSpecs Specs = UVehicleStateManager::GetVehicleSpecs(VehicleType);

	if (VehicleNameText)
	{
		VehicleNameText->SetText(FText::FromString(Specs.VehicleName));
	}

	if (PowerText)
	{
		PowerText->SetText(FText::FromString(FString::Printf(TEXT("%.0f HP"), Specs.MaxPower)));
	}

	if (TorqueText)
	{
		TorqueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f Nm"), Specs.MaxTorque)));
	}

	if (AccelText)
	{
		AccelText->SetText(FText::FromString(FString::Printf(TEXT("%.1f s"), Specs.ZeroToHundredTime)));
	}

	if (TopSpeedText)
	{
		TopSpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.0f km/h"), Specs.TopSpeed)));
	}

	if (DriveTypeText)
	{
		DriveTypeText->SetText(FText::FromString(Specs.DrivetrainType));
	}
}
