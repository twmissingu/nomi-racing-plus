// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CameraSystem.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

// ── Chase Camera ─────────────────────────────────────────────────────────────

void UCameraSystem::UpdateChaseCamera(float DeltaTime, const FCameraConfig& Config)
{
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FRotator VehicleRotation = VehicleActor->GetActorRotation();
	float VehicleSpeed = GetVehicleSpeed();

	// Speed-dependent distance and height
	float SpeedNorm = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 300.0f), FVector2D(0.0f, 1.0f), VehicleSpeed);
	float DynamicDistance = Config.Distance + Config.SpeedDistanceOffset * SpeedNorm;
	float DynamicHeight = Config.Height + Config.SpeedHeightOffset * SpeedNorm;

	// Calculate desired position behind vehicle
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector DesiredLocation = VehicleLocation
		- ForwardDir * DynamicDistance
		+ FVector(0.0f, 0.0f, DynamicHeight);

	// Look-at target slightly above vehicle
	FVector LookAtTarget = VehicleLocation + FVector(0.0f, 0.0f, Config.LookAtHeight);

	// Handle look-back: flip the camera behind
	if (bLookingBack)
	{
		DesiredLocation = VehicleLocation
			+ ForwardDir * DynamicDistance
			+ FVector(0.0f, 0.0f, DynamicHeight);
	}

	// Trace to avoid clipping through geometry
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(VehicleActor);

	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, VehicleLocation, DesiredLocation,
		ECC_Visibility, QueryParams))
	{
		// Pull camera closer if blocked
		DesiredLocation = HitResult.Location + (VehicleLocation - HitResult.Location).GetSafeNormal() * 50.0f;
	}

	// Spring dynamics for smooth camera movement
	FVector SpringForce = (DesiredLocation - TargetCameraLocation) * Config.SpringStiffness;
	FVector DampingForce = -SpringVelocity * Config.SpringDamping;
	FVector TotalForce = SpringForce + DampingForce;

	SpringVelocity += TotalForce * DeltaTime;
	SpringOffset += SpringVelocity * DeltaTime;

	// Apply spring offset with limits
	float MaxSpringOffset = DynamicDistance * 0.3f;
	SpringOffset = SpringOffset.GetClampedToMaxSize(MaxSpringOffset);

	TargetCameraLocation = DesiredLocation + SpringOffset;

	// Calculate rotation to look at vehicle
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookAtTarget);
}

// ── Hood Camera ──────────────────────────────────────────────────────────────

void UCameraSystem::UpdateHoodCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position on the hood of the vehicle
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();

	FVector HoodOffset = ForwardDir * 120.0f + UpDir * 80.0f;
	TargetCameraLocation = VehicleLocation + HoodOffset;

	// Look forward along vehicle heading
	FVector LookTarget = TargetCameraLocation + ForwardDir * 1000.0f;
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);

	// Apply look-back: rotate 180 degrees
	if (bLookingBack)
	{
		TargetCameraRotation.Yaw += 180.0f;
	}
}

// ── Cockpit Camera ───────────────────────────────────────────────────────────

void UCameraSystem::UpdateCockpitCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position at driver eye level
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();
	FVector RightDir = VehicleActor->GetActorRightVector();

	// Slightly left of center (driver seat position) and up
	FVector CockpitOffset = ForwardDir * -20.0f
		+ UpDir * 120.0f
		- RightDir * 20.0f;

	TargetCameraLocation = VehicleLocation + CockpitOffset;

	// Look forward
	FVector LookTarget = TargetCameraLocation + ForwardDir * 1000.0f;
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);

	// Apply look-back
	if (bLookingBack)
	{
		TargetCameraRotation.Yaw += 180.0f;
	}
}

// ── Bumper Camera ────────────────────────────────────────────────────────────

void UCameraSystem::UpdateBumperCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position at front bumper, low to ground
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();

	FVector BumperOffset = ForwardDir * 220.0f + UpDir * 35.0f;
	TargetCameraLocation = VehicleLocation + BumperOffset;

	// Look forward and slightly up
	FVector LookTarget = VehicleLocation + ForwardDir * 2000.0f + FVector(0.0f, 0.0f, Config.LookAtHeight);
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);

	if (bLookingBack)
	{
		TargetCameraRotation.Yaw += 180.0f;
	}
}

