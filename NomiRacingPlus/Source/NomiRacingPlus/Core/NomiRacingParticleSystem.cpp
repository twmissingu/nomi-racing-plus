// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiRacingParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"

// Default spawn counts (before quality scaling)
static constexpr float DefaultTireSmokeSpawnCount = 20.0f;
static constexpr float DefaultCollisionSparksSpawnCount = 30.0f;
static constexpr float DefaultDriftSmokeSpawnCount = 15.0f;
static constexpr float DefaultExhaustSpawnCount = 10.0f;
static constexpr float DefaultSpeedTrailSpawnCount = 8.0f;
static constexpr float DefaultWaterSpraySpawnCount = 25.0f;

// Thresholds for particle triggers
static constexpr float ExhaustThrottleThreshold = 0.7f;
static constexpr float ExhaustSpeedThresholdKmh = 60.0f;
static constexpr float SpeedTrailThresholdKmh = 120.0f;
static constexpr float WaterSprayIntensityThreshold = 0.05f;

UNomiRacingParticleSystem::UNomiRacingParticleSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	CurrentQuality = EParticleQuality::High;
}

void UNomiRacingParticleSystem::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogNomiPerf, Warning, TEXT("NomiRacingParticleSystem: No owning actor, cannot initialise"));
		return;
	}

	CachedRootComponent = Owner->GetRootComponent();

	// Create managed Niagara components for systems that need persistent state
	TireSmokeSystem = CreateNiagaraComponent(TireSmokeSystemAsset, TEXT("TireSmokeSystem"));
	CollisionSparksSystem = CreateNiagaraComponent(CollisionSparksSystemAsset, TEXT("CollisionSparksSystem"));
	DriftSmokeSystem = CreateNiagaraComponent(DriftSmokeSystemAsset, TEXT("DriftSmokeSystem"));
	SpeedTrailSystem = CreateNiagaraComponent(SpeedTrailSystemAsset, TEXT("SpeedTrailSystem"));

	// Persistent effects start inactive; toggled via their update functions
	if (DriftSmokeSystem)
	{
		DriftSmokeSystem->Deactivate();
	}
	if (SpeedTrailSystem)
	{
		SpeedTrailSystem->Deactivate();
	}

	UE_LOG(LogNomiPerf, Log, TEXT("NomiRacingParticleSystem: Initialised (Quality=%d)"), static_cast<int32>(CurrentQuality));
}

void UNomiRacingParticleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ---------------------------------------------------------------------------
// SpawnTireSmoke
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::SpawnTireSmoke(FVector Location, float Intensity)
{
	if (!TireSmokeSystemAsset)
	{
		return;
	}

	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (ClampedIntensity < 0.01f)
	{
		return;
	}

	// Spawn a one-shot burst at the tire contact point
	UNiagaraComponent* SpawnedComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		TireSmokeSystemAsset,
		Location,
		FRotator::ZeroRotator,
		FVector::OneVector,
		/*bAutoDestroy=*/ true,
		/*bAutoActivate=*/ true,
		ENCPoolMethod::AutoRelease,
		/*bPreCullCheck=*/ true
	);

	if (SpawnedComp)
	{
		const float SpawnMultiplier = GetQualitySpawnMultiplier();
		const float SizeMultiplier = GetQualitySizeMultiplier();

		SpawnedComp->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultTireSmokeSpawnCount * SpawnMultiplier * ClampedIntensity);
		SpawnedComp->SetVariableFloat(FName(TEXT("ParticleSize")), SizeMultiplier);
		SpawnedComp->SetVariableFloat(FName(TEXT("Intensity")), ClampedIntensity);

		UE_LOG(LogNomiPerf, Verbose, TEXT("TireSmoke spawned at %s intensity=%.2f"), *Location.ToString(), ClampedIntensity);
	}
}

