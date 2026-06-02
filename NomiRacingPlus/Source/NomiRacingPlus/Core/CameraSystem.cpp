// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CameraSystem.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/VehicleStateManager.h"
#include "NomiRacingPlus.h"
#include "Kismet/KismetMathLibrary.h"

namespace CameraConstants
{
	// Default FOV values per mode
	constexpr float ChaseFOV = 90.0f;
	constexpr float HoodFOV = 100.0f;
	constexpr float CockpitFOV = 110.0f;
	constexpr float BumperFOV = 95.0f;
	constexpr float FreeFOV = 75.0f;
	constexpr float CinematicFOV = 70.0f;
	constexpr float ReplayFOV = 80.0f;

	// FOV speed response
	constexpr float MinSpeedFOV = 90.0f;
	constexpr float MaxSpeedFOV = 115.0f;
	constexpr float FOVSpeedThreshold = 50.0f;  // km/h where FOV starts widening
	constexpr float FOVMaxSpeed = 300.0f;        // km/h for max FOV

	// Chase camera defaults
	constexpr float ChaseDistance = 600.0f;
	constexpr float ChaseHeight = 200.0f;
	constexpr float ChaseLookAtHeight = 100.0f;
	constexpr float ChaseInterpSpeed = 5.0f;
	constexpr float ChaseRotationLag = 8.0f;
	constexpr float ChasePositionLag = 10.0f;

	// Hood camera offsets (relative to vehicle)
	constexpr float HoodOffsetX = 120.0f;
	constexpr float HoodOffsetZ = 80.0f;

	// Cockpit camera offsets (relative to vehicle)
	constexpr float CockpitOffsetX = -20.0f;
	constexpr float CockpitOffsetZ = 120.0f;

	// Bumper camera offsets
	constexpr float BumperOffsetX = 220.0f;
	constexpr float BumperOffsetZ = 35.0f;

	// Free camera
	constexpr float FreeCamDistance = 800.0f;
	constexpr float FreeCamMinPitch = -40.0f;
	constexpr float FreeCamMaxPitch = 20.0f;
	constexpr float FreeCamInterpSpeed = 6.0f;

	// Cinematic camera shots
	constexpr float CinematicMinDistance = 400.0f;
	constexpr float CinematicMaxDistance = 1200.0f;
	constexpr float CinematicHeightMin = 100.0f;
	constexpr float CinematicHeightMax = 500.0f;
	constexpr int32 CinematicNumShots = 10;

	// Cinematic transition blend time
	constexpr float CinematicBlendTime = 1.5f;

	// Shake
	constexpr float ShakeDecayRate = 3.0f;
	constexpr float SpeedShakeBaseFrequency = 2.0f;
	constexpr float MaxSpeedShakeAmplitude = 0.3f;
	constexpr float GearShiftShakeDuration = 0.15f;

	// Replay
	constexpr float ReplayInterpBlendTime = 0.1f;
	constexpr float ReplayOrbitSpeed = 30.0f;

	// Transition
	constexpr float DefaultTransitionDuration = 0.4f;

	// Speed conversion (cm/s to km/h)
	constexpr float CmPerSecToKmPerHour = 0.036f;

	// Spring dynamics
	constexpr float SpringMinStiffness = 100.0f;
	constexpr float SpringMaxStiffness = 500.0f;
	constexpr float SpringMinDamping = 5.0f;
	constexpr float SpringMaxDamping = 25.0f;
}

UCameraSystem::UCameraSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UCameraSystem::BeginPlay()
{
	Super::BeginPlay();

	// Cache vehicle reference
	VehicleActor = GetOwner();

	// Create camera component if not already created
	if (!CameraComponent && VehicleActor)
	{
		CameraComponent = NewObject<UCameraComponent>(VehicleActor, TEXT("VehicleCamera"));
		CameraComponent->SetupAttachment(VehicleActor->GetRootComponent());
		CameraComponent->RegisterComponent();
		UE_LOG(LogNomiCamera, Log, TEXT("CameraComponent created dynamically"));
	}

	// Initialize default camera configs
	InitializeDefaultConfigs();

	// Set initial camera state
	if (const FCameraConfig* Config = CameraConfigs.Find(CurrentMode))
	{
		CurrentFOV = Config->FOV;
	}

	// Initialize speed tracking
	PreviousSpeed = GetVehicleSpeed();

	UE_LOG(LogNomiCamera, Log, TEXT("CameraSystem initialized on %s"),
		VehicleActor ? *VehicleActor->GetName() : TEXT("null"));
}

