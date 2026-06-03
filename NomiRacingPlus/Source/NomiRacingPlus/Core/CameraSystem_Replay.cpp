// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CameraSystem.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

// ── Replay Camera ────────────────────────────────────────────────────────────

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
			float OrbitRad = FMath::DegreesToRadians(PlaybackTime * 30.0f);
			TargetCameraLocation = VehicleLocation + FVector(
				FMath::Cos(OrbitRad) * 800.0f,
				FMath::Sin(OrbitRad) * 800.0f,
				300.0f);
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

// ── Recording ────────────────────────────────────────────────────────────────

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

	// Circular buffer: avoid O(n) RemoveAt(0) on large arrays
	int32 MaxPoints = static_cast<int32>(MaxReplayDuration / ReplayRecordInterval);
	if (ReplayData.Num() < MaxPoints)
	{
		ReplayData.Add(Point);
	}
	else
	{
		// Overwrite oldest entry and advance head index
		ReplayData[ReplayHeadIndex] = Point;
		ReplayHeadIndex = (ReplayHeadIndex + 1) % MaxPoints;
	}
}

// ── Playback / Interpolation ─────────────────────────────────────────────────

FReplayDataPoint UCameraSystem::InterpolateReplayData(float Time) const
{
	FReplayDataPoint Result;

	int32 Count = ReplayData.Num();
	if (Count == 0)
	{
		return Result;
	}

	if (Count == 1)
	{
		return ReplayData[GetReplayIndex(0)];
	}

	// Find the two data points surrounding the requested time
	// Uses GetReplayIndex to map linear order to circular buffer layout
	int32 IndexA = 0;
	int32 IndexB = 1;

	for (int32 i = 0; i < Count - 1; ++i)
	{
		int32 IdxA = GetReplayIndex(i);
		int32 IdxB = GetReplayIndex(i + 1);

		if (ReplayData[IdxA].Timestamp <= Time && ReplayData[IdxB].Timestamp >= Time)
		{
			IndexA = i;
			IndexB = i + 1;
			break;
		}

		// If we've passed all points, use last two
		if (i == Count - 2)
		{
			IndexA = i;
			IndexB = i + 1;
		}
	}

	const FReplayDataPoint& PointA = ReplayData[GetReplayIndex(IndexA)];
	const FReplayDataPoint& PointB = ReplayData[GetReplayIndex(IndexB)];

	// Calculate interpolation alpha
	float SegmentDuration = PointB.Timestamp - PointA.Timestamp;
	float Alpha = (SegmentDuration > 0.0f)
		? FMath::Clamp((Time - PointA.Timestamp) / SegmentDuration, 0.0f, 1.0f)
		: 0.0f;

	// Smooth interpolation for camera
	float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	// Interpolate continuous values
	Result.Timestamp = Time;
	Result.Location = FMath::Lerp(PointA.Location, PointB.Location, SmoothAlpha);
	Result.Rotation = FMath::Lerp(PointA.Rotation, PointB.Rotation, SmoothAlpha);
	Result.CameraLocation = FMath::Lerp(PointA.CameraLocation, PointB.CameraLocation, SmoothAlpha);
	Result.CameraRotation = FMath::Lerp(PointA.CameraRotation, PointB.CameraRotation, SmoothAlpha);
	Result.Speed = FMath::Lerp(PointA.Speed, PointB.Speed, Alpha);
	Result.Acceleration = FMath::Lerp(PointA.Acceleration, PointB.Acceleration, Alpha);

	// Derive drift state from slip angle (avoid interpolating raw boolean)
	Result.SlipAngle = FMath::Lerp(PointA.SlipAngle, PointB.SlipAngle, Alpha);
	Result.bIsDrifting = Result.SlipAngle > 5.0f;

	return Result;
}

// ── Public Replay API ────────────────────────────────────────────────────────

void UCameraSystem::StartRecording()
{
	ReplayData.Empty();
	ReplayHeadIndex = 0;
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