// ---------------------------------------------------------------------------
// SpawnCollisionSparks
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::SpawnCollisionSparks(FVector Location, FVector Normal, float Intensity)
{
	if (!CollisionSparksSystemAsset)
	{
		return;
	}

	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (ClampedIntensity < 0.01f)
	{
		return;
	}

	// Orient sparks along the impact normal
	const FRotator Rotation = Normal.Rotation();

	UNiagaraComponent* SpawnedComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CollisionSparksSystemAsset,
		Location,
		Rotation,
		FVector::OneVector,
		/*bAutoDestroy=*/ true,
		/*bAutoActivate=*/ true,
		ENCPoolMethod::AutoRelease,
		/*bPreCullCheck=*/ true
	);

	if (SpawnedComp)
	{
		const float SpawnMultiplier = GetQualitySpawnMultiplier();

		SpawnedComp->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultCollisionSparksSpawnCount * SpawnMultiplier * ClampedIntensity);
		SpawnedComp->SetVariableVec3(FName(TEXT("ImpactNormal")), Normal);
		SpawnedComp->SetVariableFloat(FName(TEXT("Intensity")), ClampedIntensity);

		UE_LOG(LogNomiPerf, Verbose, TEXT("CollisionSparks spawned at %s normal=%s intensity=%.2f"),
			*Location.ToString(), *Normal.ToString(), ClampedIntensity);
	}
}

// ---------------------------------------------------------------------------
// UpdateDriftSmoke
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::UpdateDriftSmoke(bool bActive, float Intensity)
{
	if (!DriftSmokeSystem)
	{
		return;
	}

	if (bActive)
	{
		const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
		const float SpawnMultiplier = GetQualitySpawnMultiplier();
		const float SizeMultiplier = GetQualitySizeMultiplier();

		DriftSmokeSystem->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultDriftSmokeSpawnCount * SpawnMultiplier * ClampedIntensity);
		DriftSmokeSystem->SetVariableFloat(FName(TEXT("ParticleSize")), SizeMultiplier);
		DriftSmokeSystem->SetVariableFloat(FName(TEXT("Intensity")), ClampedIntensity);

		if (!DriftSmokeSystem->IsActive())
		{
			DriftSmokeSystem->Activate(/*bReset=*/ true);
		}
	}
	else
	{
		if (DriftSmokeSystem->IsActive())
		{
			DriftSmokeSystem->Deactivate();
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnExhaustEffect
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::SpawnExhaustEffect(FVector Location, float Throttle, float SpeedKmh)
{
	if (!ExhaustEffectSystemAsset)
	{
		return;
	}

	// Only trigger at high throttle and sufficient speed
	if (Throttle < ExhaustThrottleThreshold || SpeedKmh < ExhaustSpeedThresholdKmh)
	{
		return;
	}

	// Intensity scales with throttle beyond threshold
	const float Intensity = FMath::GetMappedRangeValueClamped(
		FVector2D(ExhaustThrottleThreshold, 1.0f),
		FVector2D(0.1f, 1.0f),
		Throttle
	);

	UNiagaraComponent* SpawnedComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExhaustEffectSystemAsset,
		Location,
		FRotator::ZeroRotator,
		FVector::OneVector,
		/*bAutoDestroy=*/ true,
		/*bAutoActivate=*/ true,
		ENCPoolMethod::AutoRelease,
		/*bPreCullCheck=*/ true
	);

	if (SpawnedComp)
	{
		const float SpawnMultiplier = GetQualitySpawnMultiplier();
		SpawnedComp->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultExhaustSpawnCount * SpawnMultiplier * Intensity);
		SpawnedComp->SetVariableFloat(FName(TEXT("Intensity")), Intensity);
		SpawnedComp->SetVariableFloat(FName(TEXT("SpeedKmh")), SpeedKmh);

		UE_LOG(LogNomiPerf, Verbose, TEXT("ExhaustEffect spawned throttle=%.2f speed=%.0f"), Throttle, SpeedKmh);
	}
}

