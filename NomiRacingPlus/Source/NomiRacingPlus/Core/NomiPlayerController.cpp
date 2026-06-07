// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiPlayerController.h"
#include "NomiRaceGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Vehicles/VehicleStateManager.h"
#include "Race/RaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"
#include "UI/ErrorToastWidget.h"
#include "UI/ErrorRecoveryWidget.h"
#include "NomiGameInstance.h"

ANomiPlayerController::ANomiPlayerController()
{
}

void ANomiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMapping.MappingContext)
		{
			Subsystem->AddMappingContext(InputMapping.MappingContext, 0);
		}
	}

	// Create and initialize menu manager
	MenuManager = NewObject<UMenuManager>(this, TEXT("MenuManager"));
	MenuManager->RegisterComponent();
	MenuManager->Initialize(this);

	// Create error toast widget (auto-binds to NomiError::OnError)
	ErrorToastWidget = CreateWidget<UErrorToastWidget>(this, UErrorToastWidget::StaticClass());
	if (ErrorToastWidget)
	{
		ErrorToastWidget->AddToViewport(200); // High Z-order so toasts appear above everything
	}

	// Create error recovery widget (hidden by default, shown on save corruption)
	ErrorRecoveryWidget = CreateWidget<UErrorRecoveryWidget>(this, UErrorRecoveryWidget::StaticClass());
	if (ErrorRecoveryWidget)
	{
		ErrorRecoveryWidget->AddToViewport(150); // Below toasts, above menus

		// Wire to GameInstance save corruption events
		if (UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetGameInstance()))
		{
			GI->OnSaveCorruptionDetected.AddDynamic(this, &ANomiPlayerController::OnSaveCorruptionDetected);
		}

		// Wire recovery action handler
		ErrorRecoveryWidget->OnRecoveryActionSelected.AddDynamic(this, &ANomiPlayerController::OnRecoveryActionHandled);

		// Check if corruption was detected during GameInstance::Init (before we could bind)
		if (UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetGameInstance()))
		{
			if (GI->bHasCorruption)
			{
				FString UserMessage = FString::Printf(TEXT("Save file '%s' is corrupted."), *GI->CorruptedFileName);
				ErrorRecoveryWidget->ShowRecoveryDialog(UserMessage, GI->bCanRestoreBackup);
				GI->bHasCorruption = false; // Only show once
			}
		}
	}

	// Only show the main menu when NOT on a race track.
	// Race tracks (TestTrack, NIOCityCircuit, etc.) use NomiRaceGameMode which
	// auto-starts the race and manages input.  The main menu's FInputModeUIOnly
	// would block all game input and overlay the viewport.
	bool bIsRaceTrack = false;
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GM = World->GetAuthGameMode())
		{
			bIsRaceTrack = GM->IsA<ANomiRaceGameMode>();
		}
	}

	if (!bIsRaceTrack)
	{
		MenuManager->ShowMainMenu();
	}

	UE_LOG(LogNomiRacing, Log, TEXT("Player controller initialized (race track: %s)"),
		bIsRaceTrack ? TEXT("yes") : TEXT("no"));
}

void ANomiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Cast to enhanced input component
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Input"), TEXT("Failed to get EnhancedInputComponent"));
		return;
	}

	// Bind input actions
	if (InputMapping.Actions.ThrottleAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.ThrottleAction, ETriggerEvent::Triggered, this, &ANomiPlayerController::OnThrottleTriggered);
		EnhancedInput->BindAction(InputMapping.Actions.ThrottleAction, ETriggerEvent::Completed, this, &ANomiPlayerController::OnThrottleCompleted);
	}

	if (InputMapping.Actions.BrakeAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.BrakeAction, ETriggerEvent::Triggered, this, &ANomiPlayerController::OnBrakeTriggered);
		EnhancedInput->BindAction(InputMapping.Actions.BrakeAction, ETriggerEvent::Completed, this, &ANomiPlayerController::OnBrakeCompleted);
	}

	if (InputMapping.Actions.SteerAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.SteerAction, ETriggerEvent::Triggered, this, &ANomiPlayerController::OnSteerTriggered);
		EnhancedInput->BindAction(InputMapping.Actions.SteerAction, ETriggerEvent::Completed, this, &ANomiPlayerController::OnSteerCompleted);
	}

	if (InputMapping.Actions.HandbrakeAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.HandbrakeAction, ETriggerEvent::Triggered, this, &ANomiPlayerController::OnHandbrakeTriggered);
		EnhancedInput->BindAction(InputMapping.Actions.HandbrakeAction, ETriggerEvent::Completed, this, &ANomiPlayerController::OnHandbrakeCompleted);
	}

	if (InputMapping.Actions.LookBackAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.LookBackAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnLookBackStarted);
		EnhancedInput->BindAction(InputMapping.Actions.LookBackAction, ETriggerEvent::Completed, this, &ANomiPlayerController::OnLookBackCompleted);
	}

	if (InputMapping.Actions.ChangeCameraAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.ChangeCameraAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnChangeCameraStarted);
	}

	if (InputMapping.Actions.HornAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.HornAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnHornStarted);
	}

	if (InputMapping.Actions.HeadlightsAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.HeadlightsAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnHeadlightsStarted);
	}

	if (InputMapping.Actions.PauseAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.PauseAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnPauseStarted);
	}

	if (InputMapping.Actions.ResetAction)
	{
		EnhancedInput->BindAction(InputMapping.Actions.ResetAction, ETriggerEvent::Started, this, &ANomiPlayerController::OnResetStarted);
	}
}

