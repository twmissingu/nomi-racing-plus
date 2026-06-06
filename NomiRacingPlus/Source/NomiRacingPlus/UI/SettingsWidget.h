// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MenuManager.h"
#include "SettingsWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class UComboBoxString;
class UMenuManager;
class UNomiGameInstance;

/**
 * Settings menu widget for audio, graphics, and gameplay configuration.
 * Reads current settings from NomiGameInstance on open, applies on save.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Audio Controls ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MasterVolumeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SFXVolumeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MusicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MusicVolumeText;

	// --- Graphics Controls ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> QualityPresetCombo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NaniteToggle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NaniteText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LumenToggle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LumenText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MotionBlurToggle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MotionBlurText;

	// --- Gameplay Controls ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> NOMIFrequencyCombo;

	// --- Navigation ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

protected:
	virtual void NativeConstruct() override;

private:
	// Current working copy of settings
	float WorkingMasterVolume = 1.0f;
	float WorkingSFXVolume = 0.8f;
	float WorkingMusicVolume = 0.6f;
	bool bWorkingNanite = true;
	bool bWorkingLumen = true;
	bool bWorkingMotionBlur = true;
	int32 WorkingPresetIndex = 1; // Medium
	int32 WorkingNOMIFrequency = 2;

	// Previous menu state to return to
	EMenuState ReturnState = EMenuState::MainMenu;

	// Button handlers
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnMusicVolumeChanged(float Value);

	UFUNCTION()
	void OnQualityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnNaniteClicked();

	UFUNCTION()
	void OnLumenClicked();

	UFUNCTION()
	void OnMotionBlurClicked();

	UFUNCTION()
	void OnNOMIFrequencyChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnApplyClicked();

	UFUNCTION()
	void OnBackClicked();

	// Helpers
	void LoadCurrentSettings();
	void UpdateToggleTexts();
	void UpdateVolumeTexts();
};