void UCameraSystem::InitializeDefaultConfigs()
{
	// Chase camera - classic third-person follow with spring dynamics
	FCameraConfig ChaseConfig;
	ChaseConfig.FOV = CameraConstants::ChaseFOV;
	ChaseConfig.SpeedFOVMultiplier = 1.25f;
	ChaseConfig.AccelerationFOVMultiplier = 1.05f;
	ChaseConfig.Distance = CameraConstants::ChaseDistance;
	ChaseConfig.SpeedDistanceOffset = 150.0f;
	ChaseConfig.Height = CameraConstants::ChaseHeight;
	ChaseConfig.SpeedHeightOffset = -40.0f;
	ChaseConfig.LookAtHeight = CameraConstants::ChaseLookAtHeight;
	ChaseConfig.InterpSpeed = CameraConstants::ChaseInterpSpeed;
	ChaseConfig.RotationLag = CameraConstants::ChaseRotationLag;
	ChaseConfig.PositionLag = CameraConstants::ChasePositionLag;
	ChaseConfig.bEnableShake = true;
	ChaseConfig.ShakeIntensity = 0.3f;
	ChaseConfig.SpringStiffness = 300.0f;
	ChaseConfig.SpringDamping = 15.0f;
	ChaseConfig.bEnableHeadBob = false;
	ChaseConfig.SpeedRollAngle = 0.0f;
	CameraConfigs.Add(ECameraMode::Chase, ChaseConfig);

	// Hood camera - mounted on hood, forward-facing
	FCameraConfig HoodConfig;
	HoodConfig.FOV = CameraConstants::HoodFOV;
	HoodConfig.SpeedFOVMultiplier = 1.15f;
	HoodConfig.AccelerationFOVMultiplier = 1.08f;
	HoodConfig.Distance = 0.0f;
	HoodConfig.SpeedDistanceOffset = 0.0f;
	HoodConfig.Height = CameraConstants::HoodOffsetZ;
	HoodConfig.SpeedHeightOffset = 0.0f;
	HoodConfig.LookAtHeight = 0.0f;
	HoodConfig.InterpSpeed = 15.0f;
	HoodConfig.RotationLag = 12.0f;
	HoodConfig.PositionLag = 20.0f;
	HoodConfig.bEnableShake = true;
	HoodConfig.ShakeIntensity = 0.6f;
	HoodConfig.SpringStiffness = 400.0f;
	HoodConfig.SpringDamping = 20.0f;
	HoodConfig.bEnableHeadBob = true;
	HoodConfig.HeadBobAmplitude = 3.0f;
	HoodConfig.HeadBobFrequency = 1.5f;
	HoodConfig.SpeedRollAngle = 0.0f;
	CameraConfigs.Add(ECameraMode::Hood, HoodConfig);

	// Cockpit camera - driver eye level, first person
	FCameraConfig CockpitConfig;
	CockpitConfig.FOV = CameraConstants::CockpitFOV;
	CockpitConfig.SpeedFOVMultiplier = 1.1f;
	CockpitConfig.AccelerationFOVMultiplier = 1.05f;
	CockpitConfig.Distance = 0.0f;
	CockpitConfig.SpeedDistanceOffset = 0.0f;
	CockpitConfig.Height = CameraConstants::CockpitOffsetZ;
	CockpitConfig.SpeedHeightOffset = 0.0f;
	CockpitConfig.LookAtHeight = 0.0f;
	CockpitConfig.InterpSpeed = 20.0f;
	CockpitConfig.RotationLag = 15.0f;
	CockpitConfig.PositionLag = 25.0f;
	CockpitConfig.bEnableShake = true;
	CockpitConfig.ShakeIntensity = 0.8f;
	CockpitConfig.SpringStiffness = 500.0f;
	CockpitConfig.SpringDamping = 25.0f;
	CockpitConfig.bEnableHeadBob = true;
	CockpitConfig.HeadBobAmplitude = 5.0f;
	CockpitConfig.HeadBobFrequency = 2.0f;
	CockpitConfig.SpeedRollAngle = 2.0f;
	CameraConfigs.Add(ECameraMode::Cockpit, CockpitConfig);

	// Bumper camera - low, close to front bumper
	FCameraConfig BumperConfig;
	BumperConfig.FOV = CameraConstants::BumperFOV;
	BumperConfig.SpeedFOVMultiplier = 1.2f;
	BumperConfig.AccelerationFOVMultiplier = 1.07f;
	BumperConfig.Distance = 0.0f;
	BumperConfig.SpeedDistanceOffset = 0.0f;
	BumperConfig.Height = CameraConstants::BumperOffsetZ;
	BumperConfig.SpeedHeightOffset = 0.0f;
	BumperConfig.LookAtHeight = 50.0f;
	BumperConfig.InterpSpeed = 12.0f;
	BumperConfig.RotationLag = 10.0f;
	BumperConfig.PositionLag = 18.0f;
	BumperConfig.bEnableShake = true;
	BumperConfig.ShakeIntensity = 0.7f;
	BumperConfig.SpringStiffness = 350.0f;
	BumperConfig.SpringDamping = 18.0f;
	BumperConfig.bEnableHeadBob = true;
	BumperConfig.HeadBobAmplitude = 4.0f;
	BumperConfig.HeadBobFrequency = 1.8f;
	BumperConfig.SpeedRollAngle = 0.0f;
	CameraConfigs.Add(ECameraMode::Bumper, BumperConfig);

	// Free camera - orbital camera
	FCameraConfig FreeConfig;
	FreeConfig.FOV = CameraConstants::FreeFOV;
	FreeConfig.SpeedFOVMultiplier = 1.0f;
	FreeConfig.AccelerationFOVMultiplier = 1.0f;
	FreeConfig.Distance = CameraConstants::FreeCamDistance;
	FreeConfig.SpeedDistanceOffset = 0.0f;
	FreeConfig.Height = 300.0f;
	FreeConfig.SpeedHeightOffset = 0.0f;
	FreeConfig.LookAtHeight = 80.0f;
	FreeConfig.InterpSpeed = CameraConstants::FreeCamInterpSpeed;
	FreeConfig.RotationLag = 6.0f;
	FreeConfig.PositionLag = 8.0f;
	FreeConfig.bEnableShake = false;
	FreeConfig.SpringStiffness = 200.0f;
	FreeConfig.SpringDamping = 10.0f;
	CameraConfigs.Add(ECameraMode::Free, FreeConfig);

	// Cinematic camera - auto-directing dramatic shots
	FCameraConfig CinematicConfig;
	CinematicConfig.FOV = CameraConstants::CinematicFOV;
	CinematicConfig.SpeedFOVMultiplier = 1.0f;
	CinematicConfig.AccelerationFOVMultiplier = 1.0f;
	CinematicConfig.Distance = 800.0f;
	CinematicConfig.SpeedDistanceOffset = 0.0f;
	CinematicConfig.Height = 300.0f;
	CinematicConfig.SpeedHeightOffset = 0.0f;
	CinematicConfig.LookAtHeight = 80.0f;
	CinematicConfig.InterpSpeed = 3.0f;
	CinematicConfig.RotationLag = 4.0f;
	CinematicConfig.PositionLag = 6.0f;
	CinematicConfig.bEnableShake = true;
	CinematicConfig.ShakeIntensity = 0.15f;
	CinematicConfig.SpringStiffness = 150.0f;
	CinematicConfig.SpringDamping = 8.0f;
	CameraConfigs.Add(ECameraMode::Cinematic, CinematicConfig);

	// Replay camera - smooth playback view
	FCameraConfig ReplayConfig;
	ReplayConfig.FOV = CameraConstants::ReplayFOV;
	ReplayConfig.SpeedFOVMultiplier = 1.15f;
	ReplayConfig.AccelerationFOVMultiplier = 1.05f;
	ReplayConfig.Distance = CameraConstants::ChaseDistance;
	ReplayConfig.SpeedDistanceOffset = 100.0f;
	ReplayConfig.Height = CameraConstants::ChaseHeight;
	ReplayConfig.SpeedHeightOffset = -30.0f;
	ReplayConfig.LookAtHeight = CameraConstants::ChaseLookAtHeight;
	ReplayConfig.InterpSpeed = 4.0f;
	ReplayConfig.RotationLag = 5.0f;
	ReplayConfig.PositionLag = 7.0f;
	ReplayConfig.bEnableShake = false;
	ReplayConfig.SpringStiffness = 250.0f;
	ReplayConfig.SpringDamping = 12.0f;
	CameraConfigs.Add(ECameraMode::Replay, ReplayConfig);
}

void UCameraSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!VehicleActor || !CameraComponent)
	{
		return;
	}

	// Update acceleration tracking
	float CurrentSpeed = GetVehicleSpeed();
	float RawAcceleration = (CurrentSpeed - PreviousSpeed) / FMath::Max(DeltaTime, 0.001f);
	CurrentAcceleration = FMath::FInterpTo(CurrentAcceleration, RawAcceleration, DeltaTime, 1.0f / AccelerationSmoothing);
	PreviousSpeed = CurrentSpeed;

	// Update mode transition blend
	UpdateTransition(DeltaTime);

	// Update camera shake
	UpdateShake(DeltaTime);

	// Update camera for current mode
	UpdateCamera(DeltaTime);

	// Record replay data if recording
	if (bIsRecording)
	{
		ReplayRecordTimer += DeltaTime;
		if (ReplayRecordTimer >= ReplayRecordInterval)
		{
			RecordReplayPoint();
			ReplayRecordTimer = 0.0f;
		}
		RecordingTime += DeltaTime;

		// Auto-stop if max duration exceeded
		if (RecordingTime >= MaxReplayDuration)
		{
			StopRecording();
		}
	}
}

void UCameraSystem::UpdateCamera(float DeltaTime)
{
	const FCameraConfig& Config = GetCameraConfig(CurrentMode);

	// Compute target based on mode
	switch (CurrentMode)
	{
	case ECameraMode::Chase:
		UpdateChaseCamera(DeltaTime, Config);
		break;
	case ECameraMode::Hood:
		UpdateHoodCamera(DeltaTime, Config);
		break;
	case ECameraMode::Cockpit:
		UpdateCockpitCamera(DeltaTime, Config);
		break;
	case ECameraMode::Bumper:
		UpdateBumperCamera(DeltaTime, Config);
		break;
	case ECameraMode::Free:
		UpdateFreeCamera(DeltaTime, Config);
		break;
	case ECameraMode::Cinematic:
		UpdateCinematicCamera(DeltaTime, Config);
		break;
	case ECameraMode::Replay:
		UpdateReplayCamera(DeltaTime);
		break;
	}

	// Apply dynamic FOV
	float VehicleSpeed = GetVehicleSpeed();
	float MaxSpeed = GetVehicleMaxSpeed();
	TargetFOV = CalculateDynamicFOV(Config.FOV, VehicleSpeed, MaxSpeed, CurrentAcceleration);

	// Apply shake offset to target
	if (Config.bEnableShake && bIsShaking)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			float Time = World->GetTimeSeconds();
			FVector ShakeOffset;
			ShakeOffset.X = FMath::Sin(Time * CurrentShake.Frequency * 2.0f * PI) * CurrentShake.Amplitude * Config.ShakeIntensity * CurrentShake.AxisMultiplier.X;
			ShakeOffset.Y = FMath::Cos(Time * CurrentShake.Frequency * 1.7f * PI) * CurrentShake.Amplitude * Config.ShakeIntensity * CurrentShake.AxisMultiplier.Y;
			ShakeOffset.Z = FMath::Sin(Time * CurrentShake.Frequency * 2.3f * PI) * CurrentShake.Amplitude * Config.ShakeIntensity * CurrentShake.AxisMultiplier.Z;
			TargetCameraLocation += ShakeOffset;
		}
	}

	// Apply head bob
	if (Config.bEnableHeadBob && CurrentMode != ECameraMode::Cinematic && CurrentMode != ECameraMode::Replay)
	{
		float SpeedNorm = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 200.0f), FVector2D(0.0f, 1.0f), VehicleSpeed);
		float BobFreq = Config.HeadBobFrequency * (1.0f + SpeedNorm);
		HeadBobAccumulator += DeltaTime * BobFreq;
		float BobOffset = FMath::Sin(HeadBobAccumulator * 2.0f * PI) * Config.HeadBobAmplitude * SpeedNorm;
		TargetCameraLocation.Z += BobOffset;
	}

	// Apply speed roll (subtle lean into turns)
	if (Config.SpeedRollAngle > 0.0f)
	{
		float SteeringInput = 0.0f;
		if (ANIOVehicleBase* NIOVehicle = Cast<ANIOVehicleBase>(VehicleActor))
		{
			SteeringInput = NIOVehicle->GetVehicleState().SteeringInput;
		}
		float TargetRoll = -SteeringInput * Config.SpeedRollAngle * FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 150.0f), FVector2D(0.0f, 1.0f), GetVehicleSpeed());
		TargetCameraRotation.Roll = FMath::FInterpTo(TargetCameraRotation.Roll, TargetRoll, DeltaTime, 5.0f);
	}

	// Smoothly interpolate to target
	float InterpSpeed = Config.InterpSpeed;
	CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, TargetCameraLocation, DeltaTime, InterpSpeed);
	CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, TargetCameraRotation, DeltaTime, Config.RotationLag);
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 8.0f);

	// Apply to camera component
	CameraComponent->SetWorldLocation(CurrentCameraLocation);
	CameraComponent->SetWorldRotation(CurrentCameraRotation);
	CameraComponent->FieldOfView = CurrentFOV;
}

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

