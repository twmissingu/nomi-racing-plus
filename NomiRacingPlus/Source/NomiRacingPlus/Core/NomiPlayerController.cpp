// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Core/NomiPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "NomiRacingPlus.h"

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

	UE_LOG(LogNomiRacing, Log, TEXT("Player controller initialized"));
}

void ANomiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Cast to enhanced input component
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Failed to get EnhancedInputComponent"));
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

void ANomiPlayerController::SetCameraMode(int32 Mode)
{
	CameraMode = FMath::Clamp(Mode, 0, CameraModeNames.Num() - 1);
	UE_LOG(LogNomiRacing, Log, TEXT("Camera mode: %s"), *CameraModeNames[CameraMode]);
}

void ANomiPlayerController::CycleCameraMode()
{
	CameraMode = (CameraMode + 1) % CameraModeNames.Num();
	UE_LOG(LogNomiRacing, Log, TEXT("Camera mode: %s"), *CameraModeNames[CameraMode]);
}

void ANomiPlayerController::OnThrottleTriggered(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;

	ThrottleValue = Value.Get<float>();

	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(GetPawn()))
	{
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
		if (UNIOVehicleMovementComponent* Movement = Vehicle->FindComponentByClass<UNIOVehicleMovementComponent>())
		{
			Movement->SetHandbrakeInput(false);
		}
	}
}

void ANomiPlayerController::OnLookBackStarted(const FInputActionValue& Value)
{
	bLookingBack = true;
	// TODO: Implement camera look back
}

void ANomiPlayerController::OnLookBackCompleted(const FInputActionValue& Value)
{
	bLookingBack = false;
	// TODO: Reset camera
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
	// TODO: Implement pause menu
	UE_LOG(LogNomiRacing, Log, TEXT("Pause requested"));
}
