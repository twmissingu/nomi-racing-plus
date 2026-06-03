// NIO Racing Plus - Garage Widget

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
	}
}

void UGarageWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UGarageWidget::SetModeFilter(const FString& Mode)
{
	AvailableVehicles.Empty();

	if (Mode == TEXT("NIO"))
	{
		AvailableVehicles.Add(ENIOVehicleType::EP9);
		AvailableVehicles.Add(ENIOVehicleType::ET7);
		AvailableVehicles.Add(ENIOVehicleType::ES7);
	}
	else if (Mode == TEXT("Baja"))
	{
		AvailableVehicles.Add(ENIOVehicleType::ES7);
	}
	else
	{
		AvailableVehicles.Add(ENIOVehicleType::EP9);
		AvailableVehicles.Add(ENIOVehicleType::ET7);
		AvailableVehicles.Add(ENIOVehicleType::ES7);
	}

	CurrentVehicleIndex = 0;
	UpdateVehicleDisplay();
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

	MenuManager->SetVehicleType(AvailableVehicles[CurrentVehicleIndex]);
	MenuManager->ShowTrackSelect();
}

void UGarageWidget::OnBackClicked()
{
	if (!MenuManager)
	{
		return;
	}

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
