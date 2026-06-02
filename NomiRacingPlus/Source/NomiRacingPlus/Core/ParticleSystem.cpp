// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Core/ParticleSystem.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "NomiRacingPlus.h"
#include "FXSystemComponent.h"

DEFINE_LOG_CATEGORY(LogNomiParticles);

// ============================================================================
// Constants
// ============================================================================
namespace ParticleConstants
{
	// Number of wheels on a standard vehicle
	constexpr int32 DefaultNumWheels = 4;

	// Minimum impact force to spawn collision sparks
	constexpr float MinCollisionForce = 200.0f;

	// Maximum impact force for particle scaling
	constexpr float MaxCollisionForce = 5000.0f;

	// Collision sparks burst duration (seconds)
	constexpr float CollisionSparkDuration = 0.5f;

	// Minimum distance from last decal to spawn a new one (cm)
	constexpr float MinDecalDistance = 20.0f;

	// Decal lifetime before fade starts (seconds)
	constexpr float DefaultDecalLifetime = 8.0f;

	// Decal fade out duration (seconds)
	constexpr float DefaultDecalFadeOut = 2.0f;

	// Tire smoke emission scale factor (emissions per unit of slip above threshold)
	constexpr float TireSmokeEmissionScale = 800.0f;

	// Drift smoke emission scale factor
	constexpr float DriftSmokeEmissionScale = 600.0f;

	// Dust emission scale factor
	constexpr float DustEmissionScale = 400.0f;

	// Rain ground splash emission scale factor
	constexpr float RainSplashEmissionScale = 200.0f;

	// Speed factor for tire smoke scaling (normalized to 200 km/h)
	constexpr float SpeedNormalization = 200.0f;

	// Minimum alpha for decals to be visible
	constexpr float MinDecalAlpha = 0.05f;
}

// ============================================================================
// Constructor
// ============================================================================
UParticleSystemManager::UParticleSystemManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

// ============================================================================
// Lifecycle
// ============================================================================
void UParticleSystemManager::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();

	// Cache vehicle movement component
	if (OwnerActor)
	{
		VehicleMovement = OwnerActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	}

	// Initialize configs and states
	InitializeDefaultConfigs();
	InitializeDefaultSurfaceParams();
	InitializeWheelEffects();

	UE_LOG(LogNomiParticles, Log, TEXT("ParticleSystemManager initialized with %d wheels"), GetNumWheels());
}

void UParticleSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Process pending collision sparks (spawned after physics step)
	for (const FPendingCollisionSparks& Pending : PendingCollisionSparks)
	{
		const FParticleEffectConfig& Config = GetConfigOrDefault(EParticleEffect::CollisionSparks);

		UFXSystemComponent* Sparks = SpawnOneShotEffect(EParticleEffect::CollisionSparks, Pending.Location, Pending.Normal.Rotation());
		if (Sparks)
		{
			// Scale particle count by impact force
			const float ForceAlpha = FMath::Clamp(Pending.ImpactForce / ParticleConstants::MaxCollisionForce, 0.1f, 1.0f);
			Sparks->SetWorldScale3D(Config.Scale * FMath::Lerp(0.5f, 2.0f, ForceAlpha));
			SetEffectFloatParam(Sparks, TEXT("ImpactForce"), ForceAlpha);
		}
	}
	PendingCollisionSparks.Empty();

	// Clean up finished effects
	CleanupEffects();
	CleanupDecals();

	// Update per-wheel effect states
	for (int32 i = 0; i < WheelEffects.Num(); i++)
	{
		WheelEffects[i].TimeSinceLastDecal += DeltaTime;
	}
}

// ============================================================================
// Effect Control
// ============================================================================
UFXSystemComponent* UParticleSystemManager::SpawnEffect(EParticleEffect EffectType, const FVector& Location, const FRotator& Rotation)
{
	return SpawnOneShotEffect(EffectType, Location, Rotation);
}

void UParticleSystemManager::StopEffect(UFXSystemComponent* Effect)
{
	DeactivateEffect(Effect);
}

void UParticleSystemManager::SpawnDecal(const FDecalEffectConfig& Config, const FVector& Location, const FRotator& Rotation)
{
	if (!OwnerActor || !Config.DecalMaterial || ActiveDecals.Num() >= MaxActiveDecals)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn decal component
	UDecalComponent* Decal = NewObject<UDecalComponent>(OwnerActor);
	Decal->SetDecalMaterial(Config.DecalMaterial);
	Decal->SetWorldLocation(Location);
	Decal->SetWorldRotation(Rotation);
	Decal->DecalSize = Config.Size;
	Decal->SetFadeScreenSize(0.001f);
	Decal->RegisterComponent();

	// Add to world
	World->AddInstanceComponent(Decal);

	// Track for cleanup
	ActiveDecals.Add(Decal);

	// Schedule fade-out
	Decal->SetFadeIn(0.0f, 0.0f);
	Decal->SetFadeOut(Config.Lifetime, Config.FadeOutTime);
}

