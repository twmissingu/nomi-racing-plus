// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystemInterface.h"
#include "CameraSystem.h"
#include "UI/MenuManager.h"
#include "NomiPlayerController.generated.h"

class UVehicleStateManager;
class UErrorToastWidget;
class UErrorRecoveryWidget;
enum class ERecoveryAction : uint8;

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

	// Reset vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> ResetAction;
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

	// Set camera mode by index
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraModeByIndex(int32 Mode);

	// Cycle camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CycleCameraMode();

	// Menu Control

	// Toggle pause menu on/off
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void TogglePauseMenu();

	// Reset vehicle if stuck or flipped
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void OnResetVehicle();

	// Check if currently in a menu (not racing)
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool IsInMenu() const;

	/** Handle save corruption — show recovery dialog */
	UFUNCTION()
	void OnSaveCorruptionDetected(const FString& FileName, bool bCanRestoreBackup);

	/** Handle user's recovery action selection */
	UFUNCTION()
	void OnRecoveryActionHandled(ERecoveryAction Action);

protected:
	// Input mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVehicleInputMapping InputMapping;

	// Current camera mode (0=Chase, 1=Hood, 2=Cockpit, 3=Free)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	int32 CameraMode = 0;

	// Camera mode names (must match ECameraMode enum order)
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TArray<FString> CameraModeNames = {TEXT("Chase"), TEXT("Hood"), TEXT("Cockpit"), TEXT("Bumper"), TEXT("Free"), TEXT("Cinematic"), TEXT("Replay")};

	// Menu manager component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	TObjectPtr<UMenuManager> MenuManager;

	// Error toast widget (created at runtime, added to viewport)
	UPROPERTY()
	TObjectPtr<UErrorToastWidget> ErrorToastWidget;

	// Error recovery widget (created at runtime, added to viewport)
	UPROPERTY()
	TObjectPtr<UErrorRecoveryWidget> ErrorRecoveryWidget;

	// Get error toast widget
	UFUNCTION(BlueprintCallable, Category = "UI")
	UErrorToastWidget* GetErrorToastWidget() const { return ErrorToastWidget; }

	// Get error recovery widget
	UFUNCTION(BlueprintCallable, Category = "UI")
	UErrorRecoveryWidget* GetErrorRecoveryWidget() const { return ErrorRecoveryWidget; }

	// Cached vehicle state manager from controlled pawn
	UPROPERTY()
	TObjectPtr<UVehicleStateManager> CachedVehicleStateManager;

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
	void OnResetStarted(const FInputActionValue& Value);

	// Input state
	float ThrottleValue = 0.0f;
	float BrakeValue = 0.0f;
	float SteerValue = 0.0f;
	bool bHandbrakeActive = false;
	bool bLookingBack = false;

	// Is input enabled?
	bool bInputEnabled = true;
};
