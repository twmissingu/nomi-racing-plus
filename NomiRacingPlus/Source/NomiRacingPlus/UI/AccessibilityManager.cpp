// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/AccessibilityManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "NomiRacingPlus.h"

UAccessibilityManager::UAccessibilityManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAccessibilityManager::BeginPlay()
{
	Super::BeginPlay();

	// Load saved settings on startup
	LoadSettings();

	UE_LOG(LogNomiRacing, Log, TEXT("Accessibility Manager initialized (Colorblind=%s, HighContrast=%s, FontPreset=%d)"),
		CurrentSettings.bColorblindMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.bHighContrastMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.FontSizePreset);
}

// ── Settings Control ─────────────────────────────────────────────────────

void UAccessibilityManager::ApplyColorblindMode(bool bEnabled)
{
	if (CurrentSettings.bColorblindMode == bEnabled)
	{
		return;
	}

	CurrentSettings.bColorblindMode = bEnabled;
	UE_LOG(LogNomiRacing, Log, TEXT("Colorblind mode: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAccessibilityManager::ApplyHighContrastMode(bool bEnabled)
{
	if (CurrentSettings.bHighContrastMode == bEnabled)
	{
		return;
	}

	CurrentSettings.bHighContrastMode = bEnabled;
	UE_LOG(LogNomiRacing, Log, TEXT("High contrast mode: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAccessibilityManager::SetFontSizePreset(int32 Preset)
{
	int32 ClampedPreset = FMath::Clamp(Preset, 0, 2);
	if (CurrentSettings.FontSizePreset == ClampedPreset)
	{
		return;
	}

	CurrentSettings.FontSizePreset = ClampedPreset;
	UE_LOG(LogNomiRacing, Log, TEXT("Font size preset: %d (%.2f scale)"), ClampedPreset, GetFontScale());
}

void UAccessibilityManager::SetNOMISubtitles(bool bEnabled)
{
	if (CurrentSettings.bNOMISubtitlesEnabled == bEnabled)
	{
		return;
	}

	CurrentSettings.bNOMISubtitlesEnabled = bEnabled;
	UE_LOG(LogNomiRacing, Log, TEXT("NOMI subtitles: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAccessibilityManager::SetKeyboardNavigation(bool bEnabled)
{
	if (CurrentSettings.bKeyboardNavigationEnabled == bEnabled)
	{
		return;
	}

	CurrentSettings.bKeyboardNavigationEnabled = bEnabled;
	UE_LOG(LogNomiRacing, Log, TEXT("Keyboard navigation: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAccessibilityManager::ApplySettings(const FAccessibilitySettings& NewSettings)
{
	CurrentSettings = NewSettings;
	CurrentSettings.FontSizePreset = FMath::Clamp(CurrentSettings.FontSizePreset, 0, 2);

	UE_LOG(LogNomiRacing, Log, TEXT("Accessibility settings applied (Colorblind=%s, HighContrast=%s, FontPreset=%d, Subtitles=%s, KeyboardNav=%s)"),
		CurrentSettings.bColorblindMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.bHighContrastMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.FontSizePreset,
		CurrentSettings.bNOMISubtitlesEnabled ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.bKeyboardNavigationEnabled ? TEXT("ON") : TEXT("OFF"));
}

// ── Color Queries ────────────────────────────────────────────────────────

FLinearColor UAccessibilityManager::GetAdaptedColor(const FLinearColor& OriginalColor) const
{
	FLinearColor Result = OriginalColor;

	if (CurrentSettings.bColorblindMode)
	{
		Result = AdaptRedGreenColor(Result);
	}

	if (CurrentSettings.bHighContrastMode)
	{
		Result = ApplyHighContrast(Result);
	}

	return Result;
}

FLinearColor UAccessibilityManager::GetSuccessColor() const
{
	// Default success: green (#7FFF00)
	// Colorblind-safe success: blue (#0077BB)
	FLinearColor BaseColor = CurrentSettings.bColorblindMode
		? FLinearColor(0.0f, 0.467f, 0.733f, 1.0f)   // Blue (#0077BB)
		: FLinearColor(0.5f, 1.0f, 0.0f, 1.0f);      // Green (#7FFF00)

	if (CurrentSettings.bHighContrastMode)
	{
		return ApplyHighContrast(BaseColor);
	}
	return BaseColor;
}

FLinearColor UAccessibilityManager::GetDangerColor() const
{
	// Default danger: red (#FF2244)
	// Colorblind-safe danger: orange (#EE7733)
	FLinearColor BaseColor = CurrentSettings.bColorblindMode
		? FLinearColor(0.933f, 0.467f, 0.2f, 1.0f)    // Orange (#EE7733)
		: FLinearColor(1.0f, 0.133f, 0.267f, 1.0f);   // Red (#FF2244)

	if (CurrentSettings.bHighContrastMode)
	{
		return ApplyHighContrast(BaseColor);
	}
	return BaseColor;
}

FLinearColor UAccessibilityManager::GetWarningColor() const
{
	// Warning stays yellow/orange in both modes (already distinguishable)
	FLinearColor BaseColor = FLinearColor(1.0f, 0.722f, 0.0f, 1.0f);

	if (CurrentSettings.bHighContrastMode)
	{
		return ApplyHighContrast(BaseColor);
	}
	return BaseColor;
}

// ── Font Size Queries ────────────────────────────────────────────────────

int32 UAccessibilityManager::GetFontSize(int32 BaseSize) const
{
	return FMath::RoundToInt(BaseSize * GetFontScale());
}

float UAccessibilityManager::GetFontScale() const
{
	switch (CurrentSettings.FontSizePreset)
	{
	case 0:  return 0.85f;   // Small
	case 1:  return 1.0f;    // Medium (default)
	case 2:  return 1.15f;   // Large
	default: return 1.0f;
	}
}

// ── Settings Persistence ─────────────────────────────────────────────────

FString UAccessibilityManager::GetSaveFilePath() const
{
	return FPaths::ProjectSavedDir() / SettingsFileName;
}

bool UAccessibilityManager::SaveSettings()
{
	TSharedPtr<FJsonObject> DataObj = MakeShareable(new FJsonObject());
	DataObj->SetBoolField(TEXT("ColorblindMode"), CurrentSettings.bColorblindMode);
	DataObj->SetBoolField(TEXT("HighContrastMode"), CurrentSettings.bHighContrastMode);
	DataObj->SetNumberField(TEXT("FontSizePreset"), CurrentSettings.FontSizePreset);
	DataObj->SetBoolField(TEXT("NOMISubtitlesEnabled"), CurrentSettings.bNOMISubtitlesEnabled);
	DataObj->SetBoolField(TEXT("KeyboardNavigationEnabled"), CurrentSettings.bKeyboardNavigationEnabled);

	FString JsonString;
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(DataObj.ToSharedRef(), Writer);
	}

	FString FilePath = GetSaveFilePath();
	if (!FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to save accessibility settings to: %s"), *FilePath);
		return false;
	}

	UE_LOG(LogNomiRacing, Log, TEXT("Accessibility settings saved to: %s"), *FilePath);
	return true;
}

bool UAccessibilityManager::LoadSettings()
{
	FString FilePath = GetSaveFilePath();

	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogNomiRacing, Log, TEXT("No accessibility settings file found, using defaults"));
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to load accessibility settings from: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to parse accessibility settings JSON"));
		return false;
	}

	// Read fields with safe defaults
	bool bColorblind = false;
	bool bHighContrast = false;
	int32 FontPreset = 1;
	bool bSubtitles = true;
	bool bKeyboardNav = true;

	JsonObject->TryGetBoolField(TEXT("ColorblindMode"), bColorblind);
	JsonObject->TryGetBoolField(TEXT("HighContrastMode"), bHighContrast);
	JsonObject->TryGetNumberField(TEXT("FontSizePreset"), FontPreset);
	JsonObject->TryGetBoolField(TEXT("NOMISubtitlesEnabled"), bSubtitles);
	JsonObject->TryGetBoolField(TEXT("KeyboardNavigationEnabled"), bKeyboardNav);

	CurrentSettings.bColorblindMode = bColorblind;
	CurrentSettings.bHighContrastMode = bHighContrast;
	CurrentSettings.FontSizePreset = FMath::Clamp(FontPreset, 0, 2);
	CurrentSettings.bNOMISubtitlesEnabled = bSubtitles;
	CurrentSettings.bKeyboardNavigationEnabled = bKeyboardNav;

	UE_LOG(LogNomiRacing, Log, TEXT("Accessibility settings loaded (Colorblind=%s, HighContrast=%s, FontPreset=%d)"),
		CurrentSettings.bColorblindMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.bHighContrastMode ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.FontSizePreset);

	return true;
}

// ── Internal Color Helpers ───────────────────────────────────────────────

bool UAccessibilityManager::IsRedGreenColor(const FLinearColor& Color) const
{
	// Heuristic: detect colors where red or green is the dominant channel
	// and the other is also significant (typical red-green confusion)
	float R = Color.R;
	float G = Color.G;
	float B = Color.B;

	// Strong red with low green (pure reds)
	if (R > 0.6f && G < 0.4f && B < 0.4f)
	{
		return true;
	}

	// Strong green with low red (pure greens)
	if (G > 0.6f && R < 0.4f && B < 0.4f)
	{
		return true;
	}

	// Red-green mixes (yellows, oranges, browns where red and green are both high)
	if (R > 0.5f && G > 0.5f && B < 0.3f)
	{
		return true;
	}

	return false;
}

FLinearColor UAccessibilityManager::AdaptRedGreenColor(const FLinearColor& Color) const
{
	// Swap red/green for blue/orange for colorblind-safe palette
	// Red (#FF2244) -> Orange (#EE7733)
	// Green (#7FFF00) -> Blue (#0077BB)

	if (!IsRedGreenColor(Color))
	{
		return Color;
	}

	float R = Color.R;
	float G = Color.G;
	float B = Color.B;
	float A = Color.A;

	// Strong red -> Orange (shift red toward yellow, reduce pure red dominance)
	if (R > 0.6f && G < 0.4f && B < 0.4f)
	{
		return FLinearColor(
			FMath::Min(R, 0.933f),   // Cap red at orange level
			FMath::Max(G, 0.467f),   // Boost green for orange
			FMath::Max(B, 0.2f),     // Slight blue for warmth
			A
		);
	}

	// Strong green -> Blue (shift green channel to blue)
	if (G > 0.6f && R < 0.4f && B < 0.4f)
	{
		return FLinearColor(
			FMath::Max(R, 0.0f),     // Minimal red
			FMath::Min(G, 0.467f),   // Reduce green
			FMath::Max(B, 0.733f),   // Boost blue
			A
		);
	}

	// Red-green mix (yellows) -> Shift toward orange-blue distinction
	if (R > 0.5f && G > 0.5f && B < 0.3f)
	{
		return FLinearColor(
			0.933f,                   // Orange
			0.467f,
			0.2f,
			A
		);
	}

	return Color;
}

FLinearColor UAccessibilityManager::ApplyHighContrast(const FLinearColor& Color) const
{
	// Increase contrast by pushing luminance away from mid-gray
	// Light colors get lighter, dark colors get darker
	float Luminance = 0.2126f * Color.R + 0.7152f * Color.G + 0.0722f * Color.B;

	const float ContrastBoost = 0.25f;

	if (Luminance > 0.5f)
	{
		// Light color: push toward white
		float Factor = 1.0f + ContrastBoost;
		return FLinearColor(
			FMath::Min(Color.R * Factor, 1.0f),
			FMath::Min(Color.G * Factor, 1.0f),
			FMath::Min(Color.B * Factor, 1.0f),
			Color.A
		);
	}
	else
	{
		// Dark color: push toward black
		float Factor = 1.0f - ContrastBoost;
		return FLinearColor(
			Color.R * Factor,
			Color.G * Factor,
			Color.B * Factor,
			Color.A
		);
	}
}