// ============================================================================
// Vehicle Effects
// ============================================================================
void UParticleSystemManager::UpdateTireEffects(float DeltaTime, const TArray<FTireState>& TireStates, float VehicleSpeedKmh)
{
	if (!OwnerActor)
	{
		return;
	}

	const int32 NumWheels = FMath::Min(TireStates.Num(), WheelEffects.Num());

	for (int32 i = 0; i < NumWheels; i++)
	{
		const FTireState& State = TireStates[i];
		FWheelEffectState& WheelFX = WheelEffects[i];

		// Update grounding and surface state
		WheelFX.bIsGrounded = State.bIsGrounded;
		WheelFX.SurfaceType = State.SurfaceType;

		if (!State.bIsGrounded)
		{
			// Wheel in air: stop all ground effects
			DeactivateEffect(WheelFX.TireSmokeComponent);
			WheelFX.TireSmokeComponent = nullptr;
			DeactivateEffect(WheelFX.DriftSmokeComponent);
			WheelFX.DriftSmokeComponent = nullptr;
			DeactivateEffect(WheelFX.DustComponent);
			WheelFX.DustComponent = nullptr;
			continue;
		}

		// Update each effect type per wheel
		UpdateWheelTireSmoke(i, State, VehicleSpeedKmh);
		UpdateWheelDriftSmoke(i, State, VehicleSpeedKmh);
		UpdateWheelTireMarks(i, State, DeltaTime);
		UpdateWheelDust(i, State, VehicleSpeedKmh);
	}
}

void UParticleSystemManager::TriggerCollisionSparks(const FVector& Location, const FVector& Normal, float ImpactForce)
{
	if (ImpactForce < ParticleConstants::MinCollisionForce)
	{
		return;
	}

	// Queue for next tick to avoid mid-physics issues
	FPendingCollisionSparks Pending;
	Pending.Location = Location;
	Pending.Normal = Normal;
	Pending.ImpactForce = ImpactForce;
	PendingCollisionSparks.Add(Pending);
}

// ============================================================================
// Environment Effects
// ============================================================================
void UParticleSystemManager::UpdateRainEffect(float DeltaTime, float TargetIntensity)
{
	RainState.TargetIntensity = FMath::Clamp(TargetIntensity, 0.0f, 1.0f);
	RainState.bIsRaining = RainState.TargetIntensity > 0.01f;

	// Interpolate current intensity toward target
	RainState.Intensity = FMath::FInterpTo(RainState.Intensity, RainState.TargetIntensity, DeltaTime, RainInterpSpeed);

	if (RainState.Intensity < 0.01f)
	{
		// Rain stopped: deactivate effects
		DeactivateEffect(RainState.RainAmbientComponent);
		RainState.RainAmbientComponent = nullptr;
		DeactivateEffect(RainState.RainSplashComponent);
		RainState.RainSplashComponent = nullptr;
		return;
	}

	// Spawn or update ambient rain (screen-space rain drops)
	if (!RainState.RainAmbientComponent)
	{
		const FParticleEffectConfig& Config = GetConfigOrDefault(EParticleEffect::RainSplash);
		if (Config.NiagaraSystem || Config.ParticleSystem)
		{
			// Attach to camera/owner for screen-space effect
			RainState.RainAmbientComponent = GetOrCreateLoopingEffect(EParticleEffect::RainSplash, OwnerActor->GetRootComponent());
			if (RainState.RainAmbientComponent)
			{
				RainState.RainAmbientComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));
			}
		}
	}

	// Update ambient rain intensity
	if (RainState.RainAmbientComponent)
	{
		SetEffectFloatParam(RainState.RainAmbientComponent, TEXT("Intensity"), RainState.Intensity);
		SetEffectFloatParam(RainState.RainAmbientComponent, TEXT("EmissionRate"), RainState.Intensity * 500.0f);
	}

	// Spawn or update ground splashes
	if (!RainState.RainSplashComponent && OwnerActor)
	{
		const FParticleEffectConfig& Config = GetConfigOrDefault(EParticleEffect::RainSplash);
		if (Config.NiagaraSystem || Config.ParticleSystem)
		{
			RainState.RainSplashComponent = GetOrCreateLoopingEffect(EParticleEffect::RainSplash, OwnerActor->GetRootComponent());
		}
	}

	if (RainState.RainSplashComponent)
	{
		SetEffectFloatParam(RainState.RainSplashComponent, TEXT("Intensity"), RainState.Intensity);
		SetEffectFloatParam(RainState.RainSplashComponent, TEXT("EmissionRate"), RainState.Intensity * ParticleConstants::RainSplashEmissionScale);
	}
}

