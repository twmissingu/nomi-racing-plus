// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiGameInstance.h"
#include "NomiRaceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"
#include "Core/JsonSerializationHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

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

	// Load saved settings — detect first launch for tutorial auto-start
	bool bLoadedSettings = LoadSettings();
	LoadProgress();

	// On first launch (no save file), flag tutorial for auto-start
	if (!bLoadedSettings)
	{
		bShouldAutoStartTutorial = true;
		UE_LOG(LogNomiRacing, Log, TEXT("First launch detected — tutorial will auto-start"));
	}

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
	// Build data as a JSON object
	TSharedPtr<FJsonObject> DataObj = MakeShareable(new FJsonObject());
	DataObj->SetNumberField(TEXT("GraphicsQuality"), Settings.GraphicsQuality);
	DataObj->SetBoolField(TEXT("EnableNanite"), Settings.bEnableNanite);
	DataObj->SetBoolField(TEXT("EnableLumen"), Settings.bEnableLumen);
	DataObj->SetBoolField(TEXT("EnableMotionBlur"), Settings.bEnableMotionBlur);
	DataObj->SetNumberField(TEXT("GraphicsPreset"), static_cast<int32>(Settings.CurrentPreset));
	DataObj->SetNumberField(TEXT("MasterVolume"), Settings.MasterVolume);
	DataObj->SetNumberField(TEXT("SFXVolume"), Settings.SFXVolume);
	DataObj->SetNumberField(TEXT("MusicVolume"), Settings.MusicVolume);
	DataObj->SetNumberField(TEXT("NOMIFrequency"), Settings.NOMIFrequency);
	DataObj->SetNumberField(TEXT("SelectedVehicle"), static_cast<int32>(Settings.SelectedVehicle));
	DataObj->SetStringField(TEXT("SelectedTrack"), Settings.SelectedTrack);
	DataObj->SetNumberField(TEXT("Difficulty"), Settings.Difficulty);
	DataObj->SetNumberField(TEXT("NumLaps"), Settings.NumLaps);
	DataObj->SetNumberField(TEXT("NumAIOpponents"), Settings.NumAIOpponents);
	DataObj->SetStringField(TEXT("GameMode"), Settings.GameMode);
	DataObj->SetBoolField(TEXT("TutorialCompleted"), Settings.bTutorialCompleted);

	// Compute checksum using sorted-key serialization for determinism
	FString DataString = JsonSerializationHelpers::SerializeSorted(DataObj);
	uint32 Checksum = JsonSerializationHelpers::CalculateCRC32(DataString);

	// Build wrapped JSON with version, checksum, and data object
	TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
	RootObj->SetStringField(TEXT("version"), SaveVersion);
	RootObj->SetNumberField(TEXT("checksum"), static_cast<int64>(Checksum));
	RootObj->SetObjectField(TEXT("data"), DataObj);

	FString JsonString;
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);
	}

	// Rotate existing save into backups before overwriting
	RotateBackups(SettingsFileName);

	// Atomic write to prevent corruption on crash
	bool bSuccess = AtomicWriteSave(SettingsFileName, JsonString);
	if (bSuccess)
	{
		UE_LOG(LogNomiRacing, Log, TEXT("Settings saved to: %s"), *GetSaveFilePath(SettingsFileName));
	}
	else
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Save"), TEXT("Failed to save settings"));
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

	// Validate integrity (version + checksum)
	FString Error;
	if (!ValidateSaveIntegrity(SettingsFileName, Error))
	{
		NomiError::Log(ENomiErrorSeverity::Warning, TEXT("Save"), FString::Printf(TEXT("Settings integrity check failed: %s"), *Error));

		// Store corruption state for NomiPlayerController to check during BeginPlay
		bHasCorruption = true;
		CorruptedFileName = SettingsFileName;
		bCanRestoreBackup = false;
		{
			FString BackupPath = GetSaveFilePath(SettingsFileName + TEXT(".backup-1"));
			bCanRestoreBackup = FPaths::FileExists(BackupPath);
		}

		// Attempt recovery from backups silently
		if (RecoverFromBackup(SettingsFileName))
		{
			UE_LOG(LogNomiRacing, Log, TEXT("Settings recovered from backup"));
			OnSaveRecoveryCompleted.Broadcast(SettingsFileName, true);
		}
		else
		{
			UE_LOG(LogNomiRacing, Warning, TEXT("No valid backup found, using defaults"));
			OnSaveRecoveryCompleted.Broadcast(SettingsFileName, false);
			return false;
		}
	}

	// Load the (now validated or recovered) file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Save"), TEXT("Failed to load settings file"));
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Extract the inner data object (new wrapped format)
		const TSharedPtr<FJsonObject>* DataObj;
		if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
		{
			(*DataObj)->TryGetNumberField(TEXT("GraphicsQuality"), Settings.GraphicsQuality);
			(*DataObj)->TryGetBoolField(TEXT("EnableNanite"), Settings.bEnableNanite);
			(*DataObj)->TryGetBoolField(TEXT("EnableLumen"), Settings.bEnableLumen);
			(*DataObj)->TryGetBoolField(TEXT("EnableMotionBlur"), Settings.bEnableMotionBlur);
			{
				int32 PresetValue = 1; // Default to Medium
				(*DataObj)->TryGetNumberField(TEXT("GraphicsPreset"), PresetValue);
				if (PresetValue >= 0 && PresetValue <= static_cast<int32>(ENIOGraphicsPreset::High))
				{
					Settings.CurrentPreset = static_cast<ENIOGraphicsPreset>(PresetValue);
				}
			}
			(*DataObj)->TryGetNumberField(TEXT("MasterVolume"), Settings.MasterVolume);
			(*DataObj)->TryGetNumberField(TEXT("SFXVolume"), Settings.SFXVolume);
			(*DataObj)->TryGetNumberField(TEXT("MusicVolume"), Settings.MusicVolume);
			(*DataObj)->TryGetNumberField(TEXT("NOMIFrequency"), Settings.NOMIFrequency);

			int32 VehicleType = 0;
			(*DataObj)->TryGetNumberField(TEXT("SelectedVehicle"), VehicleType);
			// Validate enum range before casting
			if (VehicleType >= 0 && VehicleType <= static_cast<int32>(ENIOVehicleType::Custom))
			{
				Settings.SelectedVehicle = static_cast<ENIOVehicleType>(VehicleType);
			}

			(*DataObj)->TryGetStringField(TEXT("SelectedTrack"), Settings.SelectedTrack);
			(*DataObj)->TryGetNumberField(TEXT("Difficulty"), Settings.Difficulty);
			(*DataObj)->TryGetNumberField(TEXT("NumLaps"), Settings.NumLaps);
			(*DataObj)->TryGetNumberField(TEXT("NumAIOpponents"), Settings.NumAIOpponents);
			(*DataObj)->TryGetStringField(TEXT("GameMode"), Settings.GameMode);
			(*DataObj)->TryGetBoolField(TEXT("TutorialCompleted"), Settings.bTutorialCompleted);
		}
		else
		{
			// Legacy format fallback: read fields at top level
			JsonObject->TryGetNumberField(TEXT("GraphicsQuality"), Settings.GraphicsQuality);
			JsonObject->TryGetBoolField(TEXT("EnableNanite"), Settings.bEnableNanite);
			JsonObject->TryGetBoolField(TEXT("EnableLumen"), Settings.bEnableLumen);
			JsonObject->TryGetBoolField(TEXT("EnableMotionBlur"), Settings.bEnableMotionBlur);
			{
				int32 PresetValue = 1;
				JsonObject->TryGetNumberField(TEXT("GraphicsPreset"), PresetValue);
				if (PresetValue >= 0 && PresetValue <= static_cast<int32>(ENIOGraphicsPreset::High))
				{
					Settings.CurrentPreset = static_cast<ENIOGraphicsPreset>(PresetValue);
				}
			}
			JsonObject->TryGetNumberField(TEXT("MasterVolume"), Settings.MasterVolume);
			JsonObject->TryGetNumberField(TEXT("SFXVolume"), Settings.SFXVolume);
			JsonObject->TryGetNumberField(TEXT("MusicVolume"), Settings.MusicVolume);
			JsonObject->TryGetNumberField(TEXT("NOMIFrequency"), Settings.NOMIFrequency);

			int32 VehicleType = 0;
			JsonObject->TryGetNumberField(TEXT("SelectedVehicle"), VehicleType);
			if (VehicleType >= 0 && VehicleType <= static_cast<int32>(ENIOVehicleType::Custom))
			{
				Settings.SelectedVehicle = static_cast<ENIOVehicleType>(VehicleType);
			}

			JsonObject->TryGetStringField(TEXT("SelectedTrack"), Settings.SelectedTrack);
			JsonObject->TryGetNumberField(TEXT("Difficulty"), Settings.Difficulty);
			JsonObject->TryGetNumberField(TEXT("NumLaps"), Settings.NumLaps);
			JsonObject->TryGetNumberField(TEXT("NumAIOpponents"), Settings.NumAIOpponents);
			JsonObject->TryGetStringField(TEXT("GameMode"), Settings.GameMode);
			JsonObject->TryGetBoolField(TEXT("TutorialCompleted"), Settings.bTutorialCompleted);
		}

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
	// Build data as a JSON object
	TSharedPtr<FJsonObject> DataObj = MakeShareable(new FJsonObject());
	DataObj->SetNumberField(TEXT("SeasonPoints"), Settings.SeasonPoints);

	TSharedPtr<FJsonObject> LapTimesObj = MakeShareable(new FJsonObject());
	for (const auto& Pair : Settings.BestLapTimes)
	{
		LapTimesObj->SetNumberField(Pair.Key, Pair.Value);
	}
	DataObj->SetObjectField(TEXT("BestLapTimes"), LapTimesObj);

	// Compute checksum using sorted-key serialization for determinism
	FString DataString = JsonSerializationHelpers::SerializeSorted(DataObj);
	uint32 Checksum = JsonSerializationHelpers::CalculateCRC32(DataString);

	// Build wrapped JSON
	TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
	RootObj->SetStringField(TEXT("version"), SaveVersion);
	RootObj->SetNumberField(TEXT("checksum"), static_cast<int64>(Checksum));
	RootObj->SetObjectField(TEXT("data"), DataObj);

	FString JsonString;
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);
	}

	// Rotate existing save into backups before overwriting
	RotateBackups(ProgressFileName);

	// Atomic write
	return AtomicWriteSave(ProgressFileName, JsonString);
}

