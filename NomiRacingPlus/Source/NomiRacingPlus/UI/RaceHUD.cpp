// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/RaceHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Widget.h"
#include "NomiRacingPlus.h"

URaceHUD::URaceHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URaceHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize visibility
	if (NOMICommentBox)
	{
		NOMICommentBox->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CountdownBox)
	{
		CountdownBox->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BatteryBox)
	{
		BatteryBox->SetVisibility(ESlateVisibility::Hidden);
	}

	if (DriftIndicator)
	{
		DriftIndicator->SetVisibility(ESlateVisibility::Hidden);
	}

	UE_LOG(LogNomiRacing, Log, TEXT("Race HUD constructed"));
}

void URaceHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update widget values
	if (SpeedText)
	{
		SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(HUDData.Speed))));
	}

	if (PositionText)
	{
		PositionText->SetText(FText::FromString(FormatPosition(HUDData.Position)));
	}

	if (LapText)
	{
		FString LapStr = FString::Printf(TEXT("%d / %d"), HUDData.CurrentLap, HUDData.TotalLaps);
		LapText->SetText(FText::FromString(LapStr));
	}

	if (TimerText)
	{
		TimerText->SetText(FText::FromString(FormatTime(HUDData.RaceTimer)));
	}

	if (BestLapText)
	{
		if (HUDData.BestLapTime > 0.0f)
		{
			BestLapText->SetText(FText::FromString(FormatTime(HUDData.BestLapTime)));
		}
		else
		{
			BestLapText->SetText(FText::FromString(TEXT("--:--.---")));
		}
	}

	if (ThrottleBar)
	{
		ThrottleBar->SetPercent(HUDData.ThrottleInput);
	}

	if (BrakeBar)
	{
		BrakeBar->SetPercent(HUDData.BrakeInput);
	}

	// NOMI comment visibility
	if (NOMICommentBox)
	{
		ESlateVisibility Visibility = HUDData.bNOMICommentVisible ?
			ESlateVisibility::Visible : ESlateVisibility::Hidden;
		NOMICommentBox->SetVisibility(Visibility);
	}

	if (NOMICommentText && HUDData.bNOMICommentVisible)
	{
		NOMICommentText->SetText(FText::FromString(HUDData.NOMICommentText));
	}

	// Battery bar (NIO vehicles only)
	if (BatteryBox)
	{
		ESlateVisibility Visibility = HUDData.bIsNIOVehicle ?
			ESlateVisibility::Visible : ESlateVisibility::Hidden;
		BatteryBox->SetVisibility(Visibility);
	}

	if (BatteryBar && HUDData.bIsNIOVehicle)
	{
		BatteryBar->SetPercent(HUDData.BatteryLevel / 100.0f);
	}

	// Drift indicator
	if (DriftIndicator)
	{
		ESlateVisibility Visibility = HUDData.bIsDrifting ?
			ESlateVisibility::Visible : ESlateVisibility::Hidden;
		DriftIndicator->SetVisibility(Visibility);
	}

	// Countdown
	if (CountdownBox)
	{
		ESlateVisibility Visibility = HUDData.RaceState == ERaceState::Countdown ?
			ESlateVisibility::Visible : ESlateVisibility::Hidden;
		CountdownBox->SetVisibility(Visibility);
	}

	if (CountdownText && HUDData.RaceState == ERaceState::Countdown)
	{
		if (HUDData.CountdownValue > 0.0f)
		{
			CountdownText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(HUDData.CountdownValue))));
		}
		else
		{
			CountdownText->SetText(FText::FromString(TEXT("GO!")));
		}
	}
}

void URaceHUD::UpdateHUDData(const FHUDData& NewData)
{
	HUDData = NewData;
}

void URaceHUD::SetHUDVisible(bool bVisible)
{
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void URaceHUD::ShowCountdown(float Value)
{
	HUDData.CountdownValue = Value;
	HUDData.RaceState = ERaceState::Countdown;
}

void URaceHUD::HideCountdown()
{
	HUDData.RaceState = ERaceState::Racing;
}

FString URaceHUD::FormatTime(float TimeInSeconds) const
{
	int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
	int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeInSeconds, 60.0f));
	int32 Milliseconds = FMath::FloorToInt(FMath::Fmod(TimeInSeconds * 1000.0f, 1000.0f));

	return FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds);
}

FString URaceHUD::FormatPosition(int32 Position) const
{
	if (Position <= 0)
	{
		return TEXT("--");
	}

	FString Suffix;

	if (Position % 100 >= 11 && Position % 100 <= 13)
	{
		Suffix = TEXT("th");
	}
	else
	{
		switch (Position % 10)
		{
		case 1: Suffix = TEXT("st"); break;
		case 2: Suffix = TEXT("nd"); break;
		case 3: Suffix = TEXT("rd"); break;
		default: Suffix = TEXT("th"); break;
		}
	}

	return FString::Printf(TEXT("%d%s"), Position, *Suffix);
}
