// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "LocalizationManager.h"
#include "NomiRacingPlus.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

ULocalizationManager::ULocalizationManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULocalizationManager::BeginPlay()
{
	Super::BeginPlay();

	// Load localization tables on startup
	LoadLocalizationTables();

	UE_LOG(LogNomiRacing, Log, TEXT("LocalizationManager initialized with locale: %s"),
		CurrentLocale == ELocale::EN ? TEXT("EN") : TEXT("ZH"));
}

FText ULocalizationManager::GetText(FName Key) const
{
	const TMap<FName, FText>& ActiveMap = GetActiveTextMap();

	if (const FText* Found = ActiveMap.Find(Key))
	{
		return *Found;
	}

	// Fallback to English if key not found in current locale
	if (CurrentLocale != ELocale::EN)
	{
		if (const FText* FoundEN = LocaleTextEN.Find(Key))
		{
			return *FoundEN;
		}
	}

	// Last resort: return the key name as text
	return FText::FromName(Key);
}

void ULocalizationManager::SetLocale(ELocale NewLocale)
{
	if (CurrentLocale != NewLocale)
	{
		CurrentLocale = NewLocale;
		UE_LOG(LogNomiRacing, Log, TEXT("Locale switched to: %s"),
			NewLocale == ELocale::EN ? TEXT("EN") : TEXT("ZH"));
	}
}

void ULocalizationManager::LoadLocalizationTables()
{
	// Clear existing data
	LocaleTextEN.Empty();
	LocaleTextZH.Empty();

	// Load English locale
	const FString ENPath = GetLocaleFilePath(TEXT("EN"));
	if (ParseLocaleFile(ENPath, LocaleTextEN))
	{
		UE_LOG(LogNomiRacing, Log, TEXT("Loaded EN locale: %d keys"), LocaleTextEN.Num());
	}
	else
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("Failed to load EN locale from: %s"), *ENPath);
	}

	// Load Chinese locale
	const FString ZHPath = GetLocaleFilePath(TEXT("ZH"));
	if (ParseLocaleFile(ZHPath, LocaleTextZH))
	{
		UE_LOG(LogNomiRacing, Log, TEXT("Loaded ZH locale: %d keys"), LocaleTextZH.Num());
	}
	else
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("Failed to load ZH locale from: %s"), *ZHPath);
	}
}

TArray<FName> ULocalizationManager::GetAvailableKeys() const
{
	TArray<FName> AllKeys;

	// Collect keys from the active locale
	const TMap<FName, FText>& ActiveMap = GetActiveTextMap();
	ActiveMap.GetKeys(AllKeys);

	return AllKeys;
}

bool ULocalizationManager::ParseLocaleFile(const FString& FilePath, TMap<FName, FText>& OutTextMap)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("Could not read locale file: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("Invalid JSON in locale file: %s"), *FilePath);
		return false;
	}

	// Parse flat key-value pairs: { "MainMenuTitle": "NIO Racing Plus", ... }
	for (const auto& Pair : JsonObject->Values)
	{
		FString TextValue;
		if (Pair.Value->TryGetString(TextValue))
		{
			FName Key(*Pair.Key);
			FText LocalizedText = FText::FromString(TextValue);
			OutTextMap.Add(Key, LocalizedText);
		}
	}

	return OutTextMap.Num() > 0;
}

const TMap<FName, FText>& ULocalizationManager::GetActiveTextMap() const
{
	if (CurrentLocale == ELocale::ZH)
	{
		return LocaleTextZH;
	}
	return LocaleTextEN;
}

FString ULocalizationManager::GetLocaleFilePath(const FString& LocaleCode) const
{
	return FPaths::ProjectContentDir() / TEXT("Localization") / (LocaleCode + TEXT(".json"));
}