void UCameraSystem::UpdateHoodCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position on the hood of the vehicle
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();

	FVector HoodOffset = ForwardDir * CameraConstants::HoodOffsetX + UpDir * CameraConstants::HoodOffsetZ;
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

void UCameraSystem::UpdateCockpitCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position at driver eye level
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();
	FVector RightDir = VehicleActor->GetActorRightVector();

	// Slightly left of center (driver seat position) and up
	FVector CockpitOffset = ForwardDir * CameraConstants::CockpitOffsetX
		+ UpDir * CameraConstants::CockpitOffsetZ
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

void UCameraSystem::UpdateBumperCamera(float DeltaTime, const FCameraConfig& Config)
{
	// Position at front bumper, low to ground
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FVector ForwardDir = VehicleActor->GetActorForwardVector();
	FVector UpDir = VehicleActor->GetActorUpVector();

	FVector BumperOffset = ForwardDir * CameraConstants::BumperOffsetX + UpDir * CameraConstants::BumperOffsetZ;
	TargetCameraLocation = VehicleLocation + BumperOffset;

	// Look forward and slightly up
	FVector LookTarget = VehicleLocation + ForwardDir * 2000.0f + FVector(0.0f, 0.0f, Config.LookAtHeight);
	TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);

	if (bLookingBack)
	{
		TargetCameraRotation.Yaw += 180.0f;
	}
}

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

