// NomiRacingPlus - Race Settings Widget

#include "UI/RaceSettingsWidget.h"
#include "UI/MenuManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void URaceSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartRaceButton)
	{
		StartRaceButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnStartRace);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnBack);
	}

	if (AIPlusButton)
	{
		AIPlusButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnAIPlus);
	}

	if (AIMinusButton)
	{
		AIMinusButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnAIMinus);
	}

	if (DifficultyEasyBtn)
	{
		DifficultyEasyBtn->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnDifficultyClickedEasy);
	}

	if (DifficultyMedBtn)
	{
		DifficultyMedBtn->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnDifficultyClickedMed);
	}

	if (DifficultyHardBtn)
	{
		DifficultyHardBtn->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnDifficultyClickedHard);
	}

	if (DifficultyExpertBtn)
	{
		DifficultyExpertBtn->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnDifficultyClickedExpert);
	}

	if (LapsPlusButton)
	{
		LapsPlusButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnLapsPlus);
	}

	if (LapsMinusButton)
	{
		LapsMinusButton->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnLapsMinus);
	}

	if (WeatherNextBtn)
	{
		WeatherNextBtn->OnClicked.AddDynamic(this, &URaceSettingsWidget::OnWeatherNext);
	}

	UpdateDisplay();
}

void URaceSettingsWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void URaceSettingsWidget::OnAIPlus()
{
	int32 MaxAI = 15;

	// Baja mode caps AI opponents at 8
	if (MenuManager && MenuManager->GetMenuContext().GameMode == TEXT("Baja"))
	{
		MaxAI = 8;
	}

	AICount = FMath::Clamp(AICount + 1, 0, MaxAI);
	UpdateDisplay();
}

void URaceSettingsWidget::OnAIMinus()
{
	AICount = FMath::Clamp(AICount - 1, 0, 15);
	UpdateDisplay();
}

void URaceSettingsWidget::OnDifficultyClickedEasy()
{
	OnDifficultyClicked(0);
}

void URaceSettingsWidget::OnDifficultyClickedMed()
{
	OnDifficultyClicked(1);
}

void URaceSettingsWidget::OnDifficultyClickedHard()
{
	OnDifficultyClicked(2);
}

void URaceSettingsWidget::OnDifficultyClickedExpert()
{
	OnDifficultyClicked(3);
}

void URaceSettingsWidget::OnDifficultyClicked(int32 InDifficulty)
{
	DifficultyIndex = FMath::Clamp(InDifficulty, 0, 3);
	UpdateDisplay();
}

void URaceSettingsWidget::OnLapsPlus()
{
	NumLaps = FMath::Clamp(NumLaps + 1, 1, 10);
	UpdateDisplay();
}

void URaceSettingsWidget::OnLapsMinus()
{
	NumLaps = FMath::Clamp(NumLaps - 1, 1, 10);
	UpdateDisplay();
}

void URaceSettingsWidget::OnWeatherNext()
{
	WeatherIndex = (WeatherIndex + 1) % 3;
	UpdateDisplay();
}

void URaceSettingsWidget::OnStartRace()
{
	if (MenuManager)
	{
		// Convert difficulty index to 0-100 range for MenuManager
		int32 DifficultyValue = 0;
		switch (DifficultyIndex)
		{
		case 0: DifficultyValue = 25;  break; // EASY
		case 1: DifficultyValue = 50;  break; // NORMAL
		case 2: DifficultyValue = 75;  break; // HARD
		case 3: DifficultyValue = 100; break; // EXPERT
		default: DifficultyValue = 50; break;
		}

		MenuManager->SetRaceSettings(AICount, DifficultyValue, NumLaps, WeatherIndex);
		MenuManager->StartRace();
	}
}

void URaceSettingsWidget::OnBack()
{
	if (MenuManager)
	{
		MenuManager->ReturnToPrevious();
	}
}

void URaceSettingsWidget::UpdateDisplay()
{
	if (AICountText)
	{
		AICountText->SetText(FText::AsNumber(AICount));
	}

	if (DifficultyText)
	{
		static const TCHAR* DifficultyNames[] = { TEXT("EASY"), TEXT("NORMAL"), TEXT("HARD"), TEXT("EXPERT") };
		DifficultyText->SetText(FText::FromString(DifficultyNames[FMath::Clamp(DifficultyIndex, 0, 3)]));
	}

	// Hide laps in Baja mode (point-to-point races)
	const bool bIsBaja = MenuManager && MenuManager->GetMenuContext().GameMode == TEXT("Baja");

	if (LapsText)
	{
		if (bIsBaja)
		{
			LapsText->SetText(FText::FromString(TEXT("N/A")));
		}
		else
		{
			LapsText->SetText(FText::AsNumber(NumLaps));
		}
	}

	if (LapsPlusButton)
	{
		LapsPlusButton->SetIsEnabled(!bIsBaja);
	}

	if (LapsMinusButton)
	{
		LapsMinusButton->SetIsEnabled(!bIsBaja);
	}

	if (WeatherText)
	{
		static const TCHAR* WeatherNames[] = { TEXT("Clear"), TEXT("Dusk"), TEXT("Night") };
		WeatherText->SetText(FText::FromString(WeatherNames[FMath::Clamp(WeatherIndex, 0, 2)]));
	}
}