void ANomiPlayerController::SetInputMapping(const FVehicleInputMapping& NewMapping)
{
	// Remove old mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMapping.MappingContext)
		{
			Subsystem->RemoveMappingContext(InputMapping.MappingContext);
		}
	}

	InputMapping = NewMapping;

	// Add new mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMapping.MappingContext)
		{
			Subsystem->AddMappingContext(InputMapping.MappingContext, 0);
		}
	}
}

void ANomiPlayerController::SetInputEnabled(bool bEnabled)
{
	bInputEnabled = bEnabled;

	if (!bEnabled)
	{
		// Reset input values
		ThrottleValue = 0.0f;
		BrakeValue = 0.0f;
		SteerValue = 0.0f;
		bHandbrakeActive = false;
	}
}

void ANomiPlayerController::SetCameraModeByIndex(int32 Mode)
{
	if (CameraModeNames.Num() == 0)
	{
		return;
	}

	CameraMode = FMath::Clamp(Mode, 0, CameraModeNames.Num() - 1);

	// Forward to camera system on vehicle
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		if (UCameraSystem* CamSys = Vehicle->FindComponentByClass<UCameraSystem>())
		{
			CamSys->SetCameraMode(static_cast<ECameraMode>(CameraMode));
		}
	}

	UE_LOG(LogNomiRacing, Log, TEXT("Camera mode: %s"), *CameraModeNames[CameraMode]);
}

void ANomiPlayerController::CycleCameraMode()
{
	// Delegate camera cycling entirely to CameraSystem to avoid desync
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		if (UCameraSystem* CamSys = Vehicle->FindComponentByClass<UCameraSystem>())
		{
			CamSys->CycleCameraMode();
			// Read back the current mode from CameraSystem for logging
			CameraMode = static_cast<int32>(CamSys->GetCameraMode());
		}
	}

	if (CameraModeNames.IsValidIndex(CameraMode))
	{
		UE_LOG(LogNomiRacing, Log, TEXT("Camera mode: %s"), *CameraModeNames[CameraMode]);
	}
}

void ANomiPlayerController::OnThrottleTriggered(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;

	ThrottleValue = Value.Get<float>();

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->ThrottleInput = ThrottleValue;
		// Also forward to Chaos Vehicle if available
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetThrottleInput(ThrottleValue);
		}
	}
}

void ANomiPlayerController::OnThrottleCompleted(const FInputActionValue& Value)
{
	ThrottleValue = 0.0f;

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->ThrottleInput = 0.0f;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetThrottleInput(0.0f);
		}
	}
}

void ANomiPlayerController::OnBrakeTriggered(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;

	BrakeValue = Value.Get<float>();

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->BrakeInput = BrakeValue;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetBrakeInput(BrakeValue);
		}
	}
}

void ANomiPlayerController::OnBrakeCompleted(const FInputActionValue& Value)
{
	BrakeValue = 0.0f;

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->BrakeInput = 0.0f;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetBrakeInput(0.0f);
		}
	}
}

void ANomiPlayerController::OnSteerTriggered(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;

	SteerValue = Value.Get<float>();

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->SteeringInput = SteerValue;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetSteeringInput(SteerValue);
		}
	}
}

void ANomiPlayerController::OnSteerCompleted(const FInputActionValue& Value)
{
	SteerValue = 0.0f;

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->SteeringInput = 0.0f;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetSteeringInput(0.0f);
		}
	}
}