void UCameraSystem::UpdateCinematicCamera(float DeltaTime, const FCameraConfig& Config)
{
	FVector VehicleLocation = VehicleActor->GetActorLocation();
	FRotator VehicleRotation = VehicleActor->GetActorRotation();
	float VehicleSpeed = GetVehicleSpeed();

	CinematicShotTimer += DeltaTime;

	// Advance cinematic angle based on speed
	float SpeedFactor = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 200.0f), FVector2D(0.5f, 2.0f), VehicleSpeed);
	CinematicAngle += CinematicOrbitSpeed * SpeedFactor * DeltaTime;

	// Cycle through cinematic shots
	if (bCinematicAutoCycle && CinematicShotTimer >= CinematicShotDuration)
	{
		CinematicShotTimer = 0.0f;
		CinematicShotIndex = (CinematicShotIndex + 1) % CameraConstants::CinematicNumShots;

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
		CinematicTransitionAlpha += DeltaTime / CameraConstants::CinematicBlendTime;
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

void UCameraSystem::UpdateReplayCamera(float DeltaTime)
{
	if (ReplayData.Num() < 2)
	{
		return;
	}

	// Don't advance time if paused
	if (!bIsReplayPaused)
	{
		// Advance playback time
		PlaybackTime += DeltaTime * PlaybackSpeed;

		// Loop playback
		float TotalDuration = GetReplayDuration();
		if (TotalDuration > 0.0f && PlaybackTime >= TotalDuration)
		{
			PlaybackTime = 0.0f;
		}
	}

	// Interpolate replay data
	FReplayDataPoint Interpolated = InterpolateReplayData(PlaybackTime);

	// Apply camera based on replay camera angle
	FVector VehicleLocation = Interpolated.Location;
	FRotator VehicleRotation = Interpolated.Rotation;

	switch (ReplayCameraAngle)
	{
	case EReplayCameraAngle::Chase:
		{
			// Use recorded camera position
			TargetCameraLocation = Interpolated.CameraLocation;
			TargetCameraRotation = Interpolated.CameraRotation;
			break;
		}

	case EReplayCameraAngle::Cinematic:
		{
			// Auto-orbit around the replay vehicle
			float OrbitRad = FMath::DegreesToRadians(PlaybackTime * CameraConstants::ReplayOrbitSpeed);
			FVector OrbitOffset;
			OrbitOffset.X = FMath::Cos(OrbitRad) * 800.0f;
			OrbitOffset.Y = FMath::Sin(OrbitRad) * 800.0f;
			OrbitOffset.Z = 300.0f;
			TargetCameraLocation = VehicleLocation + OrbitOffset;
			FVector LookTarget = VehicleLocation + FVector(0.0f, 0.0f, 100.0f);
			TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);
			break;
		}

	case EReplayCameraAngle::FreeOrbit:
		{
			// User-controlled orbit
			FRotator OrbitRotation(ReplayOrbitPitch, ReplayOrbitYaw, 0.0f);
			FVector OrbitOffset = OrbitRotation.Vector() * (-800.0f);
			OrbitOffset.Z = 300.0f;
			TargetCameraLocation = VehicleLocation + OrbitOffset;
			FVector LookTarget = VehicleLocation + FVector(0.0f, 0.0f, 100.0f);
			TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);
			break;
		}

	case EReplayCameraAngle::TrackSide:
		{
			// Fixed position on track side
			TargetCameraLocation = VehicleLocation + FVector(0.0f, 700.0f, 200.0f);
			FVector LookTarget = VehicleLocation + FVector(0.0f, 0.0f, 50.0f);
			TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);
			break;
		}

	case EReplayCameraAngle::TopDown:
		{
			// Top-down view
			TargetCameraLocation = VehicleLocation + FVector(0.0f, 0.0f, 1500.0f);
			TargetCameraRotation = FRotator(-90.0f, 0.0f, 0.0f);
			break;
		}

	case EReplayCameraAngle::Bumper:
		{
			// Bumper view during replay
			FVector ForwardDir = VehicleRotation.Vector();
			TargetCameraLocation = VehicleLocation + ForwardDir * 220.0f + FVector(0.0f, 0.0f, 35.0f);
			FVector LookTarget = VehicleLocation + ForwardDir * 2000.0f;
			TargetCameraRotation = UKismetMathLibrary::FindLookAtRotation(TargetCameraLocation, LookTarget);
			break;
		}
	}

	// Apply dynamic FOV from replay speed
	const FCameraConfig& Config = GetCameraConfig(ECameraMode::Replay);
	float MaxSpeed = GetVehicleMaxSpeed();
	TargetFOV = CalculateDynamicFOV(Config.FOV, Interpolated.Speed, MaxSpeed, Interpolated.Acceleration);
}

