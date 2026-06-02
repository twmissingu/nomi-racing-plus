// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiGameInstance.h"
#include "NomiRaceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMisc.h"
#include "RHI.h"
#include "NomiRacingPlus.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UNomiGameInstance::UNomiGameInstance()
{
}

void UNomiGameInstance::Init()
{
	Super::Init();

	// Create the race progression component
	RaceProgression = NewObject<URaceProgression>(this, TEXT("RaceProgression"));
	if (RaceProgression)
	{
		// Manually initialize since BeginPlay() won't be called for GameInstance-owned components
		RaceProgression->InitializeSystems();
		RaceProgression->LoadProgression();
		UE_LOG(LogNomiRace, Log, TEXT("RaceProgression component created and loaded"));
	}

	// Load saved settings
	LoadSettings();
	LoadProgress();

	// Apply settings
	ApplyGraphicsSettings();
	ApplyAudioSettings();

	UE_LOG(LogNomiRacing, Log, TEXT("NomiGameInstance initialized"));
}

void UNomiGameInstance::UpdateSettings(const FNomiGameSettings& NewSettings)
{
	Settings = NewSettings;

	// Apply changes
	ApplyGraphicsSettings();
	ApplyAudioSettings();

	UE_LOG(LogNomiRacing, Log, TEXT("Settings updated"));
}

bool UNomiGameInstance::SaveSettings()
{
	FString FilePath = GetSaveFilePath(SettingsFileName);

	// Convert settings to JSON string
	FString JsonString;
	// Simple JSON serialization
	JsonString += TEXT("{\n");
	JsonString += FString::Printf(TEXT("  \"GraphicsQuality\": %d,\n"), Settings.GraphicsQuality);
	JsonString += FString::Printf(TEXT("  \"EnableNanite\": %s,\n"), Settings.bEnableNanite ? TEXT("true") : TEXT("false"));
	JsonString += FString::Printf(TEXT("  \"EnableLumen\": %s,\n"), Settings.bEnableLumen ? TEXT("true") : TEXT("false"));
	JsonString += FString::Printf(TEXT("  \"EnableMotionBlur\": %s,\n"), Settings.bEnableMotionBlur ? TEXT("true") : TEXT("false"));
	JsonString += FString::Printf(TEXT("  \"MasterVolume\": %.2f,\n"), Settings.MasterVolume);
	JsonString += FString::Printf(TEXT("  \"SFXVolume\": %.2f,\n"), Settings.SFXVolume);
	JsonString += FString::Printf(TEXT("  \"MusicVolume\": %.2f,\n"), Settings.MusicVolume);
	JsonString += FString::Printf(TEXT("  \"NOMIFrequency\": %d,\n"), Settings.NOMIFrequency);
	JsonString += FString::Printf(TEXT("  \"SelectedVehicle\": %d,\n"), (int32)Settings.SelectedVehicle);
	JsonString += FString::Printf(TEXT("  \"SelectedTrack\": \"%s\",\n"), *Settings.SelectedTrack);
	JsonString += FString::Printf(TEXT("  \"Difficulty\": %d,\n"), Settings.Difficulty);
	JsonString += FString::Printf(TEXT("  \"NumLaps\": %d,\n"), Settings.NumLaps);
	JsonString += FString::Printf(TEXT("  \"NumAIOpponents\": %d\n"), Settings.NumAIOpponents);
	JsonString += TEXT("}\n");

	// Save to file
	bool bSuccess = FFileHelper::SaveStringToFile(JsonString, *FilePath);
	if (bSuccess)
	{
		UE_LOG(LogNomiRacing, Log, TEXT("Settings saved to: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to save settings"));
	}

	return bSuccess;
}