// ---------------------------------------------------------------------------
// UpdateSpeedTrail
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::UpdateSpeedTrail(bool bActive, float SpeedKmh)
{
	if (!SpeedTrailSystem)
	{
		return;
	}

	if (bActive && SpeedKmh >= SpeedTrailThresholdKmh)
	{
		const float Intensity = FMath::GetMappedRangeValueClamped(
			FVector2D(SpeedTrailThresholdKmh, 350.0f),
			FVector2D(0.2f, 1.0f),
			SpeedKmh
		);
		const float SpawnMultiplier = GetQualitySpawnMultiplier();
		const float SizeMultiplier = GetQualitySizeMultiplier();

		SpeedTrailSystem->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultSpeedTrailSpawnCount * SpawnMultiplier * Intensity);
		SpeedTrailSystem->SetVariableFloat(FName(TEXT("ParticleSize")), SizeMultiplier);
		SpeedTrailSystem->SetVariableFloat(FName(TEXT("Intensity")), Intensity);
		SpeedTrailSystem->SetVariableFloat(FName(TEXT("SpeedKmh")), SpeedKmh);

		if (!SpeedTrailSystem->IsActive())
		{
			SpeedTrailSystem->Activate(/*bReset=*/ true);
		}
	}
	else
	{
		if (SpeedTrailSystem->IsActive())
		{
			SpeedTrailSystem->Deactivate();
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnWaterSpray
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::SpawnWaterSpray(FVector Location, float Intensity)
{
	if (!WaterSpraySystemAsset)
	{
		return;
	}

	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (ClampedIntensity < WaterSprayIntensityThreshold)
	{
		return;
	}

	UNiagaraComponent* SpawnedComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		WaterSpraySystemAsset,
		Location,
		FRotator::ZeroRotator,
		FVector::OneVector,
		/*bAutoDestroy=*/ true,
		/*bAutoActivate=*/ true,
		ENCPoolMethod::AutoRelease,
		/*bPreCullCheck=*/ true
	);

	if (SpawnedComp)
	{
		const float SpawnMultiplier = GetQualitySpawnMultiplier();
		SpawnedComp->SetVariableFloat(FName(TEXT("SpawnCount")), DefaultWaterSpraySpawnCount * SpawnMultiplier * ClampedIntensity);
		SpawnedComp->SetVariableFloat(FName(TEXT("Intensity")), ClampedIntensity);

		UE_LOG(LogNomiPerf, Verbose, TEXT("WaterSpray spawned at %s intensity=%.2f"), *Location.ToString(), ClampedIntensity);
	}
}

// ---------------------------------------------------------------------------
// SetParticleQuality
// ---------------------------------------------------------------------------
void UNomiRacingParticleSystem::SetParticleQuality(int32 Level)
{
	switch (Level)
	{
	case 0:
		CurrentQuality = EParticleQuality::Low;
		break;
	case 1:
		CurrentQuality = EParticleQuality::Medium;
		break;
	case 2:
	default:
		CurrentQuality = EParticleQuality::High;
		break;
	}

	UE_LOG(LogNomiPerf, Log, TEXT("NomiRacingParticleSystem: Quality set to %d"), static_cast<int32>(CurrentQuality));
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

UNiagaraComponent* UNomiRacingParticleSystem::CreateNiagaraComponent(UNiagaraSystem* System, const FName& ComponentName)
{
	if (!System)
	{
		UE_LOG(LogNomiPerf, Warning, TEXT("NomiRacingParticleSystem: No Niagara asset for %s, skipping"), *ComponentName.ToString());
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(Owner, ComponentName);
	if (!NiagaraComp)
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Particles"), FString::Printf(TEXT("NomiRacingParticleSystem: Failed to create Niagara component %s"), *ComponentName.ToString()));
		return nullptr;
	}

	NiagaraComp->SetAsset(System);
	NiagaraComp->SetAutoActivate(false);
	NiagaraComp->SetupAttachment(CachedRootComponent.Get());
	NiagaraComp->RegisterComponent();

	UE_LOG(LogNomiPerf, Log, TEXT("NomiRacingParticleSystem: Created Niagara component %s"), *ComponentName.ToString());
	return NiagaraComp;
}

void UNomiRacingParticleSystem::ApplyQualityToComponent(UNiagaraComponent* Component, float BaseSpawnCount)
{
	if (!Component)
	{
		return;
	}

	const float SpawnMultiplier = GetQualitySpawnMultiplier();
	const float SizeMultiplier = GetQualitySizeMultiplier();

	Component->SetVariableFloat(FName(TEXT("SpawnCount")), BaseSpawnCount * SpawnMultiplier);
	Component->SetVariableFloat(FName(TEXT("ParticleSize")), SizeMultiplier);
}

float UNomiRacingParticleSystem::GetQualitySpawnMultiplier() const
{
	switch (CurrentQuality)
	{
	case EParticleQuality::Low:    return 0.4f;
	case EParticleQuality::Medium: return 0.7f;
	case EParticleQuality::High:   return 1.0f;
	default:                       return 1.0f;
	}
}

float UNomiRacingParticleSystem::GetQualitySizeMultiplier() const
{
	switch (CurrentQuality)
	{
	case EParticleQuality::Low:    return 0.7f;
	case EParticleQuality::Medium: return 0.85f;
	case EParticleQuality::High:   return 1.0f;
	default:                       return 1.0f;
	}
}
