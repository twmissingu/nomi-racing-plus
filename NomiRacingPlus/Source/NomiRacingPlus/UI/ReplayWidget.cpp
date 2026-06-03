#include "UI/ReplayWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Core/CameraSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogNomiReplay, Log, All);

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UReplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Playback
	if (PlayPauseButton)
	{
		PlayPauseButton->OnClicked.AddDynamic(this, &UReplayWidget::OnPlayPause);
	}
	if (StopButton)
	{
		StopButton->OnClicked.AddDynamic(this, &UReplayWidget::OnStop);
	}

	// Speed buttons
	if (Speed025Btn) { Speed025Btn->OnClicked.AddDynamic(this, &UReplayWidget::OnSpeed025); }
	if (Speed05Btn)  { Speed05Btn->OnClicked.AddDynamic(this, &UReplayWidget::OnSpeed05); }
	if (Speed1xBtn)  { Speed1xBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnSpeed1x); }
	if (Speed2xBtn)  { Speed2xBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnSpeed2x); }
	if (Speed4xBtn)  { Speed4xBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnSpeed4x); }

	// Camera angle buttons
	if (CameraChaseBtn)     { CameraChaseBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraChase); }
	if (CameraCinematicBtn) { CameraCinematicBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraCinematic); }
	if (CameraFreeBtn)      { CameraFreeBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraFree); }
	if (CameraTrackBtn)     { CameraTrackBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraTrack); }
	if (CameraTopBtn)       { CameraTopBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraTop); }
	if (CameraBumperBtn)    { CameraBumperBtn->OnClicked.AddDynamic(this, &UReplayWidget::OnCameraBumper); }

	UpdateButtonStates();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UReplayWidget::SetCameraSystem(UCameraSystem* System)
{
	CameraSystem = System;
}

void UReplayWidget::UpdateTimeline(float CurrentTime, float TotalTime)
{
	if (TotalTime <= 0.0f)
	{
		return;
	}

	const float Percent = FMath::Clamp(CurrentTime / TotalTime, 0.0f, 1.0f);

	if (TimelineBar)
	{
		TimelineBar->SetPercent(Percent);
	}

	if (TimeText)
	{
		const FTimespan Current = FTimespan::FromSeconds(CurrentTime);
		const FTimespan Total = FTimespan::FromSeconds(TotalTime);
		const FString TimeStr = FString::Printf(
			TEXT("%02d:%02d / %02d:%02d"),
			static_cast<int32>(Current.GetMinutes()),
			Current.GetSeconds(),
			static_cast<int32>(Total.GetMinutes()),
			Total.GetSeconds());
		TimeText->SetText(FText::FromString(TimeStr));
	}
}

// ---------------------------------------------------------------------------
// Playback handlers
// ---------------------------------------------------------------------------

void UReplayWidget::OnPlayPause()
{
	if (!CameraSystem)
	{
		UE_LOG(LogNomiReplay, Warning, TEXT("OnPlayPause: CameraSystem is null"));
		return;
	}

	bIsPlaying = !bIsPlaying;

	if (bIsPlaying)
	{
		CameraSystem->StartPlayback();
	}
	else
	{
		CameraSystem->StopPlayback();
	}

	UpdateButtonStates();
}

void UReplayWidget::OnStop()
{
	if (CameraSystem)
	{
		CameraSystem->StopPlayback();
	}

	bIsPlaying = false;
	PlaybackSpeed = 1.0f;

	SetVisibility(ESlateVisibility::Hidden);

	UpdateButtonStates();
}

// ---------------------------------------------------------------------------
// Speed button wrappers
// ---------------------------------------------------------------------------

void UReplayWidget::OnSpeed025() { OnSpeedChanged(0.25f); }
void UReplayWidget::OnSpeed05()  { OnSpeedChanged(0.5f); }
void UReplayWidget::OnSpeed1x()  { OnSpeedChanged(1.0f); }
void UReplayWidget::OnSpeed2x()  { OnSpeedChanged(2.0f); }
void UReplayWidget::OnSpeed4x()  { OnSpeedChanged(4.0f); }

void UReplayWidget::OnSpeedChanged(float NewSpeed)
{
	PlaybackSpeed = NewSpeed;

	if (CameraSystem)
	{
		CameraSystem->SetPlaybackSpeed(NewSpeed);
	}

	if (SpeedText)
	{
		const FString SpeedStr = (FMath::IsNearlyEqual(NewSpeed, FMath::RoundToFloat(NewSpeed)))
			? FString::Printf(TEXT("%.0fx"), NewSpeed)
			: FString::Printf(TEXT("%.2fx"), NewSpeed);
		SpeedText->SetText(FText::FromString(SpeedStr));
	}

	UpdateButtonStates();
}

// ---------------------------------------------------------------------------
// Camera angle button wrappers
// ---------------------------------------------------------------------------

void UReplayWidget::OnCameraChase()     { OnCameraAngleChanged(0); }
void UReplayWidget::OnCameraCinematic() { OnCameraAngleChanged(1); }
void UReplayWidget::OnCameraFree()      { OnCameraAngleChanged(2); }
void UReplayWidget::OnCameraTrack()     { OnCameraAngleChanged(3); }
void UReplayWidget::OnCameraTop()       { OnCameraAngleChanged(4); }
void UReplayWidget::OnCameraBumper()    { OnCameraAngleChanged(5); }

void UReplayWidget::OnCameraAngleChanged(int32 CameraIndex)
{
	if (CameraSystem)
	{
		CameraSystem->SetCameraMode(static_cast<ECameraMode>(CameraIndex));
	}

	UpdateButtonStates();
}

// ---------------------------------------------------------------------------
// Visual state
// ---------------------------------------------------------------------------

void UReplayWidget::UpdateButtonStates()
{
	struct FSpeedEntry
	{
		TObjectPtr<UButton> Button;
		float Speed;
	};

	const FSpeedEntry SpeedEntries[] = {
		{Speed025Btn, 0.25f},
		{Speed05Btn, 0.5f},
		{Speed1xBtn, 1.0f},
		{Speed2xBtn, 2.0f},
		{Speed4xBtn, 4.0f},
	};

	for (const FSpeedEntry& Entry : SpeedEntries)
	{
		if (!Entry.Button)
		{
			continue;
		}
		// Disable the button that matches the current speed to indicate selection.
		Entry.Button->SetIsEnabled(!FMath::IsNearlyEqual(Entry.Speed, PlaybackSpeed));
	}

	struct FCameraEntry
	{
		TObjectPtr<UButton> Button;
		int32 Index;
	};

	const FCameraEntry CameraEntries[] = {
		{CameraChaseBtn, 0},
		{CameraCinematicBtn, 1},
		{CameraFreeBtn, 2},
		{CameraTrackBtn, 3},
		{CameraTopBtn, 4},
		{CameraBumperBtn, 5},
	};

	const int32 ActiveCamera = CameraSystem ? static_cast<int32>(CameraSystem->GetCameraMode()) : -1;

	for (const FCameraEntry& Entry : CameraEntries)
	{
		if (!Entry.Button)
		{
			continue;
		}
		Entry.Button->SetIsEnabled(Entry.Index != ActiveCamera);
	}
}
