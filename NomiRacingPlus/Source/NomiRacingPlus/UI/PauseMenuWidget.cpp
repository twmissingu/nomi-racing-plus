/**
 * PauseMenuWidget.cpp
 * UI widget for the in-game pause menu.
 */

#include "UI/PauseMenuWidget.h"
#include "UI/MenuManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "NomiRacingPlus.h"

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

	// Unpause the game
	UGameplayStatics::SetGamePaused(this, false);

	// Update MenuManager state back to Racing
	if (MenuManager)
	{
		MenuManager->ReturnToPrevious();
	}
}

void UPauseMenuWidget::OnRestart()
{
	// TODO: Show confirmation dialog via NomiPlayerController (requires World context)
	// For now, execute immediately to avoid blocking the user
	RemoveFromParent();
	UGameplayStatics::SetGamePaused(this, false);

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
	if (MenuManager)
	{
		MenuManager->ShowSettings();
	}
}

void UPauseMenuWidget::OnMainMenu()
{
	// TODO: Show confirmation dialog via NomiPlayerController (requires World context)
	// For now, execute immediately to avoid blocking the user
	RemoveFromParent();
	UGameplayStatics::SetGamePaused(this, false);

	if (MenuManager)
	{
		MenuManager->ShowMainMenu();
	}
}

// ---------------------------------------------------------------------------
// Confirmation Dialog
// ---------------------------------------------------------------------------

void UPauseMenuWidget::ShowConfirmDialog(const FString& Message, EConfirmAction Action)
{
	if (bShowingConfirmDialog)
	{
		return;
	}

	bShowingConfirmDialog = true;
	PendingConfirmAction = Action;

	// State machine only: visual display is handled by NomiPlayerController
	// which creates the confirmation dialog as a separate widget and adds it to viewport.
	UE_LOG(LogNomiRacing, Log, TEXT("PauseMenu confirm dialog shown: %s"), *Message);
}

void UPauseMenuWidget::OnConfirmClicked()
{
	bShowingConfirmDialog = false;

	// Execute pending action based on enum
	switch (PendingConfirmAction)
	{
	case EConfirmAction::Restart:
	{
		RemoveFromParent();
		UGameplayStatics::SetGamePaused(this, false);

		UWorld* World = GetWorld();
		if (World)
		{
			AGameModeBase* GM = World->GetAuthGameMode();
			if (GM)
			{
				GM->ResetLevel();
			}
		}
		break;
	}

	case EConfirmAction::ReturnToMainMenu:
	{
		RemoveFromParent();
		UGameplayStatics::SetGamePaused(this, false);

		if (MenuManager)
		{
			MenuManager->ShowMainMenu();
		}
		break;
	}

	default:
		break;
	}

	PendingConfirmAction = EConfirmAction::None;
}

void UPauseMenuWidget::OnCancelClicked()
{
	bShowingConfirmDialog = false;
	PendingConfirmAction = EConfirmAction::None;
}
