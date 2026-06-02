// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystemInterface.h"
#include "Core/CameraSystem.h"
#include "NomiPlayerController.generated.h"

/**
 * Input actions for vehicle control
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FVehicleInputActions
{
	GENERATED_BODY()

	// Throttle (accelerator pedal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> ThrottleAction;

	// Brake (brake pedal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> BrakeAction;

	// Steering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> SteerAction;

	// Handbrake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> HandbrakeAction;

	// Look back
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> LookBackAction;

	// Change camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> ChangeCameraAction;

	// Horn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> HornAction;

	// Toggle headlights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> HeadlightsAction;

	// Pause
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;
};

/**
 * Input mapping context for vehicle controls
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FVehicleInputMapping
{
	GENERATED_BODY()

	// Input mapping context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	// Input actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVehicleInputActions Actions;
};

/**
 * NIO Racing Plus Player Controller
 * Handles input, camera, and player-specific logic
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANomiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANomiPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Input Actions

	// Set input mapping context
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputMapping(const FVehicleInputMapping& NewMapping);

	// Enable/disable input
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputEnabled(bool bEnabled);

	// Camera Control

	// Get current camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	int32 GetCameraMode() const { return CameraMode; }

	// Set camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(int32 Mode);

	// Cycle camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CycleCameraMode();

protected:
	// Input mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVehicleInputMapping InputMapping;

	// Current camera mode (0=Chase, 1=Hood, 2=Cockpit, 3=Free)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	int32 CameraMode = 0;

	// Camera mode names
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TArray<FString> CameraModeNames = {TEXT("Chase"), TEXT("Hood"), TEXT("Cockpit"), TEXT("Free")};

private:
	// Input handlers
	void OnThrottleTriggered(const FInputActionValue& Value);
	void OnThrottleCompleted(const FInputActionValue& Value);
	void OnBrakeTriggered(const FInputActionValue& Value);
	void OnBrakeCompleted(const FInputActionValue& Value);
	void OnSteerTriggered(const FInputActionValue& Value);
	void OnSteerCompleted(const FInputActionValue& Value);
	void OnHandbrakeTriggered(const FInputActionValue& Value);
	void OnHandbrakeCompleted(const FInputActionValue& Value);
	void OnLookBackStarted(const FInputActionValue& Value);
	void OnLookBackCompleted(const FInputActionValue& Value);
	void OnChangeCameraStarted(const FInputActionValue& Value);
	void OnHornStarted(const FInputActionValue& Value);
	void OnHeadlightsStarted(const FInputActionValue& Value);
	void OnPauseStarted(const FInputActionValue& Value);

	// Input state
	float ThrottleValue = 0.0f;
	float BrakeValue = 0.0f;
	float SteerValue = 0.0f;
	bool bHandbrakeActive = false;
	bool bLookingBack = false;

	// Is input enabled?
	bool bInputEnabled = true;
};
