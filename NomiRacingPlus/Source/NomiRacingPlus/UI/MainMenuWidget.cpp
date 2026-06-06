// NomiRacingPlus - Main Menu Widget

#include "UI/MainMenuWidget.h"
#include "UI/MenuManager.h"
#include "UI/TutorialManager.h"
#include "Core/NomiGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GTModeButton)
	{
		GTModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnGTModeClicked);
		GTModeButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnGTModeHovered);
		GTModeButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnModeUnhovered);
	}

	if (NIOModeButton)
	{
		NIOModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNIOModeClicked);
		NIOModeButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnNIOModeHovered);
		NIOModeButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnModeUnhovered);
	}

	if (BajaModeButton)
	{
		BajaModeButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBajaModeClicked);
		BajaModeButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnBajaModeHovered);
		BajaModeButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnModeUnhovered);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}

	if (TutorialButton)
	{
		TutorialButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTutorialClicked);
	}

	// Clear description initially
	SetModeDescription(TEXT(""));

	// Auto-start tutorial on first launch (no save file detected)
	if (GetWorld())
	{
		UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetWorld()->GetGameInstance());
		if (GI && GI->ShouldAutoStartTutorial())
		{
			// Defer to next frame to ensure all initialization is complete
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, GI]()
			{
				if (TutorialManager && !TutorialManager->IsTutorialComplete())
				{
					TutorialManager->StartTutorial();
					GI->ClearAutoStartTutorialFlag();
				}
			}, 0.1f, false);
		}
	}
}

void UMainMenuWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UMainMenuWidget::SetTutorialManager(UTutorialManager* Manager)
{
	TutorialManager = Manager;
}

void UMainMenuWidget::AutoStartTutorialIfNeeded()
{
	// Auto-start tutorial if NomiGameInstance flagged it (first launch, no save file)
	if (GetWorld())
	{
		UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetWorld()->GetGameInstance());
		if (GI && GI->ShouldAutoStartTutorial())
		{
			if (TutorialManager && !TutorialManager->IsTutorialComplete())
			{
				TutorialManager->StartTutorial();
				GI->ClearAutoStartTutorialFlag();
			}
		}
	}
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
	if (MenuManager)
	{
		MenuManager->ShowSettings();
	}
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

void UMainMenuWidget::OnTutorialClicked()
{
	if (TutorialManager)
	{
		TutorialManager->StartTutorial();
	}
}

void UMainMenuWidget::OnGTModeHovered()
{
	SetModeDescription(TEXT("GT Mode — Classic grand touring racing. Balanced performance across all NIO vehicles on paved circuits."));
}

void UMainMenuWidget::OnNIOModeHovered()
{
	SetModeDescription(TEXT("NIO Mode — Electric vehicle showcase. Experience the full lineup of NIO electric cars with authentic EV physics and NOMI companion."));
}

void UMainMenuWidget::OnBajaModeHovered()
{
	SetModeDescription(TEXT("Baja Mode — Off-road rally racing. Sand dunes, rocky terrain, and unpredictable conditions. Only available vehicles can participate."));
}

void UMainMenuWidget::OnModeUnhovered()
{
	SetModeDescription(TEXT(""));
}

void UMainMenuWidget::SetModeDescription(const FString& Text)
{
	if (ModeDescriptionText)
	{
		ModeDescriptionText->SetText(FText::FromString(Text));
	}
}