void UParticleSystemManager::UpdateDustEffect(float Speed, bool bOnDirt)
{
	// Legacy single-call method; prefer per-wheel UpdateWheelDust
	if (!OwnerActor)
	{
		return;
	}

	if (!bOnDirt || Speed < DustSpeedThreshold)
	{
		return;
	}

	const float SpeedAlpha = FMath::Clamp(Speed / ParticleConstants::SpeedNormalization, 0.0f, 1.0f);
	const float EmissionRate = SpeedAlpha * ParticleConstants::DustEmissionScale;

	// Spawn a one-shot dust cloud at vehicle location
	const FVector Location = OwnerActor->GetActorLocation();
	UFXSystemComponent* Dust = SpawnOneShotEffect(EParticleEffect::DustCloud, Location, FRotator::ZeroRotator);
	if (Dust)
	{
		SetEffectFloatParam(Dust, TEXT("EmissionRate"), EmissionRate);
	}
}

// ============================================================================
// Configuration
// ============================================================================
void UParticleSystemManager::SetEffectConfig(EParticleEffect EffectType, const FParticleEffectConfig& Config)
{
	EffectConfigs.Add(EffectType, Config);
}

const FParticleEffectConfig& UParticleSystemManager::GetEffectConfig(EParticleEffect EffectType) const
{
	return GetConfigOrDefault(EffectType);
}

void UParticleSystemManager::SetSurfaceParams(ETireSurfaceType Surface, const FSurfaceParticleParams& Params)
{
	SurfaceParams.Add(Surface, Params);
}

void UParticleSystemManager::SetRainEnabled(bool bEnabled)
{
	if (!bEnabled)
	{
		RainState.TargetIntensity = 0.0f;
		RainState.bIsRaining = false;
	}
}

// ============================================================================
// Per-Wheel Updates
// ============================================================================
void UParticleSystemManager::UpdateWheelTireSmoke(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh)
{
	if (!WheelEffects.IsValidIndex(WheelIndex))
	{
		return;
	}

	FWheelEffectState& WheelFX = WheelEffects[WheelIndex];
	const float AbsSlipRatio = FMath::Abs(TireState.SlipRatio);
	const FSurfaceParticleParams& SurfParams = GetSurfaceParamsOrDefault(TireState.SurfaceType);

	// Determine if smoke should be active
	const bool bShouldSmoke = AbsSlipRatio > TireSmokeSlipThreshold && VehicleSpeedKmh > 5.0f;

	if (bShouldSmoke)
	{
		// Calculate intensity based on slip ratio above threshold
		const float SlipIntensity = FMath::Clamp(
			(AbsSlipRatio - TireSmokeSlipThreshold) / (1.0f - TireSmokeSlipThreshold), 0.0f, 1.0f);

		// Speed factor: more visible smoke at lower speeds (burnouts)
		const float SpeedFactor = 1.0f - FMath::Clamp(VehicleSpeedKmh / ParticleConstants::SpeedNormalization, 0.0f, 0.7f);

		// Final emission rate
		const float EmissionRate = SlipIntensity * ParticleConstants::TireSmokeEmissionScale
			* SurfParams.EmissionMultiplier * (0.5f + SpeedFactor * 0.5f);

		if (!WheelFX.TireSmokeComponent)
		{
			// Create new looping smoke effect attached to wheel
			WheelFX.TireSmokeComponent = GetOrCreateLoopingEffect(EParticleEffect::TireSmoke, OwnerActor->GetRootComponent());
		}

		if (WheelFX.TireSmokeComponent)
		{
			const FVector WheelLoc = GetWheelLocation(WheelIndex);
			WheelFX.TireSmokeComponent->SetWorldLocation(WheelLoc);
			WheelFX.TireSmokeComponent->SetWorldScale3D(FVector(SurfParams.ScaleMultiplier));
			SetEffectFloatParam(WheelFX.TireSmokeComponent, TEXT("EmissionRate"), EmissionRate);
			SetEffectVectorParam(WheelFX.TireSmokeComponent, TEXT("Color"),
				FVector(SurfParams.Color.R, SurfParams.Color.G, SurfParams.Color.B));

			if (!IsEffectActive(WheelFX.TireSmokeComponent))
			{
				WheelFX.TireSmokeComponent->Activate(true);
			}
		}
	}
	else
	{
		// Slip below threshold: deactivate smoke
		if (WheelFX.TireSmokeComponent)
		{
			DeactivateEffect(WheelFX.TireSmokeComponent);
			WheelFX.TireSmokeComponent = nullptr;
		}
	}
}

