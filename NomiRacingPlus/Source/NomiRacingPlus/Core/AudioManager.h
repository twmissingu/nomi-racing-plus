// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "AudioManager.generated.h"

/**
 * Audio category types
 */
UENUM(BlueprintType)
enum class EAudioCategory : uint8
{
	Master       UMETA(DisplayName = "Master"),
	Motor        UMETA(DisplayName = "Motor"),
	SFX          UMETA(DisplayName = "SFX"),
	Music        UMETA(DisplayName = "Music"),
	UI           UMETA(DisplayName = "UI"),
	NOMI         UMETA(DisplayName = "NOMI")
};

/**
 * Motor sound configuration for electric vehicles
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FElectricMotorSoundConfig
{
	GENERATED_BODY()

	// Base motor sound (looping)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	TObjectPtr<USoundCue> MotorSoundCue;

	// Motor sound pitch range (min RPM -> max RPM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	float MinPitch = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	float MaxPitch = 2.0f;

	// Motor sound volume range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	float MinVolume = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	float MaxVolume = 1.0f;

	// Wind sound (speed-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	TObjectPtr<USoundCue> WindSoundCue;

	// Tire sound (slip-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	TObjectPtr<USoundCue> TireSoundCue;

	// Regenerative braking sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	TObjectPtr<USoundCue> RegenSoundCue;
};

/**
 * Audio Manager - handles all game audio
 * Controls motor sounds, SFX, music, and volume levels
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UAudioManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UAudioManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Volume Control

	// Set volume for category
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetVolume(EAudioCategory Category, float Volume);

	// Get volume for category
	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetVolume(EAudioCategory Category) const;

	// Mute/unmute category
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMuted(EAudioCategory Category, bool bMuted);

	// Is category muted?
	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool IsMuted(EAudioCategory Category) const;

	// Motor Sound Control

	// Update motor sound based on RPM and throttle
	UFUNCTION(BlueprintCallable, Category = "Audio|Motor")
	void UpdateMotorSound(float RPM, float ThrottleInput, float SpeedKmh);

	// Play motor start sound
	UFUNCTION(BlueprintCallable, Category = "Audio|Motor")
	void PlayMotorStart();

	// Play motor stop sound
	UFUNCTION(BlueprintCallable, Category = "Audio|Motor")
	void PlayMotorStop();

	// Update tire sound based on slip
	UFUNCTION(BlueprintCallable, Category = "Audio|Motor")
	void UpdateTireSound(float SlipAmount, bool bIsDrifting);

	// Update regen braking sound
	UFUNCTION(BlueprintCallable, Category = "Audio|Motor")
	void UpdateRegenSound(bool bIsRegenerating, float Intensity);

	// SFX Playback

	// Play collision sound
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
	void PlayCollisionSound(float ImpactForce);

	// Play horn sound
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
	void PlayHornSound();

	// Play checkpoint sound
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
	void PlayCheckpointSound();

	// Play countdown beep
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
	void PlayCountdownBeep(bool bIsFinalBeep);

	// Play finish sound
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
	void PlayFinishSound();

	// Music Control

	// Play background music
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void PlayMusic(USoundCue* MusicCue);

	// Stop background music
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void StopMusic();

	// Fade music
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void FadeMusic(float TargetVolume, float FadeDuration);

	// NOMI Voice

	// Play NOMI comment audio
	UFUNCTION(BlueprintCallable, Category = "Audio|NOMI")
	void PlayNOMIVoice(USoundCue* VoiceCue);

	// MetaSound Integration

	// Play motor sound using MetaSound (RPM-based pitch, throttle-based filter)
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	void PlayMotorMetaSound(float RPM, float Throttle, float Load);

	// Update MetaSound motor parameters in real-time
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	void UpdateMotorMetaSound(float RPM, float Throttle, float Load);

	// Stop MetaSound motor
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	void StopMotorMetaSound();

	// Set environment reverb (tunnel/open/indoor)
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	void SetEnvironmentReverb(const FString& EnvironmentType);

	// Toggle between basic audio and MetaSound
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	void SetUseMetaSound(bool bUseMeta) { bUseMetaSound = bUseMeta; }

	// Is MetaSound enabled?
	UFUNCTION(BlueprintCallable, Category = "Audio|MetaSound")
	bool IsUsingMetaSound() const { return bUseMetaSound; }

protected:
	// Volume levels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<EAudioCategory, float> VolumeLevels;

	// Muted categories
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSet<EAudioCategory> MutedCategories;

	// Motor sound config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Motor")
	FElectricMotorSoundConfig MotorSoundConfig;

	// Collision sounds (random selection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TArray<TObjectPtr<USoundCue>> CollisionSounds;

	// Horn sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TObjectPtr<USoundCue> HornSound;

	// Checkpoint sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TObjectPtr<USoundCue> CheckpointSound;

	// Countdown beep sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TObjectPtr<USoundCue> CountdownBeepSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TObjectPtr<USoundCue> CountdownFinalSound;

	// Finish sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
	TObjectPtr<USoundCue> FinishSound;

private:
	// Audio component references
	UPROPERTY()
	TObjectPtr<UAudioComponent> MotorAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> WindAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> TireAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> RegenAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> NOMIAudioComponent;

	// Current motor state
	float CurrentMotorRPM = 0.0f;
	float CurrentMotorVolume = 0.0f;

	// Music fade state
	bool bMusicFading = false;
	float MusicFadeTarget = 0.0f;
	float MusicFadeSpeed = 0.0f;
	float CurrentMusicVolume = 0.0f;

	// MetaSound support
	bool bUseMetaSound = true;
	TObjectPtr<UAudioComponent> MotorMetaSoundComponent;
	FString CurrentEnvironmentType = TEXT("open");

	// Create audio component
	UAudioComponent* CreateAudioComponent(USoundCue* SoundCue);

	// Update volume for audio component
	void UpdateComponentVolume(UAudioComponent* Component, EAudioCategory Category, float BaseVolume);
};
