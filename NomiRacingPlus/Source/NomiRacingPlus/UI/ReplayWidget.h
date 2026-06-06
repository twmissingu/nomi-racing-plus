#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReplayWidget.generated.h"

class UButton;
class UTextBlock;
class UProgressBar;
class UCameraSystem;

/**
 * Replay playback UI widget.
 * Provides play/pause, stop, speed control, camera angle switching,
 * and a timeline scrubber for the replay system.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UReplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bind a camera system to this widget for replay control. */
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void SetCameraSystem(UCameraSystem* System);

	/** Call each frame to refresh the timeline bar and time display. */
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void UpdateTimeline(float CurrentTime, float TotalTime);

	/** Update telemetry overlay with current vehicle data. */
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void UpdateTelemetry(float Speed, float RPM, float Throttle, float Brake, float Steering, float GForceLong, float GForceLat, float TireTempFL, float TireTempFR, float TireTempRL, float TireTempRR);

protected:
	virtual void NativeConstruct() override;

	// ---- Playback controls ----

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayPauseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StopButton;

	// ---- Speed controls ----

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Speed025Btn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Speed05Btn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Speed1xBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Speed2xBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Speed4xBtn;

	// ---- Camera angle controls ----

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraChaseBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraCinematicBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraFreeBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraTrackBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraTopBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CameraBumperBtn;

	// ---- Display elements ----

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeedText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> TimelineBar;

	// ---- Telemetry overlay ----

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TelemetryToggleBtn;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetrySpeedText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetryRPMText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetryThrottleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetryBrakeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetrySteeringText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetryGForceText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TelemetryTireTempText;

	UPROPERTY(BlueprintReadWrite, Category = "Replay")
	bool bShowTelemetry = false;

	// ---- State ----

	UPROPERTY(BlueprintReadWrite, Category = "Replay")
	bool bIsPlaying = false;

	UPROPERTY(BlueprintReadWrite, Category = "Replay")
	float PlaybackSpeed = 1.0f;

private:
	UPROPERTY()
	TObjectPtr<UCameraSystem> CameraSystem;

	// ---- Playback handlers ----

	UFUNCTION()
	void OnPlayPause();

	UFUNCTION()
	void OnStop();

	// ---- Speed button handlers (AddDynamic requires zero-arg UFUNCTIONs) ----

	UFUNCTION() void OnSpeed025();
	UFUNCTION() void OnSpeed05();
	UFUNCTION() void OnSpeed1x();
	UFUNCTION() void OnSpeed2x();
	UFUNCTION() void OnSpeed4x();

	// ---- Camera angle button handlers ----

	UFUNCTION() void OnCameraChase();
	UFUNCTION() void OnCameraCinematic();
	UFUNCTION() void OnCameraFree();
	UFUNCTION() void OnCameraTrack();
	UFUNCTION() void OnCameraTop();
	UFUNCTION() void OnCameraBumper();

	// ---- Telemetry toggle ----

	UFUNCTION() void OnTelemetryToggle();

	// ---- Internal helpers ----

	void OnSpeedChanged(float NewSpeed);
	void OnCameraAngleChanged(int32 CameraIndex);
	void UpdateButtonStates();
};