// ── Free Camera ──────────────────────────────────────────────────────────────

void UCameraSystem::UpdateFreeCamera(float DeltaTime, const FCameraConfig& Config)
{
	FVector VehicleLocation = VehicleActor->GetActorLocation();

	// Build orbit rotation from yaw/pitch
	FRotator OrbitRotation(FreeCamOrbitPitch, FreeCamOrbitYaw, 0.0f);
	FVector OrbitOffset = OrbitRotation.Vector() * (-Config.Distance);
	OrbitOffset.Z = Config.Height;

	TargetCameraLocation = VehicleLocation + OrbitOffset;

	// Look at vehicle
	FVector LookAtTarget = VehicleLocation + FVector(0.0f, 0.0f, Config.LookAtHeight);
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookAtTarget);
}

// ── Cinematic Camera ─────────────────────────────────────────────────────────

void UCameraSystem::UpdateCinematicCamera(float DeltaTime, const FCameraConfig& Config)
{
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FRotator VehicleRotation = VehicleActor->GetActorRotation();
	float VehicleSpeed = GetVehicleSpeed();

	CinematicShotTimer += DeltaTime;

	// Advance cinematic angle based on speed
	float SpeedFactor = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 200.0f), FVector2D(0.5f, 2.0f), VehicleSpeed);
	CinematicAngle += 20.0f * SpeedFactor * DeltaTime;

	// Cycle through cinematic shots
	if (bCinematicAutoCycle && CinematicShotTimer >= CinematicShotDuration)
	{
		CinematicShotTimer = 0.0f;
		CinematicShotIndex = (CinematicShotIndex + 1) % static_cast<int32>(ECinematicShotType::Count);

		// Update current shot type
		CurrentCinematicShot = static_cast<ECinematicShotType>(CinematicShotIndex);

		// Start transition blend for cinematic
		CinematicTransitionAlpha = 0.0f;
		CinematicTransitionStart = TargetCameraLocation;
		CinematicTransitionStartRot = TargetCameraRotation;
	}

	// Calculate shot parameters based on shot type
	float Distance = Config.Distance;
	float Height = Config.Height;

	FVector ShotLocation = CalculateCinematicShotPosition(
		CurrentCinematicShot, VehicleLocation, VehicleRotation, CinematicAngle, Distance, Height);

	// Blend cinematic transition
	if (CinematicTransitionAlpha < 1.0f)
	{
		CinematicTransitionAlpha += DeltaTime / 1.5f;
		CinematicTransitionAlpha = FMath::Clamp(CinematicTransitionAlpha, 0.0f, 1.0f);
		float BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, CinematicTransitionAlpha, 2.0f);
		TargetCameraLocation = FMath::Lerp(CinematicTransitionStart, ShotLocation, BlendAlpha);
	}
	else
	{
		TargetCameraLocation = ShotLocation;
	}

	// Look at vehicle
	FVector LookAtTarget = VehicleLocation + FVector(0.0f, 0.0f, Config.LookAtHeight);
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookAtTarget);

	// Apply Dutch angle for certain shots
	if (CurrentCinematicShot == ECinematicShotType::DutchAngle)
	{
		TargetCameraRotation.Roll = 15.0f;
	}
}