void UParticleSystemManager::UpdateWheelDriftSmoke(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh)
{
	if (!WheelEffects.IsValidIndex(WheelIndex))
	{
		return;
	}

	FWheelEffectState& WheelFX = WheelEffects[WheelIndex];
	const float AbsSlipAngle = FMath::Abs(TireState.SlipAngleDeg);
	const FSurfaceParticleParams& SurfParams = GetSurfaceParamsOrDefault(TireState.SurfaceType);

	// Drift smoke activates at high slip angles (lateral sliding)
	const bool bShouldDrift = AbsSlipAngle > DriftAngleThreshold && VehicleSpeedKmh > 15.0f;

	if (bShouldDrift)
	{
		const float DriftIntensity = FMath::Clamp(
			(AbsSlipAngle - DriftAngleThreshold) / (45.0f - DriftAngleThreshold), 0.0f, 1.0f);

		const float EmissionRate = DriftIntensity * ParticleConstants::DriftSmokeEmissionScale
			* SurfParams.EmissionMultiplier;

		if (!WheelFX.DriftSmokeComponent)
		{
			WheelFX.DriftSmokeComponent = GetOrCreateLoopingEffect(EParticleEffect::DriftSmoke, OwnerActor->GetRootComponent());
		}

		if (WheelFX.DriftSmokeComponent)
		{
			const FVector WheelLoc = GetWheelLocation(WheelIndex);
			WheelFX.DriftSmokeComponent->SetWorldLocation(WheelLoc);
			WheelFX.DriftSmokeComponent->SetWorldScale3D(FVector(SurfParams.ScaleMultiplier * 1.2f));
			SetEffectFloatParam(WheelFX.DriftSmokeComponent, TEXT("EmissionRate"), EmissionRate);
			SetEffectVectorParam(WheelFX.DriftSmokeComponent, TEXT("Color"),
				FVector(SurfParams.Color.R * 0.9f, SurfParams.Color.G * 0.9f, SurfParams.Color.B * 0.9f));

			if (!IsEffectActive(WheelFX.DriftSmokeComponent))
			{
				WheelFX.DriftSmokeComponent->Activate(true);
			}
		}
	}
	else
	{
		if (WheelFX.DriftSmokeComponent)
		{
			DeactivateEffect(WheelFX.DriftSmokeComponent);
			WheelFX.DriftSmokeComponent = nullptr;
		}
	}
}

void UParticleSystemManager::UpdateWheelTireMarks(int32 WheelIndex, const FTireState& TireState, float DeltaTime)
{
	if (!WheelEffects.IsValidIndex(WheelIndex))
	{
		return;
	}

	FWheelEffectState& WheelFX = WheelEffects[WheelIndex];
	const FSurfaceParticleParams& SurfParams = GetSurfaceParamsOrDefault(TireState.SurfaceType);

	// Only leave marks when grounded and slipping significantly
	const float AbsSlip = FMath::Max(FMath::Abs(TireState.SlipRatio), FMath::Abs(TireState.SlipAngleDeg) / 45.0f);
	const bool bShouldMark = TireState.bIsGrounded && AbsSlip > 0.3f && SurfParams.bProducesMarks;

	if (!bShouldMark)
	{
		return;
	}

	// Check spacing: only spawn decals at intervals
	const FVector WheelLoc = GetWheelLocation(WheelIndex);
	const float DistFromLast = FVector::Dist(WheelLoc, WheelFX.LastDecalLocation);

	if (DistFromLast < TireMarkSpacing && WheelFX.LastDecalLocation != FVector::ZeroVector)
	{
		return;
	}

	// Check decal config
	const FString MarkKey = TEXT("TireMark");
	const FDecalEffectConfig* DecalConfig = DecalConfigs.Find(MarkKey);
	if (!DecalConfig || !DecalConfig->DecalMaterial)
	{
		return;
	}

	// Scale decal size by slip intensity
	FDecalEffectConfig ScaledConfig = *DecalConfig;
	const float SlipScale = FMath::Lerp(0.6f, 1.5f, FMath::Clamp(AbsSlip, 0.0f, 1.0f));
	ScaledConfig.Size = DecalConfig->Size * SlipScale;

	// Place decal on ground surface
	const FRotator WheelRot = GetWheelRotation(WheelIndex);
	SpawnDecal(ScaledConfig, WheelLoc, FRotator(0.0f, WheelRot.Yaw, 0.0f));

	WheelFX.LastDecalLocation = WheelLoc;
	WheelFX.TimeSinceLastDecal = 0.0f;
}

