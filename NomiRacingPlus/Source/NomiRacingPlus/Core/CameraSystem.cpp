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
		USceneComponent* RootComp = VehicleActor->GetRootComponent();
		if (RootComp)
		{
			CameraComponent = NewObject<UCameraComponent>(VehicleActor, TEXT("VehicleCamera"));
			CameraComponent->SetupAttachment(RootComp);
			CameraComponent->RegisterComponent();
			UE_LOG(LogNomiCamera, Log, TEXT("CameraComponent created dynamically"));
		}
		else
		{
			UE_LOG(LogNomiCamera, Warning, TEXT("Cannot create CameraComponent: VehicleActor has no RootComponent"));
		}
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
	if (FMath::Abs(Acceleration) > 10.0f)
	{
		float AccelNorm = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 50.0f), FVector2D(0.0f, 1.0f), FMath::Abs(Acceleration));
		WidenedFOV *= FMath::Lerp(1.0f, Config.AccelerationFOVMultiplier, AccelNorm);
	}

	return FMath::Clamp(WidenedFOV,
		CameraConstants::MinSpeedFOV,
		CameraConstants::MaxSpeedFOV * 1.3f);
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

	static FCameraConfig DefaultConfig;
	return DefaultConfig;
}

void UCameraSystem::SetLookBack(bool bLookBack)
{
	bLookingBack = bLookBack;
}
