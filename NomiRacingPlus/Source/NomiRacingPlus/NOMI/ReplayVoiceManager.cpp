// NIO Racing Plus - Replay Voice Manager

#include "NOMI/ReplayVoiceManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

UReplayVoiceManager::UReplayVoiceManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplayVoiceManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNomiNOMI, Log, TEXT("ReplayVoiceManager initialized"));
}

void UReplayVoiceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPlaying && bVoiceEnabled)
	{
		ProcessVoiceEvents(DeltaTime);
	}
}

// === Recording API ===

void UReplayVoiceManager::RecordVoiceEvent(float Timestamp, const FString& VoiceText, ENOMIEmotion Emotion, USoundBase* SoundAsset)
{
	FReplayVoiceEvent Event;
	Event.Timestamp = Timestamp;
	Event.VoiceText = VoiceText;
	Event.Emotion = Emotion;
	Event.SoundAsset = SoundAsset;
	Event.bHasBeenPlayed = false;

	VoiceEvents.Add(Event);

	// Sort by timestamp to maintain chronological order
	VoiceEvents.Sort([](const FReplayVoiceEvent& A, const FReplayVoiceEvent& B)
	{
		return A.Timestamp < B.Timestamp;
	});

	UE_LOG(LogNomiNOMI, Verbose, TEXT("Recorded voice event at %.1fs: %s"), Timestamp, *VoiceText);
}

void UReplayVoiceManager::RecordCommentaryEvent(float Timestamp, const FNOMIComment& Comment)
{
	RecordVoiceEvent(Timestamp, Comment.Text, Comment.Emotion, nullptr);
}

// === Playback API ===

void UReplayVoiceManager::StartPlayback()
{
	bIsPlaying = true;
	CurrentPlaybackTime = 0.0f;
	NextEventIndex = 0;

	// Reset all played flags
	for (FReplayVoiceEvent& Event : VoiceEvents)
	{
		Event.bHasBeenPlayed = false;
	}

	UE_LOG(LogNomiNOMI, Log, TEXT("Voice playback started with %d events"), VoiceEvents.Num());
}

void UReplayVoiceManager::StopPlayback()
{
	bIsPlaying = false;

	// Stop any currently playing audio
	if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
	{
		VoiceAudioComponent->Stop();
	}

	ResetPlaybackState();

	UE_LOG(LogNomiNOMI, Log, TEXT("Voice playback stopped"));
}

void UReplayVoiceManager::SeekToTime(float NewTime)
{
	CurrentPlaybackTime = NewTime;

	// Find the next event to play
	NextEventIndex = 0;
	for (int32 i = 0; i < VoiceEvents.Num(); i++)
	{
		if (VoiceEvents[i].Timestamp > NewTime)
		{
			NextEventIndex = i;
			break;
		}
		// Mark events before NewTime as played
		VoiceEvents[i].bHasBeenPlayed = true;
	}

	// Stop current voice if playing
	if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
	{
		VoiceAudioComponent->Stop();
	}
}

void UReplayVoiceManager::SetPlaybackSpeed(float Speed)
{
	PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 10.0f);
}

TArray<FReplayVoiceEvent> UReplayVoiceManager::GetVoiceEventsInRange(float StartTime, float EndTime) const
{
	TArray<FReplayVoiceEvent> Result;

	for (const FReplayVoiceEvent& Event : VoiceEvents)
	{
		if (Event.Timestamp >= StartTime && Event.Timestamp <= EndTime)
		{
			Result.Add(Event);
		}
	}

	return Result;
}

// === Internal Methods ===

void UReplayVoiceManager::ProcessVoiceEvents(float DeltaTime)
{
	// Update playback time
	CurrentPlaybackTime += DeltaTime * PlaybackSpeed;

	// Check if we need to play any voice events
	while (NextEventIndex < VoiceEvents.Num())
	{
		const FReplayVoiceEvent& NextEvent = VoiceEvents[NextEventIndex];

		if (NextEvent.Timestamp <= CurrentPlaybackTime && !NextEvent.bHasBeenPlayed)
		{
			// Play this voice event
			PlayVoiceEvent(NextEvent);
			VoiceEvents[NextEventIndex].bHasBeenPlayed = true;
			NextEventIndex++;
		}
		else if (NextEvent.Timestamp > CurrentPlaybackTime)
		{
			// Haven't reached this event yet
			break;
		}
		else
		{
			// Already played, move to next
			NextEventIndex++;
		}
	}
}

void UReplayVoiceManager::PlayVoiceEvent(const FReplayVoiceEvent& Event)
{
	CurrentVoiceEvent = Event;

	// Play sound if available
	if (Event.SoundAsset)
	{
		VoiceAudioComponent = UGameplayStatics::SpawnSound2D(
			GetOwner(),
			Event.SoundAsset,
			VoiceVolume
		);
	}

	UE_LOG(LogNomiNOMI, Verbose, TEXT("Playing voice at %.1fs: %s"), Event.Timestamp, *Event.VoiceText);
}

void UReplayVoiceManager::ResetPlaybackState()
{
	CurrentPlaybackTime = 0.0f;
	NextEventIndex = 0;
	CurrentVoiceEvent = FReplayVoiceEvent();
}
