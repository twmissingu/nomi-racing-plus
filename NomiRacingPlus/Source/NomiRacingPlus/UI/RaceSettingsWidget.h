// NomiRacingPlus - Race Settings Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RaceSettingsWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;

/**
 * Race configuration widget - allows player to set AI count, difficulty,
 * number of laps, and weather before starting a race.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API URaceSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Bind widgets ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartRaceButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AIPlusButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AIMinusButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DifficultyEasyBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DifficultyMedBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DifficultyHardBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LapsPlusButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LapsMinusButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AICountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DifficultyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LapsText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeatherText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeatherNextBtn;

	// --- Settings state ---

	UPROPERTY()
	int32 AICount = 5;

	UPROPERTY()
	int32 DifficultyIndex = 1;

	UPROPERTY()
	int32 NumLaps = 3;

	UPROPERTY()
	int32 WeatherIndex = 0;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnAIPlus();

	UFUNCTION()
	void OnAIMinus();

	UFUNCTION()
	void OnDifficultyClickedEasy();

	UFUNCTION()
	void OnDifficultyClickedMed();

	UFUNCTION()
	void OnDifficultyClickedHard();

	void OnDifficultyClicked(int32 InDifficulty);

	UFUNCTION()
	void OnLapsPlus();

	UFUNCTION()
	void OnLapsMinus();

	UFUNCTION()
	void OnWeatherNext();

	UFUNCTION()
	void OnStartRace();

	UFUNCTION()
	void OnBack();

	void UpdateDisplay();
};
