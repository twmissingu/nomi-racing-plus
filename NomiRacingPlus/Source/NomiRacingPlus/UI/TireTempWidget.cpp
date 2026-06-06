// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/TireTempWidget.h"
#include "Components/TextBlock.h"
#include "NomiRacingPlus.h"

UTireTempWidget::UTireTempWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTireTempWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Widget is hidden by default, shown only during racing
	SetVisibility(ESlateVisibility::Collapsed);

	UE_LOG(LogNomiRacing, Log, TEXT("Tire Temp Widget constructed"));
}

void UTireTempWidget::UpdateTemperatures(float FL, float FR, float RL, float RR)
{
	// Update individual tire displays
	UpdateTireText(TireTempFLText, FL);
	UpdateTireText(TireTempFRText, FR);
	UpdateTireText(TireTempRLText, RL);
	UpdateTireText(TireTempRRText, RR);

	// Calculate and display average
	const float AvgTemp = (FL + FR + RL + RR) / 4.0f;

	if (AvgTireTempText)
	{
		const FString AvgStr = FString::Printf(TEXT("Avg: %s"), *FormatTemperature(AvgTemp));
		AvgTireTempText->SetText(FText::FromString(AvgStr));
		AvgTireTempText->SetColorAndOpacity(GetTemperatureColor(AvgTemp));
	}
}

void UTireTempWidget::ShowWidget()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UTireTempWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

FSlateColor UTireTempWidget::GetTemperatureColor(float Temperature) const
{
	// Blue: Cold tires — low grip
	if (Temperature < ColdThreshold)
	{
		return FSlateColor(FLinearColor(0.2f, 0.4f, 1.0f)); // Bright blue
	}

	// Green: Optimal temperature — maximum grip
	if (Temperature >= OptimalMin && Temperature <= OptimalMax)
	{
		return FSlateColor(FLinearColor(0.0f, 0.8f, 0.2f)); // Bright green
	}

	// Red: Overheating — grip loss
	if (Temperature > OverheatThreshold)
	{
		return FSlateColor(FLinearColor(1.0f, 0.15f, 0.15f)); // Bright red
	}

	// Transition zones (40-60 or 90-110): white/neutral
	return FSlateColor(FLinearColor::White);
}

FString UTireTempWidget::FormatTemperature(float Temperature) const
{
	return FString::Printf(TEXT("%.0f°C"), Temperature);
}

void UTireTempWidget::UpdateTireText(UTextBlock* TextBlock, float Temperature) const
{
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(FormatTemperature(Temperature)));
		TextBlock->SetColorAndOpacity(GetTemperatureColor(Temperature));
	}
}