void UCameraSystem::UpdateShake(float DeltaTime)
{
	// Decay shake timers
	if (CollisionShakeTimer > 0.0f)
	{
		CollisionShakeTimer -= DeltaTime;
		if (CollisionShakeTimer <= 0.0f)
		{
			CollisionShakeTimer = 0.0f;
			CollisionShakeIntensity = 0.0f;
		}
	}

	if (DriftShakeTimer > 0.0f)
	{
		DriftShakeTimer -= DeltaTime;
		if (DriftShakeTimer <= 0.0f)
		{
			DriftShakeTimer = 0.0f;
			DriftShakeIntensity = 0.0f;
		}
	}

	if (GearShiftShakeTimer > 0.0f)
	{
		GearShiftShakeTimer -= DeltaTime;
		if (GearShiftShakeTimer <= 0.0f)
		{
			GearShiftShakeTimer = 0.0f;
		}
	}

	// Speed-based shake (continuous, subtle vibration at high speed)
	float VehicleSpeed = GetVehicleSpeed();
	float SpeedNorm = FMath::GetMappedRangeValueClamped(
		FVector2D(CameraConstants::FOVSpeedThreshold, CameraConstants::FOVMaxSpeed),
		FVector2D(0.0f, 1.0f), VehicleSpeed);
	SpeedShakeAccumulator += DeltaTime * CameraConstants::SpeedShakeBaseFrequency * (1.0f + SpeedNorm);

	// Update generic shake timer
	if (bIsShaking)
	{
		ShakeTimer -= DeltaTime;
		if (ShakeTimer <= 0.0f)
		{
			bIsShaking = false;
			ShakeTimer = 0.0f;
		}
	}
}

void UCameraSystem::UpdateTransition(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	TransitionAlpha += DeltaTime / TransitionDuration;
	if (TransitionAlpha >= 1.0f)
	{
		TransitionAlpha = 1.0f;
		bIsTransitioning = false;
	}
}

float UCameraSystem::CalculateDynamicFOV(float BaseFOV, float Speed, float MaxSpeed, float Acceleration) const
{
	// Only widen FOV above threshold speed
	if (Speed <= CameraConstants::FOVSpeedThreshold)
	{
		return BaseFOV;
	}

	float SpeedRatio = FMath::GetMappedRangeValueClamped(
		FVector2D(CameraConstants::FOVSpeedThreshold, MaxSpeed),
		FVector2D(0.0f, 1.0f), Speed);

	// Smooth ease-in curve for FOV widening
	float EasedRatio = FMath::InterpEaseIn(0.0f, 1.0f, SpeedRatio, 2.0f);

	float WidenedFOV = FMath::Lerp(
		CameraConstants::MinSpeedFOV,
		CameraConstants::MaxSpeedFOV,
		EasedRatio);

	// Apply per-mode speed multiplier
	const FCameraConfig& Config = GetCameraConfig(CurrentMode);
	WidenedFOV *= Config.SpeedFOVMultiplier;

	// Apply acceleration-based FOV boost
	if (FMath::Abs(Acceleration) > 10.0f) // Only when accelerating/braking significantly
	{
		float AccelNorm = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 50.0f), FVector2D(0.0f, 1.0f), FMath::Abs(Acceleration));
		WidenedFOV *= FMath::Lerp(1.0f, Config.AccelerationFOVMultiplier, AccelNorm);
	}

	// Clamp to reasonable range
	return FMath::Clamp(WidenedFOV, 60.0f, 130.0f);
}

void UCameraSystem::RecordReplayPoint()
{
	if (!VehicleActor || !CameraComponent)
	{
		return;
	}

	FReplayDataPoint Point;
	Point.Timestamp = RecordingTime;
	Point.Location = VehicleActor->GetActorLocation();
	Point.Rotation = VehicleActor->GetActorRotation();
	Point.CameraLocation = CameraComponent->GetComponentLocation();
	Point.CameraRotation = CameraComponent->GetComponentRotation();
	Point.Speed = GetVehicleSpeed();
	Point.Acceleration = CurrentAcceleration;

	// Get drift state if available
	if (ANIOVehicleBase* NIOVehicle = Cast<ANIOVehicleBase>(VehicleActor))
	{
		const FNIOVehicleState& State = NIOVehicle->GetVehicleState();
		Point.bIsDrifting = State.bIsDrifting;
		Point.SlipAngle = State.SlipAngle;
	}

	// Remove oldest data if at capacity
	int32 MaxPoints = static_cast<int32>(MaxReplayDuration / ReplayRecordInterval);
	if (ReplayData.Num() >= MaxPoints)
	{
		ReplayData.RemoveAt(0);
	}

	ReplayData.Add(Point);
}