void UParticleSystemManager::UpdateWheelDust(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh)
{
	if (!WheelEffects.IsValidIndex(WheelIndex))
	{
		return;
	}

	FWheelEffectState& WheelFX = WheelEffects[WheelIndex];
	const FSurfaceParticleParams& SurfParams = GetSurfaceParamsOrDefault(TireState.SurfaceType);

	// Dust only on loose surfaces at sufficient speed
	const bool bShouldDust = TireState.bIsGrounded
		&& SurfParams.bProducesDust
		&& VehicleSpeedKmh > DustSpeedThreshold;

	if (bShouldDust)
	{
		const float SpeedAlpha = FMath::Clamp(VehicleSpeedKmh / ParticleConstants::SpeedNormalization, 0.0f, 1.0f);
		const float AbsSlip = FMath::Max(FMath::Abs(TireState.SlipRatio), 0.1f);
		const float EmissionRate = SpeedAlpha * AbsSlip * ParticleConstants::DustEmissionScale
			* SurfParams.EmissionMultiplier;

		if (!WheelFX.DustComponent)
		{
			WheelFX.DustComponent = GetOrCreateLoopingEffect(EParticleEffect::DustCloud, OwnerActor->GetRootComponent());
		}

		if (WheelFX.DustComponent)
		{
			const FVector WheelLoc = GetWheelLocation(WheelIndex);
			WheelFX.DustComponent->SetWorldLocation(WheelLoc);
			WheelFX.DustComponent->SetWorldScale3D(FVector(SurfParams.ScaleMultiplier));
			SetEffectFloatParam(WheelFX.DustComponent, TEXT("EmissionRate"), EmissionRate);
			SetEffectVectorParam(WheelFX.DustComponent, TEXT("Color"),
				FVector(SurfParams.Color.R, SurfParams.Color.G, SurfParams.Color.B));

			if (!IsEffectActive(WheelFX.DustComponent))
			{
				WheelFX.DustComponent->Activate(true);
			}
		}
	}
	else
	{
		if (WheelFX.DustComponent)
		{
			DeactivateEffect(WheelFX.DustComponent);
			WheelFX.DustComponent = nullptr;
		}
	}
}

// ============================================================================
// Component Management
// ============================================================================
UFXSystemComponent* UParticleSystemManager::GetOrCreateLoopingEffect(EParticleEffect EffectType, USceneComponent* AttachTo)
{
	if (!OwnerActor || !AttachTo)
	{
		return nullptr;
	}

	const FParticleEffectConfig& Config = GetConfigOrDefault(EffectType);

	// Create Niagara component if available, otherwise Cascade
	if (Config.NiagaraSystem)
	{
		UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(OwnerActor);
		NiagaraComp->SetAsset(Config.NiagaraSystem);
		NiagaraComp->SetAutoActivate(Config.bAutoActivate);
		NiagaraComp->SetupAttachment(AttachTo);
		NiagaraComp->RegisterComponent();

		if (ActiveEffects.Num() < MaxActiveEffects)
		{
			ActiveEffects.Add(NiagaraComp);
		}

		return NiagaraComp;
	}

	if (Config.ParticleSystem)
	{
		UParticleSystemComponent* CascadeComp = NewObject<UParticleSystemComponent>(OwnerActor);
		CascadeComp->SetTemplate(Config.ParticleSystem);
		CascadeComp->bAutoActivate = Config.bAutoActivate;
		CascadeComp->SetupAttachment(AttachTo);
		CascadeComp->RegisterComponent();

		if (ActiveEffects.Num() < MaxActiveEffects)
		{
			ActiveEffects.Add(CascadeComp);
		}

		return CascadeComp;
	}

	return nullptr;
}

