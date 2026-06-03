// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/LoadingScreenWidget.h"
#include "UI/MenuManager.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY_STATIC(LogNomiLoading, Log, All);

const TArray<FString> ULoadingScreenWidget::LoadingTips = {
	TEXT("Use regenerative braking to recover energy on long straights."),
	TEXT("Drafting behind opponents reduces air resistance and saves battery."),
	TEXT("NIO vehicles deliver instant torque -- feather the throttle out of corners."),
	TEXT("Monitor your battery level; running out of charge means a flat finish."),
	TEXT("Smooth steering inputs preserve tire grip and battery life."),
	TEXT("The Apex camera gives the best view for tight cornering sections."),
	TEXT("Use the rearview camera to watch for overtakes from behind."),
	TEXT("Wet weather reduces tire grip significantly -- brake earlier in the rain."),
	TEXT("Each NIO vehicle has unique handling characteristics; try them all."),
	TEXT("Short-shifting in EV mode helps manage power delivery on low-grip surfaces."),
	TEXT("The NIO EP9 holds the Nurburgring lap record for electric vehicles."),
	TEXT("NOMI, the in-car AI companion, will comment on your driving during the race."),
	TEXT("The NIO ET7 can accelerate from 0-100 km/h in just 3.9 seconds."),
	TEXT("Battery temperature affects performance -- cool batteries deliver peak power."),
	TEXT("Hit the apex of each corner to maintain the fastest racing line.")
};

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &ULoadingScreenWidget::OnCancel);
	}

	// Show an initial tip
	ShowRandomTip();

	UE_LOG(LogNomiLoading, Log, TEXT("LoadingScreenWidget constructed"));
}

void ULoadingScreenWidget::SetProgress(float Percent)
{
	const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);

	if (LoadingBar)
	{
		LoadingBar->SetPercent(ClampedPercent);
	}

	if (ProgressText)
	{
		const int32 DisplayPercent = FMath::RoundToInt(ClampedPercent * 100.0f);
		ProgressText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), DisplayPercent)));
	}
}

void ULoadingScreenWidget::SetTrackName(const FString& Name)
{
	if (TrackNameText)
	{
		TrackNameText->SetText(FText::FromString(Name));
	}
}

void ULoadingScreenWidget::ShowRandomTip()
{
	if (TipText && LoadingTips.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, LoadingTips.Num() - 1);
		TipText->SetText(FText::FromString(LoadingTips[Index]));
	}
}

void ULoadingScreenWidget::SetMenuManager(UMenuManager* InMenuManager)
{
	MenuManager = InMenuManager;
}

void ULoadingScreenWidget::OnCancel()
{
	if (MenuManager)
	{
		MenuManager->ReturnToPrevious();
		UE_LOG(LogNomiLoading, Log, TEXT("Loading cancelled, returning to previous screen"));
	}
	else
	{
		UE_LOG(LogNomiLoading, Warning, TEXT("OnCancel: MenuManager is null"));
	}
}