void ANomiPlayerController::OnHandbrakeTriggered(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;

	bHandbrakeActive = Value.Get<bool>();

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->bHandbrake = bHandbrakeActive;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetHandbrakeInput(bHandbrakeActive);
		}
	}
}

void ANomiPlayerController::OnHandbrakeCompleted(const FInputActionValue& Value)
{
	bHandbrakeActive = false;

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->bHandbrake = false;
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetHandbrakeInput(false);
		}
	}
}

void ANomiPlayerController::OnLookBackStarted(const FInputActionValue& Value)
{
	bLookingBack = true;

	// Forward to camera system
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		if (UCameraSystem* CamSys = Vehicle->FindComponentByClass<UCameraSystem>())
		{
			CamSys->SetLookBack(true);
		}
	}
}

void ANomiPlayerController::OnLookBackCompleted(const FInputActionValue& Value)
{
	bLookingBack = false;

	// Forward to camera system
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		if (UCameraSystem* CamSys = Vehicle->FindComponentByClass<UCameraSystem>())
		{
			CamSys->SetLookBack(false);
		}
	}
}

void ANomiPlayerController::OnChangeCameraStarted(const FInputActionValue& Value)
{
	CycleCameraMode();
}

void ANomiPlayerController::OnHornStarted(const FInputActionValue& Value)
{
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->PlayHorn();
	}
}

void ANomiPlayerController::OnHeadlightsStarted(const FInputActionValue& Value)
{
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		Vehicle->ToggleHeadlights();
	}
}

void ANomiPlayerController::OnPauseStarted(const FInputActionValue& Value)
{
	TogglePauseMenu();
}

void ANomiPlayerController::OnResetStarted(const FInputActionValue& Value)
{
	OnResetVehicle();
}

void ANomiPlayerController::TogglePauseMenu()
{
	if (!MenuManager)
	{
		return;
	}

	if (MenuManager->GetCurrentState() == EMenuState::Racing)
	{
		MenuManager->ShowPauseMenu();
		UGameplayStatics::SetGamePaused(this, true);
		UE_LOG(LogNomiRacing, Log, TEXT("Game paused via menu"));
	}
	else if (MenuManager->GetCurrentState() == EMenuState::Paused)
	{
		MenuManager->ReturnToPrevious();
		UGameplayStatics::SetGamePaused(this, false);
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
		UE_LOG(LogNomiRacing, Log, TEXT("Game resumed via menu"));
	}
}

void ANomiPlayerController::OnResetVehicle()
{
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
		// Cache VehicleStateManager if not already cached
		if (!CachedVehicleStateManager)
		{
			CachedVehicleStateManager = Vehicle->FindComponentByClass<UVehicleStateManager>();
		}

		if (CachedVehicleStateManager && (CachedVehicleStateManager->IsStuck() || CachedVehicleStateManager->IsFlipped()))
		{
			CachedVehicleStateManager->ResetVehicle();
			UE_LOG(LogNomiRacing, Log, TEXT("Vehicle reset triggered"));
		}
	}
}

bool ANomiPlayerController::IsInMenu() const
{
	return MenuManager && MenuManager->GetCurrentState() != EMenuState::Racing;
}

// ---------------------------------------------------------------------------
// Save Recovery
// ---------------------------------------------------------------------------

void ANomiPlayerController::OnSaveCorruptionDetected(const FString& FileName, bool bCanRestoreBackup)
{
	if (ErrorRecoveryWidget)
	{
		FString UserMessage = FString::Printf(
			TEXT("Your save file \"%s\" could not be loaded.\n\nWould you like to restore from a backup or reset to defaults?"),
			*FileName
		);
		ErrorRecoveryWidget->ShowRecoveryDialog(UserMessage, bCanRestoreBackup);
	}
}

void ANomiPlayerController::OnRecoveryActionHandled(ERecoveryAction Action)
{
	UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetGameInstance());
	if (!GI)
	{
		return;
	}

	switch (Action)
	{
	case ERecoveryAction::RestoreBackup:
		UE_LOG(LogNomiRacing, Log, TEXT("Recovery action: Restoring from backup"));
		GI->RestoreSettingsFromBackup();
		break;

	case ERecoveryAction::ResetDefaults:
		UE_LOG(LogNomiRacing, Log, TEXT("Recovery action: Resetting to defaults"));
		GI->ResetSettingsToDefaults();
		break;

	case ERecoveryAction::Cancel:
		UE_LOG(LogNomiRacing, Log, TEXT("Recovery action: Cancelled by user"));
		break;
	}
}
