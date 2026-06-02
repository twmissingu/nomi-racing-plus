// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

UAudioManager::UAudioManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize default volumes
	VolumeLevels.Add(EAudioCategory::Master, 1.0f);
	VolumeLevels.Add(EAudioCategory::Motor, 0.8f);
	VolumeLevels.Add(EAudioCategory::SFX, 0.8f);
	VolumeLevels.Add(EAudioCategory::Music, 0.6f);
	VolumeLevels.Add(EAudioCategory::UI, 0.7f);
	VolumeLevels.Add(EAudioCategory::NOMI, 0.9f);
}

void UAudioManager::BeginPlay()
{
	Super::BeginPlay();

	// Create motor audio components
	if (MotorSoundConfig.MotorSoundCue)
	{
		MotorAudioComponent = CreateAudioComponent(MotorSoundConfig.MotorSoundCue);
		if (MotorAudioComponent)
		{
			MotorAudioComponent->bAutoActivate = false;
		}
	}

	if (MotorSoundConfig.WindSoundCue)
	{
		WindAudioComponent = CreateAudioComponent(MotorSoundConfig.WindSoundCue);
		if (WindAudioComponent)
		{
			WindAudioComponent->bAutoActivate = false;
		}
	}

	if (MotorSoundConfig.TireSoundCue)
	{
		TireAudioComponent = CreateAudioComponent(MotorSoundConfig.TireSoundCue);
		if (TireAudioComponent)
		{
			TireAudioComponent->bAutoActivate = false;
		}
	}

	if (MotorSoundConfig.RegenSoundCue)
	{
		RegenAudioComponent = CreateAudioComponent(MotorSoundConfig.RegenSoundCue);
		if (RegenAudioComponent)
		{
			RegenAudioComponent->bAutoActivate = false;
		}
	}

	UE_LOG(LogNomiRacing, Log, TEXT("Audio Manager initialized"));
}

void UAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update music fade
	if (bMusicFading && MusicAudioComponent)
	{
		CurrentMusicVolume = FMath::FInterpTo(CurrentMusicVolume, MusicFadeTarget, DeltaTime, MusicFadeSpeed);
		MusicAudioComponent->SetVolumeMultiplier(CurrentMusicVolume * GetVolume(EAudioCategory::Music));

		if (FMath::IsNearlyEqual(CurrentMusicVolume, MusicFadeTarget, 0.01f))
		{
			bMusicFading = false;
			if (CurrentMusicVolume <= 0.01f)
			{
				MusicAudioComponent->Stop();
			}
		}
	}
}

void UAudioManager::SetVolume(EAudioCategory Category, float Volume)
{
	VolumeLevels.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudioManager::GetVolume(EAudioCategory Category) const
{
	const float* Volume = VolumeLevels.Find(Category);
	return Volume ? *Volume : 1.0f;
}

void UAudioManager::SetMuted(EAudioCategory Category, bool bMuted)
{
	if (bMuted)
	{
		MutedCategories.Add(Category);
	}
	else
	{
		MutedCategories.Remove(Category);
	}
}

bool UAudioManager::IsMuted(EAudioCategory Category) const
{
	return MutedCategories.Contains(Category);
}

void UAudioManager::UpdateMotorSound(float RPM, float ThrottleInput, float SpeedKmh)
{
	if (!MotorAudioComponent || !MotorSoundConfig.MotorSoundCue)
	{
		return;
	}

	// Calculate motor sound pitch based on RPM
	float RPMNormalized = FMath::Clamp(RPM / 12000.0f, 0.0f, 1.0f);
	float Pitch = FMath::Lerp(MotorSoundConfig.MinPitch, MotorSoundConfig.MaxPitch, RPMNormalized);

	// Calculate motor sound volume based on throttle
	float Volume = FMath::Lerp(MotorSoundConfig.MinVolume, MotorSoundConfig.MaxVolume, ThrottleInput);

	// Apply to audio component
	MotorAudioComponent->SetPitchMultiplier(Pitch);
	UpdateComponentVolume(MotorAudioComponent, EAudioCategory::Motor, Volume);

	// Start motor sound if not playing
	if (!MotorAudioComponent->IsPlaying() && ThrottleInput > 0.01f)
	{
		MotorAudioComponent->Play();
	}
	else if (MotorAudioComponent->IsPlaying() && ThrottleInput <= 0.01f && RPM < 100.0f)
	{
		MotorAudioComponent->Stop();
	}

	// Update wind sound based on speed
	if (WindAudioComponent)
	{
		float WindVolume = FMath::Clamp(SpeedKmh / 200.0f, 0.0f, 1.0f);
		float WindPitch = FMath::Lerp(0.8f, 1.5f, WindVolume);

		WindAudioComponent->SetPitchMultiplier(WindPitch);
		UpdateComponentVolume(WindAudioComponent, EAudioCategory::Motor, WindVolume);

		if (!WindAudioComponent->IsPlaying() && SpeedKmh > 10.0f)
		{
			WindAudioComponent->Play();
		}
		else if (WindAudioComponent->IsPlaying() && SpeedKmh <= 5.0f)
		{
			WindAudioComponent->Stop();
		}
	}
}

void UAudioManager::PlayMotorStart()
{
	// Play a subtle motor start sound
	if (MotorAudioComponent && MotorSoundConfig.MotorSoundCue)
	{
		MotorAudioComponent->Play();
	}
}

void UAudioManager::PlayMotorStop()
{
	if (MotorAudioComponent)
	{
		MotorAudioComponent->Stop();
	}

	if (WindAudioComponent)
	{
		WindAudioComponent->Stop();
	}
}

void UAudioManager::UpdateTireSound(float SlipAmount, bool bIsDrifting)
{
	if (!TireAudioComponent || !MotorSoundConfig.TireSoundCue)
	{
		return;
	}

	float Volume = FMath::Clamp(SlipAmount / 1.0f, 0.0f, 1.0f);

	if (bIsDrifting)
	{
		Volume = FMath::Max(Volume, 0.5f);
	}

	UpdateComponentVolume(TireAudioComponent, EAudioCategory::SFX, Volume);

	if (!TireAudioComponent->IsPlaying() && Volume > 0.1f)
	{
		TireAudioComponent->Play();
	}
	else if (TireAudioComponent->IsPlaying() && Volume <= 0.05f)
	{
		TireAudioComponent->Stop();
	}
}

void UAudioManager::UpdateRegenSound(bool bIsRegenerating, float Intensity)
{
	if (!RegenAudioComponent || !MotorSoundConfig.RegenSoundCue)
	{
		return;
	}

	if (bIsRegenerating)
	{
		float Volume = FMath::Clamp(Intensity, 0.0f, 1.0f);
		UpdateComponentVolume(RegenAudioComponent, EAudioCategory::Motor, Volume);

		if (!RegenAudioComponent->IsPlaying())
		{
			RegenAudioComponent->Play();
		}
	}
	else
	{
		if (RegenAudioComponent->IsPlaying())
		{
			RegenAudioComponent->Stop();
		}
	}
}

void UAudioManager::PlayCollisionSound(float ImpactForce)
{
	if (CollisionSounds.Num() == 0)
	{
		return;
	}

	// Select random collision sound
	int32 Index = FMath::RandRange(0, CollisionSounds.Num() - 1);
	USoundCue* Sound = CollisionSounds[Index];

	if (Sound && GetOwner())
	{
		// Scale volume by impact force
		float Volume = FMath::Clamp(ImpactForce / 1000.0f, 0.3f, 1.0f);
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetOwner()->GetActorLocation(), Volume);
	}
}

