// NomiRacingPlus - Results Widget

#include "UI/ResultsWidget.h"
#include "UI/MenuManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Race/RaceManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogNomiResults, Log, All);

void UResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RematchButton)
	{
		RematchButton->OnClicked.AddDynamic(this, &UResultsWidget::OnRematch);
	}

	if (GarageButton)
	{
		GarageButton->OnClicked.AddDynamic(this, &UResultsWidget::OnGarage);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UResultsWidget::OnMainMenu);
	}
}

void UResultsWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UResultsWidget::SetResults(const FRaceSessionResult& Result, bool bIsBajaMode)
{
	// Position: "P1", "P2", etc. with ordinal suffix
	if (PositionText)
	{
		const int32 Pos = Result.FinalPosition;
		const FString Suffix = GetOrdinalSuffix(Pos);
		PositionText->SetText(FText::FromString(FString::Printf(TEXT("P%d%s"), Pos, *Suffix)));
	}

	// Track name
	if (TrackNameText)
	{
		TrackNameText->SetText(FText::FromString(Result.TrackName));
	}

	// Total time: MM:SS.mmm
	if (TotalTimeText)
	{
		TotalTimeText->SetText(FText::FromString(FormatTime(Result.TotalRaceTime)));
	}

	// Best lap: format time, or "N/A" for Baja mode
	if (BestLapText)
	{
		if (bIsBajaMode)
		{
			BestLapText->SetText(FText::FromString(TEXT("N/A")));
		}
		else
		{
			BestLapText->SetText(FText::FromString(FormatTime(Result.BestLapTime)));
		}
	}

	// Baja mode adjustments: hide BestLap, show completion status
	if (bIsBajaMode)
	{
		if (BestLapText)
		{
			BestLapText->SetVisibility(ESlateVisibility::Collapsed);
		}

		// Show completion status
		if (GapToFirstText)
		{
			const bool bFinished = Result.FinalPosition > 0 && Result.FinalPosition <= Result.TotalRacers;
			GapToFirstText->SetText(FText::FromString(bFinished ? TEXT("100%") : TEXT("DNF")));
		}
	}

	UE_LOG(LogNomiResults, Log, TEXT("Results set: Position=%d, TotalRaceTime=%.3f, BestLap=%.3f, Baja=%d"),
		Result.FinalPosition, Result.TotalRaceTime, Result.BestLapTime, bIsBajaMode ? 1 : 0);
}

void UResultsWidget::OnRematch()
{
	if (MenuManager)
	{
		MenuManager->StartRace();
	}
}

void UResultsWidget::OnGarage()
{
	if (MenuManager)
	{
		MenuManager->ShowGarage();
	}
}

void UResultsWidget::OnMainMenu()
{
	if (MenuManager)
	{
		MenuManager->ShowMainMenu();
	}
}

FString UResultsWidget::FormatTime(float TimeInSeconds)
{
	if (TimeInSeconds <= 0.0f)
	{
		return TEXT("00:00.000");
	}

	const int32 TotalSeconds = FMath::FloorToInt(TimeInSeconds);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	const int32 Milliseconds = FMath::FloorToInt((TimeInSeconds - TotalSeconds) * 1000.0f);

	return FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds);
}

FString UResultsWidget::GetOrdinalSuffix(int32 Position)
{
	// Handle 11th, 12th, 13th as special cases
	const int32 Mod100 = Position % 100;
	if (Mod100 >= 11 && Mod100 <= 13)
	{
		return TEXT("th");
	}

	switch (Position % 10)
	{
	case 1: return TEXT("st");
	case 2: return TEXT("nd");
	case 3: return TEXT("rd");
	default: return TEXT("th");
	}
}