UFXSystemComponent* UParticleSystemManager::SpawnOneShotEffect(EParticleEffect EffectType, const FVector& Location, const FRotator& Rotation)
{
	if (!OwnerActor)
	{
		return nullptr;
	}

	const FParticleEffectConfig& Config = GetConfigOrDefault(EffectType);

	// Spawn Niagara system if available
	if (Config.NiagaraSystem)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Config.NiagaraSystem,
			Location,
			Rotation,
			Config.Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (NiagaraComp)
		{
			NiagaraComp->SetAutoDestroy(true);
			if (ActiveEffects.Num() < MaxActiveEffects)
			{
				ActiveEffects.Add(NiagaraComp);
			}
		}

		return NiagaraComp;
	}

	// Fall back to Cascade particle system
	if (Config.ParticleSystem)
	{
		UParticleSystemComponent* CascadeComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			Config.ParticleSystem,
			Location,
			Rotation,
			Config.Scale,
			true,
			EPSCPoolMethod::AutoRelease
		);

		if (CascadeComp)
		{
			CascadeComp->SetAutoDestroy(true);
			if (ActiveEffects.Num() < MaxActiveEffects)
			{
				ActiveEffects.Add(CascadeComp);
			}
		}

		return CascadeComp;
	}

	return nullptr;
}

void UParticleSystemManager::DeactivateEffect(UFXSystemComponent* Effect)
{
	if (!Effect)
	{
		return;
	}

	Effect->DeactivateSystem();

	// For Cascade: set auto-destroy. For Niagara: DeactivateSystem + auto-destroy is handled by pool.
	if (UParticleSystemComponent* CascadeComp = Cast<UParticleSystemComponent>(Effect))
	{
		CascadeComp->SetAutoDestroy(true);
	}
}

// ============================================================================
// Effect Parameter Helpers
// ============================================================================
void UParticleSystemManager::SetEffectFloatParam(UFXSystemComponent* Effect, const FName& ParamName, float Value)
{
	if (!Effect)
	{
		return;
	}

	if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Effect))
	{
		NiagaraComp->SetNiagaraVariableFloat(ParamName.ToString(), Value);
	}
	else if (UParticleSystemComponent* CascadeComp = Cast<UParticleSystemComponent>(Effect))
	{
		CascadeComp->SetFloatParameter(ParamName, Value);
	}
}

void UParticleSystemManager::SetEffectVectorParam(UFXSystemComponent* Effect, const FName& ParamName, const FVector& Value)
{
	if (!Effect)
	{
		return;
	}

	if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Effect))
	{
		NiagaraComp->SetNiagaraVariableVec3(ParamName.ToString(), Value);
	}
	else if (UParticleSystemComponent* CascadeComp = Cast<UParticleSystemComponent>(Effect))
	{
		CascadeComp->SetVectorParameter(ParamName, Value);
	}
}

bool UParticleSystemManager::IsEffectActive(UFXSystemComponent* Effect) const
{
	if (!Effect)
	{
		return false;
	}

	return Effect->IsActive();
}

// ============================================================================
// Cleanup
// ============================================================================
void UParticleSystemManager::CleanupEffects()
{
	// Remove null or finished effects from the active pool
	ActiveEffects.RemoveAll([](const TObjectPtr<UFXSystemComponent>& Effect)
	{
		if (!Effect || Effect->IsPendingKillPending())
		{
			return true;
		}

		// Check if Cascade effect with auto-destroy is no longer active
		if (const UParticleSystemComponent* CascadeComp = Cast<UParticleSystemComponent>(Effect))
		{
			return !CascadeComp->IsActive() && CascadeComp->bAutoDestroy;
		}

		return !Effect->IsActive();
	});
}

void UParticleSystemManager::CleanupDecals()
{
	// Remove expired decals
	ActiveDecals.RemoveAll([](const TObjectPtr<UDecalComponent>& Decal)
	{
		return !Decal || Decal->IsPendingKillPending();
	});
}