FReplayDataPoint UCameraSystem::InterpolateReplayData(float Time) const
{
	FReplayDataPoint Result;

	if (ReplayData.Num() == 0)
	{
		return Result;
	}

	if (ReplayData.Num() == 1)
	{
		return ReplayData[0];
	}

	// Find the two data points surrounding the requested time
	int32 IndexA = 0;
	int32 IndexB = 1;

	for (int32 i = 0; i < ReplayData.Num() - 1; ++i)
	{
		if (ReplayData[i].Timestamp <= Time && ReplayData[i + 1].Timestamp >= Time)
		{
			IndexA = i;
			IndexB = i + 1;
			break;
		}

		// If we've passed all points, use last two
		if (i == ReplayData.Num() - 2)
		{
			IndexA = i;
			IndexB = i + 1;
		}
	}

	const FReplayDataPoint& PointA = ReplayData[IndexA];
	const FReplayDataPoint& PointB = ReplayData[IndexB];

	// Calculate interpolation alpha
	float SegmentDuration = PointB.Timestamp - PointA.Timestamp;
	float Alpha = (SegmentDuration > 0.0f)
		? FMath::Clamp((Time - PointA.Timestamp) / SegmentDuration, 0.0f, 1.0f)
		: 0.0f;

	// Smooth interpolation for camera
	float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	// Interpolate
	Result.Timestamp = Time;
	Result.Location = FMath::Lerp(PointA.Location, PointB.Location, SmoothAlpha);
	Result.Rotation = FMath::Lerp(PointA.Rotation, PointB.Rotation, SmoothAlpha);
	Result.CameraLocation = FMath::Lerp(PointA.CameraLocation, PointB.CameraLocation, SmoothAlpha);
	Result.CameraRotation = FMath::Lerp(PointA.CameraRotation, PointB.CameraRotation, SmoothAlpha);
	Result.Speed = FMath::Lerp(PointA.Speed, PointB.Speed, Alpha);
	Result.Acceleration = FMath::Lerp(PointA.Acceleration, PointB.Acceleration, Alpha);
	Result.bIsDrifting = Alpha > 0.5f ? PointB.bIsDrifting : PointA.bIsDrifting;
	Result.SlipAngle = FMath::Lerp(PointA.SlipAngle, PointB.SlipAngle, Alpha);

	return Result;
}

float UCameraSystem::GetVehicleSpeed() const
{
	if (!VehicleActor)
	{
		return 0.0f;
	}

	if (ANIOVehicleBase* NIOVehicle = Cast<ANIOVehicleBase>(VehicleActor))
	{
		return NIOVehicle->GetVehicleState().SpeedKmh;
	}

	// Fallback: calculate from velocity
	FVector Velocity = VehicleActor->GetVelocity();
	return Velocity.Size() * CameraConstants::CmPerSecToKmPerHour;
}

float UCameraSystem::GetVehicleMaxSpeed() const
{
	if (!VehicleActor)
	{
		return 250.0f;
	}

	if (ANIOVehicleBase* NIOVehicle = Cast<ANIOVehicleBase>(VehicleActor))
	{
		return NIOVehicle->GetPerformanceConfig().TopSpeedKph;
	}

	return 250.0f;
}

float UCameraSystem::GetVehicleAcceleration() const
{
	return CurrentAcceleration;
}

// ── Public API ────────────────────────────────────────────────────────────────

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

FString UCameraSystem::GetCameraModeName() const
{
	switch (CurrentMode)
	{
	case ECameraMode::Chase:     return TEXT("Chase");
	case ECameraMode::Hood:      return TEXT("Hood");
	case ECameraMode::Cockpit:   return TEXT("Cockpit");
	case ECameraMode::Bumper:    return TEXT("Bumper");
	case ECameraMode::Free:      return TEXT("Free");
	case ECameraMode::Cinematic: return TEXT("Cinematic");
	case ECameraMode::Replay:    return TEXT("Replay");
	default:                     return TEXT("Unknown");
	}
}

void UCameraSystem::SetCameraConfig(ECameraMode Mode, const FCameraConfig& Config)
{
	CameraConfigs.Add(Mode, Config);
}

const FCameraConfig& UCameraSystem::GetCameraConfig(ECameraMode Mode) const
{
	if (const FCameraConfig* Config = CameraConfigs.Find(Mode))
	{
		return *Config;
	}

	// Return a default config if mode not configured
	static FCameraConfig DefaultConfig;
	return DefaultConfig;
}

// ── Camera Shake ──────────────────────────────────────────────────────────────

void UCameraSystem::TriggerShake(const FCameraShakeParams& Params)
{
	bIsShaking = true;
	ShakeTimer = Params.Duration;
	CurrentShake = Params;
}

void UCameraSystem::TriggerCollisionShake(float ImpactForce)
{
	// Scale shake with impact force
	CollisionShakeIntensity = FMath::Clamp(ImpactForce * 0.01f, 0.0f, 2.0f);
	CollisionShakeTimer = 0.4f;

	FCameraShakeParams Params;
	Params.Frequency = 25.0f;
	Params.Amplitude = CollisionShakeIntensity * 3.0f;
	Params.Duration = 0.4f;
	Params.FalloffRate = CameraConstants::ShakeDecayRate;
	Params.AxisMultiplier = FVector(1.0f, 1.0f, 0.7f);
	TriggerShake(Params);

	UE_LOG(LogNomiCamera, Verbose, TEXT("Collision shake triggered: force=%.1f, intensity=%.2f"),
		ImpactForce, CollisionShakeIntensity);
}

