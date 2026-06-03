/**
 * PauseMenuWidget.cpp
 * UI widget for the in-game pause menu.
 */

#include "UI/PauseMenuWidget.h"
#include "UI/MenuManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

void UPauseMenuWidget::SetMenuManager(UMenuManager* InMenuManager)
{
	MenuManager = InMenuManager;
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnContinue);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestart);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettings);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenu);
	}

	if (PauseTitleText)
	{
		PauseTitleText->SetText(FText::FromString(TEXT("Paused")));
	}
}

void UPauseMenuWidget::OnContinue()
{
	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UPauseMenuWidget::OnRestart()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GM = World->GetAuthGameMode();
		if (GM)
		{
			GM->ResetLevel();
		}
	}
}

void UPauseMenuWidget::OnSettings()
{
	// Placeholder: integrate settings menu here when available.
	UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnSettings - Settings not yet implemented."));
}

void UPauseMenuWidget::OnMainMenu()
{
	RemoveFromParent();

	if (MenuManager)
	{
		MenuManager->ShowMainMenu();
	}
}
