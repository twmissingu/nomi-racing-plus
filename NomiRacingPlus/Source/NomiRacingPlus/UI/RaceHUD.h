// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Race/RaceManager.h"
#include "NOMI/CommentaryEngine.h"
#include "RaceHUD.generated.h"

/**
 * HUD data structure for binding
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FHUDData
{
	GENERATED_BODY()

	// Speed (km/h)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float Speed = 0.0f;

	// Current position
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 Position = 0;

	// Total racers
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 TotalRacers = 0;

	// Current lap
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 CurrentLap = 0;

	// Total laps
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 TotalLaps = 0;

	// Race timer
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float RaceTimer = 0.0f;

	// Best lap time
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float BestLapTime = -1.0f;

	// Current lap time
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CurrentLapTime = 0.0f;

	// Throttle input (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float ThrottleInput = 0.0f;

	// Brake input (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float BrakeInput = 0.0f;

	// Steering input (-1 to 1)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float SteeringInput = 0.0f;

	// Is drifting?
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bIsDrifting = false;

	// Drift angle
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float DriftAngle = 0.0f;

	// Battery level (for NIO vehicles)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float BatteryLevel = 100.0f;

	// Is NIO vehicle?
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bIsNIOVehicle = false;

	// NOMI comment text
	UPROPERTY(BlueprintReadOnly, Category = "HUD|NOMI")
	FString NOMICommentText;

	// NOMI emotion
	UPROPERTY(BlueprintReadOnly, Category = "HUD|NOMI")
	ENOMIEmotion NOMIEmotion = ENOMIEmotion::Idle;

	// Is NOMI comment visible?
	UPROPERTY(BlueprintReadOnly, Category = "HUD|NOMI")
	bool bNOMICommentVisible = false;

	// Countdown value (0 = not in countdown)
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CountdownValue = 0.0f;

	// Race state
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	ERaceState RaceState = ERaceState::Idle;
};

/**
 * Race HUD - main in-game HUD widget
 * Displays speed, position, lap, timer, NOMI comments, and controls
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API URaceHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	URaceHUD(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Update HUD data
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUDData(const FHUDData& NewData);

	// Get current HUD data
	UFUNCTION(BlueprintCallable, Category = "HUD")
	const FHUDData& GetHUDData() const { return HUDData; }

	// Show/hide HUD
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHUDVisible(bool bVisible);

	// Show countdown
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowCountdown(float Value);

	// Hide countdown
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideCountdown();

protected:
	// HUD data
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	FHUDData HUDData;

	// Speed text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeedText;

	// Position text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PositionText;

	// Lap text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LapText;

	// Timer text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	// Best lap text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BestLapText;

	// Throttle bar widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ThrottleBar;

	// Brake bar widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BrakeBar;

	// NOMI comment text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NOMICommentText;

	// NOMI comment box widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NOMICommentBox;

	// Countdown text widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	// Countdown box widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> CountdownBox;

	// Battery bar widget binding (NIO vehicles only)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BatteryBar;

	// Battery box widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> BatteryBox;

	// Drift indicator widget binding
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> DriftIndicator;

private:
	// Format time as MM:SS.mmm
	FString FormatTime(float TimeInSeconds) const;

	// Format position with suffix (1st, 2nd, 3rd, etc.)
	FString FormatPosition(int32 Position) const;
};