bool UNomiGameInstance::LoadSettings()
{
	FString FilePath = GetSaveFilePath(SettingsFileName);

	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogNomiRacing, Log, TEXT("No saved settings found, using defaults"));
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to load settings file"));
		return false;
	}

	// Parse JSON (simple parser)
	// Note: In production, use FJsonObjectConverter
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetNumberField(TEXT("GraphicsQuality"), Settings.GraphicsQuality);
		JsonObject->TryGetBoolField(TEXT("EnableNanite"), Settings.bEnableNanite);
		JsonObject->TryGetBoolField(TEXT("EnableLumen"), Settings.bEnableLumen);
		JsonObject->TryGetBoolField(TEXT("EnableMotionBlur"), Settings.bEnableMotionBlur);
		JsonObject->TryGetNumberField(TEXT("MasterVolume"), Settings.MasterVolume);
		JsonObject->TryGetNumberField(TEXT("SFXVolume"), Settings.SFXVolume);
		JsonObject->TryGetNumberField(TEXT("MusicVolume"), Settings.MusicVolume);
		JsonObject->TryGetNumberField(TEXT("NOMIFrequency"), Settings.NOMIFrequency);

		int32 VehicleType = 0;
		JsonObject->TryGetNumberField(TEXT("SelectedVehicle"), VehicleType);
		// Validate enum range before casting
		if (VehicleType >= 0 && VehicleType <= static_cast<int32>(ENIOVehicleType::Custom))
		{
			Settings.SelectedVehicle = static_cast<ENIOVehicleType>(VehicleType);
		}

		JsonObject->TryGetStringField(TEXT("SelectedTrack"), Settings.SelectedTrack);
		JsonObject->TryGetNumberField(TEXT("Difficulty"), Settings.Difficulty);
		JsonObject->TryGetNumberField(TEXT("NumLaps"), Settings.NumLaps);
		JsonObject->TryGetNumberField(TEXT("NumAIOpponents"), Settings.NumAIOpponents);

		UE_LOG(LogNomiRacing, Log, TEXT("Settings loaded successfully"));
		return true;
	}

	return false;
}

void UNomiGameInstance::ResetSettings()
{
	Settings = FNomiGameSettings();
	ApplyGraphicsSettings();
	ApplyAudioSettings();

	UE_LOG(LogNomiRacing, Log, TEXT("Settings reset to defaults"));
}

void UNomiGameInstance::UpdateBestLapTime(const FString& TrackName, float LapTime)
{
	float* ExistingTime = Settings.BestLapTimes.Find(TrackName);
	if (!ExistingTime || LapTime < *ExistingTime)
	{
		Settings.BestLapTimes.Add(TrackName, LapTime);
		SaveProgress();

		UE_LOG(LogNomiRacing, Log, TEXT("New best lap time for %s: %.3f"), *TrackName, LapTime);
	}
}

float UNomiGameInstance::GetBestLapTime(const FString& TrackName) const
{
	const float* Time = Settings.BestLapTimes.Find(TrackName);
	return Time ? *Time : -1.0f;
}

void UNomiGameInstance::AddSeasonPoints(int32 Points)
{
	Settings.SeasonPoints += Points;
	SaveProgress();

	UE_LOG(LogNomiRacing, Log, TEXT("Season points added: %d (Total: %d)"), Points, Settings.SeasonPoints);
}

bool UNomiGameInstance::SaveProgress()
{
	FString FilePath = GetSaveFilePath(ProgressFileName);

	FString JsonString;
	JsonString += TEXT("{\n");
	JsonString += FString::Printf(TEXT("  \"SeasonPoints\": %d,\n"), Settings.SeasonPoints);
	JsonString += TEXT("  \"BestLapTimes\": {\n");

	int32 Index = 0;
	for (const auto& Pair : Settings.BestLapTimes)
	{
		JsonString += FString::Printf(TEXT("    \"%s\": %.3f"), *Pair.Key, Pair.Value);
		if (Index < Settings.BestLapTimes.Num() - 1)
		{
			JsonString += TEXT(",");
		}
		JsonString += TEXT("\n");
		Index++;
	}

	JsonString += TEXT("  }\n");
	JsonString += TEXT("}\n");

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UNomiGameInstance::LoadProgress()
{
	FString FilePath = GetSaveFilePath(ProgressFileName);

	if (!FPaths::FileExists(FilePath))
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetNumberField(TEXT("SeasonPoints"), Settings.SeasonPoints);

		const TSharedPtr<FJsonObject>* LapTimesObj;
		if (JsonObject->TryGetObjectField(TEXT("BestLapTimes"), LapTimesObj))
		{
			for (const auto& Pair : (*LapTimesObj)->Values)
			{
				float Time = 0.0f;
				Pair.Value->TryGetNumber(Time);
				Settings.BestLapTimes.Add(Pair.Key, Time);
			}
		}

		return true;
	}

	return false;
}