// ============================================================================
// Initialization
// ============================================================================
void UParticleSystemManager::InitializeDefaultConfigs()
{
	// Tire Smoke - looping, white/gray
	FParticleEffectConfig TireSmokeConfig;
	TireSmokeConfig.EmissionRate = 200.0f;
	TireSmokeConfig.Lifetime = 1.5f;
	TireSmokeConfig.Scale = FVector(1.0f);
	TireSmokeConfig.Color = FLinearColor(0.9f, 0.9f, 0.9f, 0.8f);
	TireSmokeConfig.bLoop = true;
	TireSmokeConfig.bAutoActivate = false;
	EffectConfigs.Add(EParticleEffect::TireSmoke, TireSmokeConfig);

	// Drift Smoke - looping, thicker than tire smoke
	FParticleEffectConfig DriftSmokeConfig;
	DriftSmokeConfig.EmissionRate = 300.0f;
	DriftSmokeConfig.Lifetime = 2.0f;
	DriftSmokeConfig.Scale = FVector(1.5f);
	DriftSmokeConfig.Color = FLinearColor(0.85f, 0.82f, 0.78f, 0.9f);
	DriftSmokeConfig.bLoop = true;
	DriftSmokeConfig.bAutoActivate = false;
	EffectConfigs.Add(EParticleEffect::DriftSmoke, DriftSmokeConfig);

	// Collision Sparks - one-shot, bright orange/yellow
	FParticleEffectConfig CollisionConfig;
	CollisionConfig.EmissionRate = 500.0f;
	CollisionConfig.Lifetime = 0.4f;
	CollisionConfig.Scale = FVector(0.5f);
	CollisionConfig.Color = FLinearColor(1.0f, 0.7f, 0.2f, 1.0f);
	CollisionConfig.bLoop = false;
	CollisionConfig.bAutoActivate = true;
	CollisionConfig.bEnableCollision = true;
	EffectConfigs.Add(EParticleEffect::CollisionSparks, CollisionConfig);

	// Dust Cloud - looping, brown/tan
	FParticleEffectConfig DustConfig;
	DustConfig.EmissionRate = 150.0f;
	DustConfig.Lifetime = 2.5f;
	DustConfig.Scale = FVector(2.0f);
	DustConfig.Color = FLinearColor(0.7f, 0.6f, 0.5f, 0.6f);
	DustConfig.bLoop = true;
	DustConfig.bAutoActivate = false;
	EffectConfigs.Add(EParticleEffect::DustCloud, DustConfig);

	// Rain Splash - looping, blue-white
	FParticleEffectConfig RainConfig;
	RainConfig.EmissionRate = 300.0f;
	RainConfig.Lifetime = 0.3f;
	RainConfig.Scale = FVector(0.5f);
	RainConfig.Color = FLinearColor(0.8f, 0.85f, 0.95f, 0.5f);
	RainConfig.bLoop = true;
	RainConfig.bAutoActivate = false;
	EffectConfigs.Add(EParticleEffect::RainSplash, RainConfig);

	// Tire Mark decal config
	FDecalEffectConfig TireMarkConfig;
	TireMarkConfig.Size = FVector(20.0f, 8.0f, 8.0f);
	TireMarkConfig.Lifetime = ParticleConstants::DefaultDecalLifetime;
	TireMarkConfig.FadeOutTime = ParticleConstants::DefaultDecalFadeOut;
	DecalConfigs.Add(TEXT("TireMark"), TireMarkConfig);
}

void UParticleSystemManager::InitializeDefaultSurfaceParams()
{
	// Tarmac (Dry) - dark gray smoke, produces marks, no dust
	FSurfaceParticleParams Tarmac;
	Tarmac.Color = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f);
	Tarmac.EmissionMultiplier = 1.0f;
	Tarmac.ScaleMultiplier = 1.0f;
	Tarmac.LifetimeMultiplier = 1.0f;
	Tarmac.bProducesMarks = true;
	Tarmac.bProducesDust = false;
	SurfaceParams.Add(ETireSurfaceType::Tarmac, Tarmac);

	// Tarmac (Wet) - lighter smoke, reduced marks, no dust
	FSurfaceParticleParams TarmacWet;
	TarmacWet.Color = FLinearColor(0.5f, 0.5f, 0.55f, 0.6f);
	TarmacWet.EmissionMultiplier = 0.7f;
	TarmacWet.ScaleMultiplier = 0.8f;
	TarmacWet.LifetimeMultiplier = 0.8f;
	TarmacWet.bProducesMarks = true;
	TarmacWet.bProducesDust = false;
	SurfaceParams.Add(ETireSurfaceType::TarmacWet, TarmacWet);

	// Gravel - brown particles, no marks, produces dust
	FSurfaceParticleParams Gravel;
	Gravel.Color = FLinearColor(0.55f, 0.45f, 0.35f, 0.9f);
	Gravel.EmissionMultiplier = 1.5f;
	Gravel.ScaleMultiplier = 1.3f;
	Gravel.LifetimeMultiplier = 1.5f;
	Gravel.bProducesMarks = false;
	Gravel.bProducesDust = true;
	SurfaceParams.Add(ETireSurfaceType::Gravel, Gravel);

	// Sand - tan particles, no marks, heavy dust
	FSurfaceParticleParams Sand;
	Sand.Color = FLinearColor(0.8f, 0.7f, 0.5f, 0.9f);
	Sand.EmissionMultiplier = 2.0f;
	Sand.ScaleMultiplier = 1.5f;
	Sand.LifetimeMultiplier = 2.0f;
	Sand.bProducesMarks = false;
	Sand.bProducesDust = true;
	SurfaceParams.Add(ETireSurfaceType::Sand, Sand);

	// Grass - green-brown, no marks, light dust
	FSurfaceParticleParams Grass;
	Grass.Color = FLinearColor(0.4f, 0.5f, 0.3f, 0.7f);
	Grass.EmissionMultiplier = 1.2f;
	Grass.ScaleMultiplier = 1.1f;
	Grass.LifetimeMultiplier = 1.3f;
	Grass.bProducesMarks = false;
	Grass.bProducesDust = true;
	SurfaceParams.Add(ETireSurfaceType::Grass, Grass);

	// Concrete - light gray, produces marks, no dust
	FSurfaceParticleParams Concrete;
	Concrete.Color = FLinearColor(0.5f, 0.5f, 0.5f, 0.7f);
	Concrete.EmissionMultiplier = 0.9f;
	Concrete.ScaleMultiplier = 0.9f;
	Concrete.LifetimeMultiplier = 0.9f;
	Concrete.bProducesMarks = true;
	Concrete.bProducesDust = false;
	SurfaceParams.Add(ETireSurfaceType::Concrete, Concrete);

	// Ice - white-blue, minimal marks, no dust
	FSurfaceParticleParams Ice;
	Ice.Color = FLinearColor(0.85f, 0.9f, 0.95f, 0.5f);
	Ice.EmissionMultiplier = 0.5f;
	Ice.ScaleMultiplier = 0.7f;
	Ice.LifetimeMultiplier = 0.5f;
	Ice.bProducesMarks = false;
	Ice.bProducesDust = false;
	SurfaceParams.Add(ETireSurfaceType::Ice, Ice);

	// Custom - defaults to tarmac
	FSurfaceParticleParams Custom;
	Custom.Color = FLinearColor(0.4f, 0.4f, 0.4f, 0.8f);
	Custom.EmissionMultiplier = 1.0f;
	Custom.ScaleMultiplier = 1.0f;
	Custom.LifetimeMultiplier = 1.0f;
	Custom.bProducesMarks = true;
	Custom.bProducesDust = false;
	SurfaceParams.Add(ETireSurfaceType::Custom, Custom);
}