void UCameraSystem::TriggerDriftShake(float DriftIntensity)
{
	DriftShakeIntensity = FMath::Clamp(DriftIntensity * 0.5f, 0.0f, 1.5f);
	DriftShakeTimer = 0.2f;

	FCameraShakeParams Params;
	Params.Frequency = 12.0f;
	Params.Amplitude = DriftShakeIntensity * 1.5f;
	Params.Duration = 0.2f;
	Params.FalloffRate = CameraConstants::ShakeDecayRate;
	Params.AxisMultiplier = FVector(0.8f, 1.0f, 0.3f);
	TriggerShake(Params);
}

void UCameraSystem::TriggerGearShiftShake()
{
	GearShiftShakeTimer = CameraConstants::GearShiftShakeDuration;

	FCameraShakeParams Params;
	Params.Frequency = 30.0f;
	Params.Amplitude = 0.3f;
	Params.Duration = CameraConstants::GearShiftShakeDuration;
	Params.FalloffRate = 8.0f;
	Params.AxisMultiplier = FVector(1.0f, 0.3f, 0.3f);
	TriggerShake(Params);
}

// ── Cinematic System ──────────────────────────────────────────────────────────

void UCameraSystem::SetCinematicShot(ECinematicShotType ShotType)
{
	if (CurrentCinematicShot != ShotType)
	{
		// Start transition to new shot
		CinematicTransitionAlpha = 0.0f;
		CinematicTransitionStart = TargetCameraLocation;
		CinematicTransitionStartRot = TargetCameraRotation;
		CurrentCinematicShot = ShotType;
		CinematicShotIndex = static_cast<int32>(ShotType);
		CinematicShotTimer = 0.0f;
	}
}

// ── Replay System ─────────────────────────────────────────────────────────────

void UCameraSystem::StartRecording()
{
	ReplayData.Empty();
	RecordingTime = 0.0f;
	ReplayRecordTimer = 0.0f;
	bIsRecording = true;
	bIsPlaying = false;

	UE_LOG(LogNomiCamera, Log, TEXT("Replay recording started"));
}

void UCameraSystem::StopRecording()
{
	bIsRecording = false;
	UE_LOG(LogNomiCamera, Log, TEXT("Replay recording stopped: %.1fs, %d data points"),
		RecordingTime, ReplayData.Num());
}

void UCameraSystem::StartPlayback()
{
	if (ReplayData.Num() < 2)
	{
		UE_LOG(LogNomiCamera, Warning, TEXT("Cannot start playback: insufficient replay data (%d points)"),
			ReplayData.Num());
		return;
	}

	PlaybackTime = 0.0f;
	bIsPlaying = true;
	bIsRecording = false;
	bIsReplayPaused = false;

	SetCameraMode(ECameraMode::Replay);

	UE_LOG(LogNomiCamera, Log, TEXT("Replay playback started: %.1fs duration, %d data points"),
		GetReplayDuration(), ReplayData.Num());
}

void UCameraSystem::StopPlayback()
{
	bIsPlaying = false;
	bIsReplayPaused = false;
	PlaybackTime = 0.0f;

	// Return to chase mode
	SetCameraMode(ECameraMode::Chase);

	UE_LOG(LogNomiCamera, Log, TEXT("Replay playback stopped"));
}

float UCameraSystem::GetReplayDuration() const
{
	if (ReplayData.Num() == 0)
	{
		return 0.0f;
	}

	return ReplayData.Last().Timestamp - ReplayData[0].Timestamp;
}

void UCameraSystem::SetPlaybackTime(float Time)
{
	PlaybackTime = FMath::Clamp(Time, 0.0f, GetReplayDuration());
}

void UCameraSystem::SetPlaybackSpeed(float Speed)
{
	// Clamp to valid speed options
	static const TArray<float> ValidSpeeds = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f };
	float ClosestSpeed = 1.0f;
	float MinDiff = TNumericLimits<float>::Max();

	for (float ValidSpeed : ValidSpeeds)
	{
		float Diff = FMath::Abs(Speed - ValidSpeed);
		if (Diff < MinDiff)
		{
			MinDiff = Diff;
			ClosestSpeed = ValidSpeed;
		}
	}

	PlaybackSpeed = ClosestSpeed;
	UE_LOG(LogNomiCamera, Log, TEXT("Replay playback speed set to: %.2fx"), PlaybackSpeed);
}

void UCameraSystem::SetReplayCameraAngle(EReplayCameraAngle Angle)
{
	ReplayCameraAngle = Angle;
	UE_LOG(LogNomiCamera, Log, TEXT("Replay camera angle changed to: %d"), static_cast<int32>(Angle));
}

void UCameraSystem::ToggleReplayPause()
{
	bIsReplayPaused = !bIsReplayPaused;
	UE_LOG(LogNomiCamera, Log, TEXT("Replay %s"), bIsReplayPaused ? TEXT("paused") : TEXT("resumed"));
}

float UCameraSystem::GetReplayProgress() const
{
	float Duration = GetReplayDuration();
	if (Duration <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(PlaybackTime / Duration, 0.0f, 1.0f);
}

void UCameraSystem::SetLookBack(bool bLookBack)
{
	bLookingBack = bLookBack;
}
