// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TireTempWidget.generated.h"

// Forward declarations
class UTextBlock;

/**
 * Tire temperature display widget for racing HUD
 * Shows per-wheel tire temperatures with color-coded status indicators
 *
 * Color coding:
 * - Blue: Cold tires (<40°C) — low grip
 * - Green: Optimal temperature (60-90°C) — maximum grip
 * - Red: Overheating (>110°C) — grip loss
 *
 * This widget is hidden by default and should only be visible during racing.
 * Blueprintable to allow designers to customize appearance in UMG.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UTireTempWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTireTempWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/**
	 * Update all tire temperatures and refresh display
	 * @param FL Temperature of front-left tire (Celsius)
	 * @param FR Temperature of front-right tire (Celsius)
	 * @param RL Temperature of rear-left tire (Celsius)
	 * @param RR Temperature of rear-right tire (Celsius)
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Tires")
	void UpdateTemperatures(float FL, float FR, float RL, float RR);

	/** Show the tire temperature widget */
	UFUNCTION(BlueprintCallable, Category = "HUD|Tires")
	void ShowWidget();

	/** Hide the tire temperature widget */
	UFUNCTION(BlueprintCallable, Category = "HUD|Tires")
	void HideWidget();

protected:
	// Front-left temperature text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TireTempFLText;

	// Front-right temperature text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TireTempFRText;

	// Rear-left temperature text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TireTempRLText;

	// Rear-right temperature text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TireTempRRText;

	// Average temperature text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AvgTireTempText;

private:
	/**
	 * Get color for a given temperature value
	 * @param Temperature Temperature in Celsius
	 * @return FSlateColor representing the tire temperature state
	 */
	FSlateColor GetTemperatureColor(float Temperature) const;

	/**
	 * Format temperature value for display
	 * @param Temperature Temperature in Celsius
	 * @return Formatted string with degree symbol
	 */
	FString FormatTemperature(float Temperature) const;

	/**
	 * Update a single text block with temperature value and color
	 * @param TextBlock Widget to update
	 * @param Temperature Temperature value to display
	 */
	void UpdateTireText(UTextBlock* TextBlock, float Temperature) const;

	// Temperature thresholds (matching TirePhysicsModel)
	static constexpr float ColdThreshold = 40.0f;
	static constexpr float OptimalMin = 60.0f;
	static constexpr float OptimalMax = 90.0f;
	static constexpr float OverheatThreshold = 110.0f;
};
