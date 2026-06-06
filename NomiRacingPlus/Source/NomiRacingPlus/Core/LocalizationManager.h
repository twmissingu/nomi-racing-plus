// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LocalizationManager.generated.h"

/**
 * Supported locales for the game
 */
UENUM(BlueprintType)
enum class ELocale : uint8
{
	EN UMETA(DisplayName = "English"),
	ZH UMETA(DisplayName = "Chinese")
};

/**
 * Localization Manager Component
 * Handles runtime locale switching and FText retrieval for UI strings.
 * Loads localization tables from Content/Localization/ JSON files.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API ULocalizationManager : public UActorComponent
{
	GENERATED_BODY()

public:
	ULocalizationManager();

	virtual void BeginPlay() override;

	/**
	 * Get the localized FText for a given key in the current locale.
	 * Returns the English fallback if the key is not found in the current locale.
	 *
	 * @param Key  FName identifier for the localization string
	 * @return Localized FText, or the key name as fallback
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	FText GetText(FName Key) const;

	/**
	 * Switch to a new locale at runtime and reload localization tables.
	 *
	 * @param NewLocale  Target locale to switch to
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLocale(ELocale NewLocale);

	/**
	 * Load all localization tables from Content/Localization/.
	 * Reads JSON files named "EN.json" and "ZH.json".
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void LoadLocalizationTables();

	/**
	 * Get the current locale setting.
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	ELocale GetCurrentLocale() const { return CurrentLocale; }

	/**
	 * Get list of available locale keys for debugging or UI population.
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	TArray<FName> GetAvailableKeys() const;

	// Current locale — exposed to Blueprint for settings UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	ELocale CurrentLocale = ELocale::EN;

protected:
	// Locale-specific text maps loaded from JSON
	UPROPERTY()
	TMap<FName, FText> LocaleTextEN;

	UPROPERTY()
	TMap<FName, FText> LocaleTextZH;

	// Fallback text for missing keys
	UPROPERTY(EditDefaultsOnly, Category = "Localization")
	FString FallbackLocale = TEXT("EN");

private:
	// Parse a single locale JSON file into a TMap<FName, FText>
	bool ParseLocaleFile(const FString& FilePath, TMap<FName, FText>& OutTextMap);

	// Get the active text map for the current locale
	const TMap<FName, FText>& GetActiveTextMap() const;

	// Build the expected file path for a locale JSON
	FString GetLocaleFilePath(const FString& LocaleCode) const;
};
