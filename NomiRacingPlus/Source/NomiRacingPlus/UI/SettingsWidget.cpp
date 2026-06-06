// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/SettingsWidget.h"
#include "UI/MenuManager.h"
#include "Core/NomiGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

void USettingsWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Load current settings into working copy
	LoadCurrentSettings();

	// --- Audio sliders ---
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(WorkingMasterVolume);
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMasterVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->SetValue(WorkingSFXVolume);
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSFXVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetValue(WorkingMusicVolume);
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMusicVolumeChanged);
	}

	// --- Graphics preset combo ---
	if (QualityPresetCombo)
	{
		QualityPresetCombo->ClearOptions();
		QualityPresetCombo->AddOption(TEXT("Low"));
		QualityPresetCombo->AddOption(TEXT("Medium"));
		QualityPresetCombo->AddOption(TEXT("High"));

		const FString CurrentPreset = WorkingPresetIndex == 0 ? TEXT("Low")
			: WorkingPresetIndex == 2 ? TEXT("High")
			: TEXT("Medium");
		QualityPresetCombo->SetSelectedOption(CurrentPreset);
		QualityPresetCombo->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnQualityPresetChanged);
	}

	// --- Toggle buttons ---
	if (NaniteToggle)
	{
		NaniteToggle->OnClicked.AddDynamic(this, &USettingsWidget::OnNaniteClicked);
	}
	if (LumenToggle)
	{
		LumenToggle->OnClicked.AddDynamic(this, &USettingsWidget::OnLumenClicked);
	}
	if (MotionBlurToggle)
	{
		MotionBlurToggle->OnClicked.AddDynamic(this, &USettingsWidget::OnMotionBlurClicked);
	}

	// --- NOMI frequency combo ---
	if (NOMIFrequencyCombo)
	{
		NOMIFrequencyCombo->ClearOptions();
		NOMIFrequencyCombo->AddOption(TEXT("Off"));
		NOMIFrequencyCombo->AddOption(TEXT("Low"));
		NOMIFrequencyCombo->AddOption(TEXT("Medium"));
		NOMIFrequencyCombo->AddOption(TEXT("High"));

		const FString NOMIFreq = WorkingNOMIFrequency == 0 ? TEXT("Off")
			: WorkingNOMIFrequency == 1 ? TEXT("Low")
			: WorkingNOMIFrequency == 3 ? TEXT("High")
			: TEXT("Medium");
		NOMIFrequencyCombo->SetSelectedOption(NOMIFreq);
		NOMIFrequencyCombo->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnNOMIFrequencyChanged);
	}

	// --- Navigation buttons ---
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
	}

	// Initial text updates
	UpdateVolumeTexts();
	UpdateToggleTexts();
}

void USettingsWidget::LoadCurrentSettings()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UNomiGameInstance* GI = Cast<UNomiGameInstance>(World->GetGameInstance());
	if (!GI) return;

	const FNomiGameSettings& Settings = GI->GetSettings();
	WorkingMasterVolume = Settings.MasterVolume;
	WorkingSFXVolume = Settings.SFXVolume;
	WorkingMusicVolume = Settings.MusicVolume;
	bWorkingNanite = Settings.bEnableNanite;
	bWorkingLumen = Settings.bEnableLumen;
	bWorkingMotionBlur = Settings.bEnableMotionBlur;
	WorkingPresetIndex = static_cast<int32>(Settings.CurrentPreset);
	WorkingNOMIFrequency = Settings.NOMIFrequency;
}

// --- Audio handlers ---

void USettingsWidget::OnMasterVolumeChanged(float Value)
{
	WorkingMasterVolume = Value;
	UpdateVolumeTexts();
}

void USettingsWidget::OnSFXVolumeChanged(float Value)
{
	WorkingSFXVolume = Value;
	UpdateVolumeTexts();
}

void USettingsWidget::OnMusicVolumeChanged(float Value)
{
	WorkingMusicVolume = Value;
	UpdateVolumeTexts();
}

// --- Graphics handlers ---

void USettingsWidget::OnQualityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem == TEXT("Low"))
	{
		WorkingPresetIndex = 0;
		// Low preset: disable Nanite and Lumen
		bWorkingNanite = false;
		bWorkingLumen = false;
	}
	else if (SelectedItem == TEXT("High"))
	{
		WorkingPresetIndex = 2;
		// High preset: enable Nanite and Lumen
		bWorkingNanite = true;
		bWorkingLumen = true;
	}
	else
	{
		WorkingPresetIndex = 1;
		// Medium preset: enable Nanite, disable Lumen (software fallback)
		bWorkingNanite = true;
		bWorkingLumen = false;
	}
	UpdateToggleTexts();
}

void USettingsWidget::OnNaniteClicked()
{
	bWorkingNanite = !bWorkingNanite;
	UpdateToggleTexts();
}

void USettingsWidget::OnLumenClicked()
{
	bWorkingLumen = !bWorkingLumen;
	UpdateToggleTexts();
}

void USettingsWidget::OnMotionBlurClicked()
{
	bWorkingMotionBlur = !bWorkingMotionBlur;
	UpdateToggleTexts();
}

void USettingsWidget::OnNOMIFrequencyChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem == TEXT("Off")) WorkingNOMIFrequency = 0;
	else if (SelectedItem == TEXT("Low")) WorkingNOMIFrequency = 1;
	else if (SelectedItem == TEXT("High")) WorkingNOMIFrequency = 3;
	else WorkingNOMIFrequency = 2; // Medium
}

// --- Navigation ---

void USettingsWidget::OnApplyClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UNomiGameInstance* GI = Cast<UNomiGameInstance>(World->GetGameInstance());
	if (!GI) return;

	FNomiGameSettings NewSettings = GI->GetSettings();
	NewSettings.MasterVolume = WorkingMasterVolume;
	NewSettings.SFXVolume = WorkingSFXVolume;
	NewSettings.MusicVolume = WorkingMusicVolume;
	NewSettings.bEnableNanite = bWorkingNanite;
	NewSettings.bEnableLumen = bWorkingLumen;
	NewSettings.bEnableMotionBlur = bWorkingMotionBlur;
	NewSettings.CurrentPreset = static_cast<ENIOGraphicsPreset>(WorkingPresetIndex);
	NewSettings.NOMIFrequency = WorkingNOMIFrequency;

	GI->UpdateSettings(NewSettings);
	GI->SaveSettings();

	UE_LOG(LogNomiRacing, Log, TEXT("Settings applied and saved"));
}

void USettingsWidget::OnBackClicked()
{
	if (MenuManager)
	{
		MenuManager->ReturnToPrevious();
	}
}

// --- Helpers ---

void USettingsWidget::UpdateVolumeTexts()
{
	if (MasterVolumeText)
	{
		MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), WorkingMasterVolume * 100.0f)));
	}
	if (SFXVolumeText)
	{
		SFXVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), WorkingSFXVolume * 100.0f)));
	}
	if (MusicVolumeText)
	{
		MusicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), WorkingMusicVolume * 100.0f)));
	}
}

void USettingsWidget::UpdateToggleTexts()
{
	if (NaniteText)
	{
		NaniteText->SetText(FText::FromString(bWorkingNanite ? TEXT("ON") : TEXT("OFF")));
	}
	if (LumenText)
	{
		LumenText->SetText(FText::FromString(bWorkingLumen ? TEXT("ON") : TEXT("OFF")));
	}
	if (MotionBlurText)
	{
		MotionBlurText->SetText(FText::FromString(bWorkingMotionBlur ? TEXT("ON") : TEXT("OFF")));
	}
}