bool UNomiGameInstance::LoadProgress()
{
	FString FilePath = GetSaveFilePath(ProgressFileName);

	if (!FPaths::FileExists(FilePath))
	{
		return false;
	}

	// Validate integrity
	FString Error;
	if (!ValidateSaveIntegrity(ProgressFileName, Error))
	{
		NomiError::Log(ENomiErrorSeverity::Warning, TEXT("Save"), FString::Printf(TEXT("Progress integrity check failed: %s"), *Error));

		if (RecoverFromBackup(ProgressFileName))
		{
			UE_LOG(LogNomiRacing, Log, TEXT("Progress recovered from backup"));
		}
		else
		{
			UE_LOG(LogNomiRacing, Warning, TEXT("No valid backup found for progress"));
			return false;
		}
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
		// Try wrapped format first
		const TSharedPtr<FJsonObject>* DataObj;
		if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
		{
			(*DataObj)->TryGetNumberField(TEXT("SeasonPoints"), Settings.SeasonPoints);

			const TSharedPtr<FJsonObject>* LapTimesObj;
			if ((*DataObj)->TryGetObjectField(TEXT("BestLapTimes"), LapTimesObj))
			{
				for (const auto& Pair : (*LapTimesObj)->Values)
				{
					float Time = 0.0f;
					Pair.Value->TryGetNumber(Time);
					Settings.BestLapTimes.Add(Pair.Key, Time);
				}
			}
		}
		else
		{
			// Legacy format fallback
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

uint32 UNomiGameInstance::CalculateChecksum(const FString& Content)
{
	return JsonSerializationHelpers::CalculateCRC32(Content);
}

bool UNomiGameInstance::AtomicWriteSave(const FString& FileName, const FString& JsonString)
{
	FString FilePath = GetSaveFilePath(FileName);
	FString TmpPath = FilePath + TEXT(".tmp");

	// Ensure save directory exists
	FString Directory = FPaths::GetPath(FilePath);
	IFileManager::Get().MakeDirectory(*Directory, true);

	// Write to temporary file first
	if (!FFileHelper::SaveStringToFile(JsonString, *TmpPath))
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Save"), FString::Printf(TEXT("Failed to write temp file: %s"), *TmpPath));
		return false;
	}

	// Atomic rename: replace the target file with the temp file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.MoveFile(*FilePath, *TmpPath))
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Save"), FString::Printf(TEXT("Failed to rename temp file to: %s"), *FilePath));
		// Clean up temp file on failure
		PlatformFile.DeleteFile(*TmpPath);
		return false;
	}

	return true;
}

bool UNomiGameInstance::RotateBackups(const FString& FileName)
{
	FString FilePath = GetSaveFilePath(FileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Delete the oldest backup if it exists
	FString OldestBackup = FString::Printf(TEXT("%s.backup-%d"), *FilePath, MaxBackups);
	if (PlatformFile.FileExists(*OldestBackup))
	{
		PlatformFile.DeleteFile(*OldestBackup);
	}

	// Shift backups: backup-(N-1) -> backup-N, ..., backup-1 -> backup-2
	for (int32 i = MaxBackups - 1; i >= 1; --i)
	{
		FString OlderBackup = FString::Printf(TEXT("%s.backup-%d"), *FilePath, i);
		FString NewerBackup = FString::Printf(TEXT("%s.backup-%d"), *FilePath, i + 1);
		if (PlatformFile.FileExists(*OlderBackup))
		{
			PlatformFile.MoveFile(*NewerBackup, *OlderBackup);
		}
	}

	// Shift current save to backup-1
	if (PlatformFile.FileExists(*FilePath))
	{
		FString FirstBackup = FString::Printf(TEXT("%s.backup-1"), *FilePath);
		PlatformFile.MoveFile(*FirstBackup, *FilePath);
	}

	return true;
}

bool UNomiGameInstance::ValidateSaveIntegrity(const FString& FileName, FString& OutError)
{
	FString FilePath = GetSaveFilePath(FileName);

	if (!FPaths::FileExists(FilePath))
	{
		OutError = TEXT("Save file does not exist");
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		OutError = TEXT("Failed to read save file");
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OutError = TEXT("Invalid JSON format");
		return false;
	}

	// Check version field
	FString Version;
	if (!JsonObject->TryGetStringField(TEXT("version"), Version))
	{
		OutError = TEXT("Missing version field");
		return false;
	}

	// Check checksum field
	int64 StoredChecksum = 0;
	if (!JsonObject->TryGetNumberField(TEXT("checksum"), StoredChecksum))
	{
		OutError = TEXT("Missing checksum field");
		return false;
	}

	// Extract data object and re-serialize with sorted keys for deterministic checksum
	const TSharedPtr<FJsonObject>* DataObj;
	if (!JsonObject->TryGetObjectField(TEXT("data"), DataObj))
	{
		OutError = TEXT("Missing data field");
		return false;
	}

	FString DataString = JsonSerializationHelpers::SerializeSorted(*DataObj);
	uint32 ComputedChecksum = JsonSerializationHelpers::CalculateCRC32(DataString);
	if (ComputedChecksum != static_cast<uint32>(StoredChecksum))
	{
		OutError = FString::Printf(TEXT("Checksum mismatch: stored=%llu computed=%u"), StoredChecksum, ComputedChecksum);
		return false;
	}

	return true;
}

bool UNomiGameInstance::RecoverFromBackup(const FString& FileName)
{
	FString FilePath = GetSaveFilePath(FileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Try backup-1 first, then backup-2, etc.
	for (int32 i = 1; i <= MaxBackups; ++i)
	{
		FString BackupPath = FString::Printf(TEXT("%s.backup-%d"), *FilePath, i);
		if (!PlatformFile.FileExists(*BackupPath))
		{
			continue;
		}

		// Validate backup by loading its content directly (avoids double path construction)
		FString BackupJson;
		if (!FFileHelper::LoadFileToString(BackupJson, *BackupPath))
		{
			continue;
		}

		TSharedPtr<FJsonObject> BackupObj;
		TSharedRef<TJsonReader<>> BackupReader = TJsonReaderFactory<>::Create(BackupJson);
		if (!FJsonSerializer::Deserialize(BackupReader, BackupObj) || !BackupObj.IsValid())
		{
			UE_LOG(LogNomiRacing, Warning, TEXT("Backup-%d has invalid JSON"), i);
			continue;
		}

		// Check version and checksum in the backup
		FString Version;
		int64 StoredChecksum = 0;
		const TSharedPtr<FJsonObject>* DataObj = nullptr;
		bool bValid = BackupObj->TryGetStringField(TEXT("version"), Version)
			&& BackupObj->TryGetNumberField(TEXT("checksum"), StoredChecksum)
			&& BackupObj->TryGetObjectField(TEXT("data"), DataObj);

		if (bValid && DataObj)
		{
			FString DataString = JsonSerializationHelpers::SerializeSorted(*DataObj);
			uint32 Computed = JsonSerializationHelpers::CalculateCRC32(DataString);
			if (Computed == static_cast<uint32>(StoredChecksum))
			{
				// Valid backup — copy it over the corrupted file
				if (PlatformFile.CopyFile(*FilePath, *BackupPath))
				{
					UE_LOG(LogNomiRacing, Log, TEXT("Recovered save from backup-%d: %s"), i, *FileName);
					return true;
				}
			}
			else
			{
				UE_LOG(LogNomiRacing, Warning, TEXT("Backup-%d checksum mismatch"), i);
			}
		}
		else
		{
			UE_LOG(LogNomiRacing, Warning, TEXT("Backup-%d missing required fields"), i);
		}
	}

	UE_LOG(LogNomiRacing, Warning, TEXT("No valid backups found for %s, caller should reset to defaults"), *FileName);
	return false;
}

void UNomiGameInstance::ApplyGraphicsPreset(ENIOGraphicsPreset Preset)
{
	if (!GEngine)
	{
		return;
	}

	UWorld* World = GetWorld();
	Settings.CurrentPreset = Preset;

	switch (Preset)
	{
	case ENIOGraphicsPreset::Low:
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 0"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 0"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 0"));
		GEngine->Exec(World, TEXT("sg.EffectsQuality 0"));
		GEngine->Exec(World, TEXT("sg.FoliageQuality 0"));
		GEngine->Exec(World, TEXT("r.Nanite 0"));
		GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect 0"));
		GEngine->Exec(World, TEXT("r.Lumen.Reflections 0"));
		break;

	case ENIOGraphicsPreset::Medium:
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 1"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 1"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 1"));
		GEngine->Exec(World, TEXT("sg.EffectsQuality 1"));
		GEngine->Exec(World, TEXT("sg.FoliageQuality 1"));
		if (IsNaniteSupported())
		{
			GEngine->Exec(World, TEXT("r.Nanite 1"));
		}
		else
		{
			GEngine->Exec(World, TEXT("r.Nanite 0"));
		}
		GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect 1"));
		GEngine->Exec(World, TEXT("r.Lumen.Reflections 1"));
		break;

	case ENIOGraphicsPreset::High:
		GEngine->Exec(World, TEXT("sg.PostProcessQuality 2"));
		GEngine->Exec(World, TEXT("sg.ShadowQuality 2"));
		GEngine->Exec(World, TEXT("sg.TextureQuality 2"));
		GEngine->Exec(World, TEXT("sg.EffectsQuality 2"));
		GEngine->Exec(World, TEXT("sg.FoliageQuality 2"));
		if (IsNaniteSupported())
		{
			GEngine->Exec(World, TEXT("r.Nanite 1"));
		}
		else
		{
			GEngine->Exec(World, TEXT("r.Nanite 0"));
		}
		GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect 1"));
		GEngine->Exec(World, TEXT("r.Lumen.Reflections 1"));
		break;
	}

#if PLATFORM_MAC
	// macOS fallback: disable Nanite and Lumen regardless of preset
	GEngine->Exec(World, TEXT("r.Nanite 0"));
	GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect 0"));
	GEngine->Exec(World, TEXT("r.Lumen.Reflections 0"));
#endif

	// Sync Settings.GraphicsQuality to match preset for legacy compatibility
	Settings.GraphicsQuality = static_cast<int32>(Preset);

	UE_LOG(LogNomiRacing, Log, TEXT("Graphics preset applied: %d"), static_cast<int32>(Preset));
}

void UNomiGameInstance::ApplyGraphicsSettings()
{
	if (!GEngine)
	{
		return;
	}

	// Delegate to ApplyGraphicsPreset for quality levels, Nanite, and Lumen
	ApplyGraphicsPreset(Settings.CurrentPreset);

	UWorld* World = GetWorld();

	// Apply additional individual settings not covered by presets
	// Nanite override: allow individual toggle if supported and not overridden by macOS fallback
#if !PLATFORM_MAC
	if (IsNaniteSupported() && !Settings.bEnableNanite)
	{
		GEngine->Exec(World, TEXT("r.Nanite 0"));
	}
#endif

	// Lumen override: allow individual toggle
#if !PLATFORM_MAC
	if (!Settings.bEnableLumen)
	{
		GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect 0"));
		GEngine->Exec(World, TEXT("r.Lumen.Reflections 0"));
	}
#endif

	// Motion blur (not part of presets)
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

// ---------------------------------------------------------------------------
// Save Recovery (called by ErrorRecoveryWidget)
// ---------------------------------------------------------------------------

bool UNomiGameInstance::RestoreSettingsFromBackup()
{
	if (RecoverFromBackup(SettingsFileName))
	{
		// Read the recovered file directly (avoid calling LoadSettings which would
		// trigger another corruption check and potentially re-enter this method)
		FString FilePath = GetSaveFilePath(SettingsFileName);
		FString JsonString;
		if (FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				const TSharedPtr<FJsonObject>* DataObj;
				if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
				{
					// Audio
					(*DataObj)->TryGetNumberField(TEXT("MasterVolume"), Settings.MasterVolume);
					(*DataObj)->TryGetNumberField(TEXT("SFXVolume"), Settings.SFXVolume);
					(*DataObj)->TryGetNumberField(TEXT("MusicVolume"), Settings.MusicVolume);

					// Graphics
					(*DataObj)->TryGetNumberField(TEXT("GraphicsQuality"), Settings.GraphicsQuality);
					(*DataObj)->TryGetBoolField(TEXT("EnableNanite"), Settings.bEnableNanite);
					(*DataObj)->TryGetBoolField(TEXT("EnableLumen"), Settings.bEnableLumen);
					(*DataObj)->TryGetBoolField(TEXT("EnableMotionBlur"), Settings.bEnableMotionBlur);
					{
						int32 PresetInt = static_cast<int32>(Settings.CurrentPreset);
						(*DataObj)->TryGetNumberField(TEXT("CurrentPreset"), PresetInt);
						Settings.CurrentPreset = static_cast<ENIOGraphicsPreset>(PresetInt);
					}

					// Gameplay
					(*DataObj)->TryGetNumberField(TEXT("NOMIFrequency"), Settings.NOMIFrequency);
					{
						int32 VehicleInt = static_cast<int32>(Settings.SelectedVehicle);
						(*DataObj)->TryGetNumberField(TEXT("SelectedVehicle"), VehicleInt);
						Settings.SelectedVehicle = static_cast<ENIOVehicleType>(VehicleInt);
					}
					(*DataObj)->TryGetStringField(TEXT("SelectedTrack"), Settings.SelectedTrack);
					(*DataObj)->TryGetNumberField(TEXT("Difficulty"), Settings.Difficulty);
					(*DataObj)->TryGetNumberField(TEXT("NumLaps"), Settings.NumLaps);
					(*DataObj)->TryGetNumberField(TEXT("NumAIOpponents"), Settings.NumAIOpponents);
					(*DataObj)->TryGetStringField(TEXT("GameMode"), Settings.GameMode);

					// Progress & Tutorial
					(*DataObj)->TryGetNumberField(TEXT("SeasonPoints"), Settings.SeasonPoints);
					(*DataObj)->TryGetBoolField(TEXT("TutorialCompleted"), Settings.bTutorialCompleted);

					ApplyGraphicsSettings();
					ApplyAudioSettings();
					UE_LOG(LogNomiRacing, Log, TEXT("Settings restored from backup successfully"));
					OnSaveRecoveryCompleted.Broadcast(SettingsFileName, true);
					return true;
				}
			}
		}
	}

	UE_LOG(LogNomiRacing, Warning, TEXT("Failed to restore settings from backup"));
	OnSaveRecoveryCompleted.Broadcast(SettingsFileName, false);
	return false;
}

void UNomiGameInstance::ResetSettingsToDefaults()
{
	ResetSettings();
	bool bSaved = SaveSettings();
	UE_LOG(LogNomiRacing, Log, TEXT("Settings reset to defaults (save %s)"), bSaved ? TEXT("succeeded") : TEXT("failed"));
	OnSaveRecoveryCompleted.Broadcast(SettingsFileName, bSaved);
}