bool UNomiGameInstance::IsMacOS() const
{
#if PLATFORM_MAC
	return true;
#else
	return false;
#endif
}

bool UNomiGameInstance::IsNaniteSupported() const
{
	// Nanite requires DirectX 12 or Vulkan (not Metal)
#if PLATFORM_MAC
	return false;
#else
	return true; // Assume Windows supports Nanite
#endif
}

bool UNomiGameInstance::IsLumenSupported() const
{
	// Lumen works on all platforms but with different quality
	return true;
}

FString UNomiGameInstance::GetSaveFilePath(const FString& FileName) const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / FileName;
}

void UNomiGameInstance::ApplyGraphicsSettings()
{
	if (!GEngine)
	{
		return;
	}

	UWorld* World = GetWorld();

	// Apply graphics quality settings via console commands
	switch (Settings.GraphicsQuality)
	{
	case 0: // Low
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 0"));
		break;

	case 1: // Medium
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 1"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 1"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 1"));
		break;

	case 2: // High
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 2"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 2"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 2"));
		break;
	}

	// Nanite
	if (IsNaniteSupported())
	{
		GEngine->Exec(World, Settings.bEnableNanite ?
			TEXT("r.Nanite 1") : TEXT("r.Nanite 0"));
	}

	// Lumen
	GEngine->Exec(World, Settings.bEnableLumen ?
		TEXT("r.DynamicGlobalIlluminationMethod 1") :
		TEXT("r.DynamicGlobalIlluminationMethod 0"));

	// Motion blur
	GEngine->Exec(World, Settings.bEnableMotionBlur ?
		TEXT("r.MotionBlurQuality 1") : TEXT("r.MotionBlurQuality 0"));
}

void UNomiGameInstance::ApplyAudioSettings()
{
	// Apply volume settings
	// Note: Actual implementation would use AudioMixer
}

void UNomiGameInstance::RecordRaceSession(const FRaceSessionResult& SessionResult)
{
	if (RaceProgression)
	{
		RaceProgression->RecordRaceSession(SessionResult);

		UE_LOG(LogNomiRace, Log, TEXT("Race session recorded via GameInstance: %s on %s, Position: %d"),
			*SessionResult.VehicleName, *SessionResult.TrackName, SessionResult.FinalPosition);
	}
}

AChampionshipManager* UNomiGameInstance::GetChampionshipManager() const
{
	// ChampionshipManager is spawned by the GameMode, not the GameInstance
	if (UWorld* World = GetWorld())
	{
		if (ANomiRaceGameMode* GM = Cast<ANomiRaceGameMode>(World->GetAuthGameMode()))
		{
			return GM->GetChampionshipManager();
		}
	}
	return nullptr;
}

bool UNomiGameInstance::StartChampionship(const FString& ChampionshipID)
{
	AChampionshipManager* CM = GetChampionshipManager();
	if (CM)
	{
		return CM->StartChampionshipByID(ChampionshipID);
	}

	UE_LOG(LogNomiRace, Warning, TEXT("ChampionshipManager not available"));
	return false;
}

const FPlayerStatistics& UNomiGameInstance::GetPlayerStatistics() const
{
	static FPlayerStatistics EmptyStats;
	if (RaceProgression)
	{
		return RaceProgression->GetStatistics();
	}
	return EmptyStats;
}

const TMap<EAchievement, FAchievementData>& UNomiGameInstance::GetAchievements() const
{
	static TMap<EAchievement, FAchievementData> EmptyMap;
	if (RaceProgression)
	{
		return RaceProgression->GetAchievements();
	}
	return EmptyMap;
}

const TMap<FString, FUnlockableItem>& UNomiGameInstance::GetUnlockables() const
{
	static TMap<FString, FUnlockableItem> EmptyMap;
	if (RaceProgression)
	{
		return RaceProgression->GetUnlockables();
	}
	return EmptyMap;
}
