// NIO Racing Plus - Replay Voice Manager

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NOMI/CommentaryEngine.h"
#include "ReplayVoiceManager.generated.h"

class USoundBase;
class UAudioComponent;

/**
 * Voice event recorded during replay
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FReplayVoiceEvent
{
	GENERATED_BODY()

	// Timestamp in replay (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Replay|Voice")
	float Timestamp = 0.0f;

	// Voice line text
	UPROPERTY(BlueprintReadOnly, Category = "Replay|Voice")
	FString VoiceText;

	// Emotion for visual feedback
	UPROPERTY(BlueprintReadOnly, Category = "Replay|Voice")
	ENOMIEmotion Emotion = ENOMIEmotion::Happy;

	// Sound asset to play (optional)
	UPROPERTY(BlueprintReadOnly, Category = "Replay|Voice")
	TObjectPtr<USoundBase> SoundAsset = nullptr;

	// Has this event been played?
	UPROPERTY(BlueprintReadOnly, Category = "Replay|Voice")
	bool bHasBeenPlayed = false;
};

/**
 * Replay Voice Manager - handles voice playback during replay
 * Records voice events and plays them back at appropriate timestamps
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UReplayVoiceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UReplayVoiceManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === Recording API ===

	// Record a voice event at current replay time
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void RecordVoiceEvent(float Timestamp, const FString& VoiceText, ENOMIEmotion Emotion, USoundBase* SoundAsset = nullptr);

	// Record voice event from CommentaryEngine comment
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void RecordCommentaryEvent(float Timestamp, const FNOMIComment& Comment);

	// === Playback API ===

	// Start voice playback for current replay
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void StartPlayback();

	// Stop voice playback
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void StopPlayback();

	// Seek to specific time (for replay scrubbing)
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void SeekToTime(float NewTime);

	// Set playback speed multiplier
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	void SetPlaybackSpeed(float Speed);

	// === State Access ===

	// Get current voice event (if any)
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	const FReplayVoiceEvent& GetCurrentVoiceEvent() const { return CurrentVoiceEvent; }

	// Is a voice line currently playing?
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	bool IsVoicePlaying() const { return bIsPlaying; }

	// Get total number of recorded voice events
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	int32 GetVoiceEventCount() const { return VoiceEvents.Num(); }

	// Get voice events in time range
	UFUNCTION(BlueprintCallable, Category = "Replay|Voice")
	TArray<FReplayVoiceEvent> GetVoiceEventsInRange(float StartTime, float EndTime) const;

	// === Configuration ===

	// Voice volume multiplier (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replay|Voice")
	float VoiceVolume = 1.0f;

	// Enable/disable voice playback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replay|Voice")
	bool bVoiceEnabled = true;

protected:
	virtual void BeginPlay() override;

private:
	// Recorded voice events (sorted by timestamp)
	UPROPERTY()
	TArray<FReplayVoiceEvent> VoiceEvents;

	// Current playback state
	bool bIsPlaying = false;
	float CurrentPlaybackTime = 0.0f;
	float PlaybackSpeed = 1.0f;
	int32 NextEventIndex = 0;

	// Current voice event
	FReplayVoiceEvent CurrentVoiceEvent;

	// Audio component for voice playback
	UPROPERTY()
	TObjectPtr<UAudioComponent> VoiceAudioComponent;

	// Internal methods
	void ProcessVoiceEvents(float DeltaTime);
	void PlayVoiceEvent(const FReplayVoiceEvent& Event);
	void ResetPlaybackState();
};
