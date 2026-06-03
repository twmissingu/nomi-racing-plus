// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CameraSystem.h"
#include "NomiRacingPlus.h"

namespace CameraShakeConstants
{
	// Speed thresholds for continuous shake intensity (km/h)
	constexpr float SpeedMin = 50.0f;
	constexpr float SpeedMax = 300.0f;
}

// ── Shake Update (called every tick) ─────────────────────────────────────────

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
		FVector2D(CameraShakeConstants::SpeedMin, CameraShakeConstants::SpeedMax),
		FVector2D(0.0f, 1.0f), VehicleSpeed);
	SpeedShakeAccumulator += DeltaTime * 2.0f * (1.0f + SpeedNorm);

	// Update generic shake timer with falloff
	if (bIsShaking)
	{
		ShakeTimer -= DeltaTime;
		if (ShakeTimer <= 0.0f)
		{
			bIsShaking = false;
			ShakeTimer = 0.0f;
			CurrentShake.Amplitude = 0.0f;
		}
		else if (CurrentShake.FalloffRate > 0.0f)
		{
			// Apply exponential amplitude decay based on falloff rate
			float DecayFactor = FMath::Exp(-CurrentShake.FalloffRate * DeltaTime);
			CurrentShake.Amplitude *= DecayFactor;
		}
	}
}

// ── Shake Triggers ───────────────────────────────────────────────────────────

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
	Params.FalloffRate = 3.0f;
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
	Params.FalloffRate = 3.0f;
	Params.AxisMultiplier = FVector(0.8f, 1.0f, 0.3f);
	TriggerShake(Params);
}

void UCameraSystem::TriggerGearShiftShake()
{
	GearShiftShakeTimer = 0.15f;

	FCameraShakeParams Params;
	Params.Frequency = 30.0f;
	Params.Amplitude = 0.3f;
	Params.Duration = 0.15f;
	Params.FalloffRate = 8.0f;
	Params.AxisMultiplier = FVector(1.0f, 0.3f, 0.3f);
	TriggerShake(Params);
}

// ── Cinematic Shot Control ───────────────────────────────────────────────────

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