void UParticleSystemManager::InitializeWheelEffects()
{
	const int32 NumWheels = GetNumWheels();
	WheelEffects.SetNum(NumWheels);

	for (int32 i = 0; i < NumWheels; i++)
	{
		WheelEffects[i].TireSmokeComponent = nullptr;
		WheelEffects[i].DriftSmokeComponent = nullptr;
		WheelEffects[i].DustComponent = nullptr;
		WheelEffects[i].LastDecalLocation = FVector::ZeroVector;
		WheelEffects[i].TimeSinceLastDecal = 0.0f;
		WheelEffects[i].SurfaceType = ETireSurfaceType::Tarmac;
		WheelEffects[i].bIsGrounded = true;
	}
}

// ============================================================================
// Helpers
// ============================================================================
const FParticleEffectConfig& UParticleSystemManager::GetConfigOrDefault(EParticleEffect EffectType) const
{
	const FParticleEffectConfig* Found = EffectConfigs.Find(EffectType);
	if (Found)
	{
		return *Found;
	}

	// Return a static default config
	static FParticleEffectConfig DefaultConfig;
	return DefaultConfig;
}

const FSurfaceParticleParams& UParticleSystemManager::GetSurfaceParamsOrDefault(ETireSurfaceType Surface) const
{
	const FSurfaceParticleParams* Found = SurfaceParams.Find(Surface);
	if (Found)
	{
		return *Found;
	}

	static FSurfaceParticleParams DefaultParams;
	return DefaultParams;
}

FVector UParticleSystemManager::GetWheelLocation(int32 WheelIndex) const
{
	if (!VehicleMovement || !OwnerActor)
	{
		return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
	}

	// Use vehicle movement component to get wheel world position
	if (WheelIndex >= 0 && WheelIndex < VehicleMovement->GetWheelCount())
	{
		const FWheelStatus& WheelStatus = VehicleMovement->GetWheelState(WheelIndex);
		return WheelStatus.WheelPosition;
	}

	return OwnerActor->GetActorLocation();
}

FRotator UParticleSystemManager::GetWheelRotation(int32 WheelIndex) const
{
	if (!OwnerActor)
	{
		return FRotator::ZeroRotator;
	}

	// Default to vehicle rotation (ground contact assumed flat)
	return OwnerActor->GetActorRotation();
}

int32 UParticleSystemManager::GetNumWheels() const
{
	if (VehicleMovement)
	{
		return VehicleMovement->GetWheelCount();
	}

	return ParticleConstants::DefaultNumWheels;
}