FVector UCameraSystem::CalculateCinematicShotPosition(ECinematicShotType ShotType, const FVector& VehicleLocation,
	const FRotator& VehicleRotation, float Angle, float Distance, float Height) const
{
	FVector ForwardDir = VehicleRotation.Vector();
	FVector RightDir = FRotationMatrix(VehicleRotation).GetScaledAxis(EAxis::Y);

	switch (ShotType)
	{
	case ECinematicShotType::WideOrbit:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 1000.0f,
				FMath::Sin(OrbitRad) * 1000.0f,
				300.0f);
		}

	case ECinematicShotType::LowCloseUp:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle * 1.5f);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 400.0f,
				FMath::Sin(OrbitRad) * 400.0f,
				50.0f);
		}

	case ECinematicShotType::BirdsEye:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle * 0.5f);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 300.0f,
				FMath::Sin(OrbitRad) * 300.0f,
				600.0f);
		}

	case ECinematicShotType::DramaticLowAngle:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle * 0.8f);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 600.0f,
				FMath::Sin(OrbitRad) * 600.0f,
				30.0f);
		}

	case ECinematicShotType::TrackingSide:
		{
			// Track from the side, moving with vehicle
			return VehicleLocation + RightDir * 700.0f + FVector(0.0f, 0.0f, 150.0f);
		}

	case ECinematicShotType::FrontTracking:
		{
			// Track from front
			return VehicleLocation + ForwardDir * 800.0f + FVector(0.0f, 0.0f, 200.0f);
		}

	case ECinematicShotType::DutchAngle:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle * 1.2f);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 500.0f,
				FMath::Sin(OrbitRad) * 500.0f,
				250.0f);
		}

	case ECinematicShotType::OverShoulder:
		{
			// Behind and slightly to the right
			return VehicleLocation - ForwardDir * 300.0f + RightDir * 150.0f + FVector(0.0f, 0.0f, 180.0f);
		}

	case ECinematicShotType::Helicopter:
		{
			// High orbit, further away
			float OrbitRad = FMath::DegreesToRadians(Angle * 0.3f);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 1200.0f,
				FMath::Sin(OrbitRad) * 1200.0f,
				800.0f);
		}

	case ECinematicShotType::StaticBumper:
		{
			// Low, close, static relative to ground
			return VehicleLocation + ForwardDir * 250.0f + FVector(0.0f, 0.0f, 30.0f);
		}

	default:
		{
			float OrbitRad = FMath::DegreesToRadians(Angle);
			return VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 800.0f,
				FMath::Sin(OrbitRad) * 800.0f,
				300.0f);
		}
	}
}

// ── Mode Switching ───────────────────────────────────────────────────────────

void UCameraSystem::SetCameraMode(ECameraMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	// Start transition blend
	bIsTransitioning = true;
	TransitionAlpha = 0.0f;
	TransitionStartLocation = CurrentCameraLocation;
	TransitionStartRotation = CurrentCameraRotation;
	TransitionStartFOV = CurrentFOV;

	// Reset spring dynamics
	SpringVelocity = FVector::ZeroVector;
	SpringOffset = FVector::ZeroVector;

	ECameraMode PreviousMode = CurrentMode;
	CurrentMode = NewMode;

	// Reset cinematic state when entering cinematic mode
	if (NewMode == ECameraMode::Cinematic)
	{
		CinematicShotIndex = 0;
		CinematicShotTimer = 0.0f;
		CinematicAngle = 0.0f;
		CurrentCinematicShot = ECinematicShotType::WideOrbit;
		CinematicTransitionAlpha = 1.0f; // Start fully blended
	}

	// Reset replay state when entering replay mode
	if (NewMode == ECameraMode::Replay && ReplayData.Num() == 0)
	{
		UE_LOG(LogNomiCamera, Warning, TEXT("No replay data available, entering replay mode without playback"));
	}

	UE_LOG(LogNomiCamera, Log, TEXT("Camera mode changed: %d -> %d"),
		static_cast<int32>(PreviousMode), static_cast<int32>(NewMode));
}

void UCameraSystem::CycleCameraMode()
{
	// Cycle through gameplay modes (skip Replay, which is entered explicitly)
	static const TArray<ECameraMode> CycleOrder = {
		ECameraMode::Chase,
		ECameraMode::Hood,
		ECameraMode::Cockpit,
		ECameraMode::Bumper,
		ECameraMode::Cinematic,
		ECameraMode::Free
	};

	int32 CurrentIndex = CycleOrder.IndexOfByKey(CurrentMode);
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}

	int32 NextIndex = (CurrentIndex + 1) % CycleOrder.Num();
	SetCameraMode(CycleOrder[NextIndex]);
}