void UAudioManager::PlayHornSound()
{
	if (HornSound && GetOwner())
	{
		UGameplayStatics::PlaySoundAtLocation(this, HornSound, GetOwner()->GetActorLocation());
	}
}

void UAudioManager::PlayCheckpointSound()
{
	if (CheckpointSound)
	{
		UGameplayStatics::PlaySound2D(this, CheckpointSound, GetVolume(EAudioCategory::SFX));
	}
}

void UAudioManager::PlayCountdownBeep(bool bIsFinalBeep)
{
	USoundCue* Sound = bIsFinalBeep ? CountdownFinalSound : CountdownBeepSound;

	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound, GetVolume(EAudioCategory::UI));
	}
}

void UAudioManager::PlayFinishSound()
{
	if (FinishSound)
	{
		UGameplayStatics::PlaySound2D(this, FinishSound, GetVolume(EAudioCategory::SFX));
	}
}

void UAudioManager::PlayMusic(USoundCue* MusicCue)
{
	if (!MusicCue)
	{
		return;
	}

	// Stop and destroy current music component to prevent memory leak
	if (MusicAudioComponent)
	{
		MusicAudioComponent->Stop();
		MusicAudioComponent->DestroyComponent();
		MusicAudioComponent = nullptr;
	}

	// Create new music component
	MusicAudioComponent = CreateAudioComponent(MusicCue);
	if (MusicAudioComponent)
	{
		MusicAudioComponent->bAutoActivate = true;
		MusicAudioComponent->bIsMusic = true;
		CurrentMusicVolume = 1.0f;
		MusicAudioComponent->SetVolumeMultiplier(GetVolume(EAudioCategory::Music));
		MusicAudioComponent->Play();
	}
}

void UAudioManager::StopMusic()
{
	if (MusicAudioComponent)
	{
		MusicAudioComponent->Stop();
	}
}

void UAudioManager::FadeMusic(float TargetVolume, float FadeDuration)
{
	MusicFadeTarget = TargetVolume;
	MusicFadeSpeed = 1.0f / FMath::Max(FadeDuration, 0.01f);
	bMusicFading = true;
}

void UAudioManager::PlayNOMIVoice(USoundCue* VoiceCue)
{
	if (!VoiceCue)
	{
		return;
	}

	// Stop and destroy current NOMI voice component to prevent memory leak
	if (NOMIAudioComponent)
	{
		NOMIAudioComponent->Stop();
		NOMIAudioComponent->DestroyComponent();
		NOMIAudioComponent = nullptr;
	}

	// Play new voice
	NOMIAudioComponent = CreateAudioComponent(VoiceCue);
	if (NOMIAudioComponent)
	{
		NOMIAudioComponent->bAutoActivate = true;
		NOMIAudioComponent->SetVolumeMultiplier(GetVolume(EAudioCategory::NOMI));
		NOMIAudioComponent->Play();
	}
}

UAudioComponent* UAudioManager::CreateAudioComponent(USoundCue* SoundCue)
{
	if (!SoundCue || !GetOwner())
	{
		return nullptr;
	}

	UAudioComponent* AudioComponent = NewObject<UAudioComponent>(GetOwner());
	AudioComponent->SetSound(SoundCue);
	AudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
	AudioComponent->RegisterComponent();

	return AudioComponent;
}

void UAudioManager::UpdateComponentVolume(UAudioComponent* Component, EAudioCategory Category, float BaseVolume)
{
	if (!Component)
	{
		return;
	}

	float CategoryVolume = GetVolume(Category);
	float MasterVolume = GetVolume(EAudioCategory::Master);
	bool bMuted = IsMuted(Category) || IsMuted(EAudioCategory::Master);

	float FinalVolume = bMuted ? 0.0f : BaseVolume * CategoryVolume * MasterVolume;
	Component->SetVolumeMultiplier(FinalVolume);
}
