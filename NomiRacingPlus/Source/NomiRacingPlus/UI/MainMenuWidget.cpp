// NomiRacingPlus - Main Menu Widget

#include "UI/MainMenuWidget.h"
#include "UI/MenuManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GTModeButton)
	{
		GTModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnGTModeClicked);
	}

	if (NIOModeButton)
	{
		NIOModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNIOModeClicked);
	}

	if (BajaModeButton)
	{
		BajaModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBajaModeClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}
}

void UMainMenuWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UMainMenuWidget::OnGTModeClicked()
{
	if (MenuManager)
	{
		MenuManager->SetGameMode(TEXT("GT"));
		MenuManager->ShowGarage();
	}
}

void UMainMenuWidget::OnNIOModeClicked()
{
	if (MenuManager)
	{
		MenuManager->SetGameMode(TEXT("NIO"));
		MenuManager->ShowGarage();
	}
}

void UMainMenuWidget::OnBajaModeClicked()
{
	if (MenuManager)
	{
		MenuManager->SetGameMode(TEXT("Baja"));
		MenuManager->ShowGarage();
	}
}

void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget: Settings clicked - not yet implemented"));
}

void UMainMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		nullptr,
		EQuitPreference::Quit,
		false
	);
}
