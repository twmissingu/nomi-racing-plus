// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AccessibilityManager.generated.h"

/**
 * Font size presets for accessibility
 */
UENUM(BlueprintType)
enum class EFontSizePreset : uint8
{
	Small       UMETA(DisplayName = "Small"),
	Medium      UMETA(DisplayName = "Medium"),
	Large       UMETA(DisplayName = "Large")
};

/**
 * Accessibility settings saved to disk
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAccessibilitySettings
{
	GENERATED_BODY()

	// Enable colorblind-safe color palette (swaps red/green for blue/orange)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bColorblindMode = false;

	// Enable high-contrast mode for improved readability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bHighContrastMode = false;

	// Font size preset (0=Small, 1=Medium, 2=Large)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	int32 FontSizePreset = 1;

	// Enable subtitles for NOMI voice comments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bNOMISubtitlesEnabled = true;

	// Enable keyboard navigation for menus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bKeyboardNavigationEnabled = true;
};

/**
 * Accessibility Manager - handles visual and input accessibility features
 * Provides colorblind-safe palettes, high-contrast mode, font scaling,
 * NOMI subtitles, and keyboard navigation support.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAccessibilityManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UAccessibilityManager();

	virtual void BeginPlay() override;

	// ── Settings Control ─────────────────────────────────────────────────

	// Enable or disable colorblind-safe color mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void ApplyColorblindMode(bool bEnabled);

	// Enable or disable high-contrast mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void ApplyHighContrastMode(bool bEnabled);

	// Set font size preset (0=Small, 1=Medium, 2=Large)
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetFontSizePreset(int32 Preset);

	// Enable or disable NOMI subtitles
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetNOMISubtitles(bool bEnabled);

	// Enable or disable keyboard navigation
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetKeyboardNavigation(bool bEnabled);

	// Apply all settings from a settings struct
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void ApplySettings(const FAccessibilitySettings& NewSettings);

	// ── Color Queries ────────────────────────────────────────────────────

	// Get a color adapted for the current accessibility mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colors")
	FLinearColor GetAdaptedColor(const FLinearColor& OriginalColor) const;

	// Get the success/positive color for current mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colors")
	FLinearColor GetSuccessColor() const;

	// Get the danger/error color for current mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colors")
	FLinearColor GetDangerColor() const;

	// Get the warning color for current mode
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colors")
	FLinearColor GetWarningColor() const;

	// ── Font Size Queries ────────────────────────────────────────────────

	// Get scaled font size from a base size
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Font")
	int32 GetFontSize(int32 BaseSize) const;

	// Get the font scale factor for current preset
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Font")
	float GetFontScale() const;

	// ── Settings Persistence ─────────────────────────────────────────────

	// Save accessibility settings to JSON file
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	bool SaveSettings();

	// Load accessibility settings from JSON file
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	bool LoadSettings();

	// Get current accessibility settings
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	const FAccessibilitySettings& GetSettings() const { return CurrentSettings; }

private:
	// Current accessibility settings
	UPROPERTY()
	FAccessibilitySettings CurrentSettings;

	// Settings file name
	FString SettingsFileName = TEXT("AccessibilitySettings.json");

	// Get save file path
	FString GetSaveFilePath() const;

	// Check if a color is close to red/green (needs colorblind adaptation)
	bool IsRedGreenColor(const FLinearColor& Color) const;

	// Adapt a red/green color to colorblind-safe blue/orange
	FLinearColor AdaptRedGreenColor(const FLinearColor& Color) const;

	// Increase contrast for high-contrast mode
	FLinearColor ApplyHighContrast(const FLinearColor& Color) const;
};
