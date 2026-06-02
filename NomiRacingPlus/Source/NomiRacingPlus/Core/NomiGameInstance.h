// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Vehicles/VehicleStateManager.h"
#include "Race/RaceProgression.h"
#include "Race/ChampionshipManager.h"
#include "NomiGameInstance.generated.h"

/**
 * Game settings saved to disk
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNomiGameSettings
{
	GENERATED_BODY()

	// Graphics quality level (0=Low, 1=Medium, 2=High)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
	int32 GraphicsQuality = 1;

	// Enable Nanite (Windows only)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
	bool bEnableNanite = true;

	// Enable Lumen
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
bool bEnableLumen = true;

	// Enable motion blur
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
	bool bEnableMotionBlur = true;

	// Master volume (0-1)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MasterVolume = 1.0f;

	// SFX volume (0-1)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float SFXVolume = 0.8f;

	// Music volume (0-1)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MusicVolume = 0.6f;

	// NOMI comment frequency (0=Off, 1=Low, 2=Medium, 3=High)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	int32 NOMIFrequency = 2;

	// Selected vehicle type
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	ENIOVehicleType SelectedVehicle = ENIOVehicleType::EP9;

	// Selected track
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	FString SelectedTrack = TEXT("NIOCityCircuit");

	// Difficulty (0=Easy, 1=Normal, 2=Hard)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	int32 Difficulty = 1;

	// Number of laps
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	int32 NumLaps = 3;

	// Number of AI opponents
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Gameplay")
	int32 NumAIOpponents = 7;

	// Best lap times (track name -> time)
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Progress")
	TMap<FString, float> BestLapTimes;

	// Season points
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Progress")
	int32 SeasonPoints = 0;
};

/**
 * NIO Racing Plus Game Instance
 * Manages global game state, settings, and data persistence
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API UNomiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UNomiGameInstance();

	virtual void Init() override;

	// Settings Management

	// Get current settings
	UFUNCTION(BlueprintCallable, Category = "Settings")
	const FNomiGameSettings& GetSettings() const { return Settings; }

	// Update settings
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void UpdateSettings(const FNomiGameSettings& NewSettings);

	// Save settings to disk
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool SaveSettings();

	// Load settings from disk
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool LoadSettings();

	// Reset settings to defaults
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetSettings();

	// Progress Management

	// Update best lap time for a track
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void UpdateBestLapTime(const FString& TrackName, float LapTime);

	// Get best lap time for a track
	UFUNCTION(BlueprintCallable, Category = "Progress")
	float GetBestLapTime(const FString& TrackName) const;

	// Add season points
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void AddSeasonPoints(int32 Points);

	// Get season points
	UFUNCTION(BlueprintCallable, Category = "Progress")
	int32 GetSeasonPoints() const { return Settings.SeasonPoints; }

	// Save progress
	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool SaveProgress();

	// Load progress
	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool LoadProgress();

	// Progression System

	// Get the race progression component
	UFUNCTION(BlueprintCallable, Category = "Progression")
	URaceProgression* GetRaceProgression() const { return RaceProgression; }

	// Get the championship manager (from GameMode)
	UFUNCTION(BlueprintCallable, Category = "Progression")
	AChampionshipManager* GetChampionshipManager() const;

	// Record a completed race session for progression tracking
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void RecordRaceSession(const FRaceSessionResult& SessionResult);

	// Start a championship by ID
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool StartChampionship(const FString& ChampionshipID);

	// Get player statistics
	UFUNCTION(BlueprintCallable, Category = "Progression")
	const FPlayerStatistics& GetPlayerStatistics() const;

	// Get achievement list
	UFUNCTION(BlueprintCallable, Category = "Progression")
	const TMap<EAchievement, FAchievementData>& GetAchievements() const;

	// Get unlockables list
	UFUNCTION(BlueprintCallable, Category = "Progression")
	const TMap<FString, FUnlockableItem>& GetUnlockables() const;

	// Platform Detection

	// Is running on macOS?
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsMacOS() const;

	// Is Nanite supported?
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsNaniteSupported() const;

	// Is Lumen supported?
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsLumenSupported() const;

protected:
	// Current settings
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FNomiGameSettings Settings;

	// Settings file name
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FString SettingsFileName = TEXT("NomiRacingSettings.json");

	// Progress file name
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FString ProgressFileName = TEXT("NomiRacingProgress.json");

	// Race progression component
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	TObjectPtr<URaceProgression> RaceProgression;

private:
	// Get save file path
	FString GetSaveFilePath(const FString& FileName) const;

	// Apply graphics settings
	void ApplyGraphicsSettings();

	// Apply audio settings
	void ApplyAudioSettings();
};
