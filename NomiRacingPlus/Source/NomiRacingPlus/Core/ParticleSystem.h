// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/DecalComponent.h"
#include "FXSystemComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Vehicles/TirePhysicsModel.h"
#include "ParticleSystem.generated.h"

// Forward declarations
class UChaosWheeledVehicleMovementComponent;

// Log category for particle system
DECLARE_LOG_CATEGORY_EXTERN(LogNomiParticles, Log, All);

// ============================================================================
// Particle effect types
// ============================================================================
UENUM(BlueprintType)
enum class EParticleEffect : uint8
{
	TireSmoke       UMETA(DisplayName = "Tire Smoke"),
	TireMarks       UMETA(DisplayName = "Tire Marks"),
	CollisionSparks UMETA(DisplayName = "Collision Sparks"),
	DustCloud       UMETA(DisplayName = "Dust Cloud"),
	RainSplash      UMETA(DisplayName = "Rain Splash"),
	DriftSmoke      UMETA(DisplayName = "Drift Smoke"),
	ExhaustFlame    UMETA(DisplayName = "Exhaust Flame"),
	BrakeGlow       UMETA(DisplayName = "Brake Glow"),
	BoostEffect     UMETA(DisplayName = "Boost Effect"),
	ParticleTrails  UMETA(DisplayName = "Particle Trails")
};

// ============================================================================
// Surface-specific particle parameters
// ============================================================================
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FSurfaceParticleParams
{
	GENERATED_BODY()

	// Particle color tint for this surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	FLinearColor Color = FLinearColor(0.8f, 0.75f, 0.7f, 1.0f);

	// Emission rate multiplier relative to base
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float EmissionMultiplier = 1.0f;

	// Particle scale multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ScaleMultiplier = 1.0f;

	// Lifetime multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float LifetimeMultiplier = 1.0f;

	// Does this surface produce visible marks (decals)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	bool bProducesMarks = true;

	// Does this surface produce dust clouds?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	bool bProducesDust = false;
};

// ============================================================================
// Particle effect configuration
// ============================================================================
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FParticleEffectConfig
{
	GENERATED_BODY()

	// Particle system asset (Niagara or Cascade)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	TObjectPtr<UParticleSystem> ParticleSystem;

	// Niagara system asset (preferred in UE5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	// Emission rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	float EmissionRate = 100.0f;

	// Lifetime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	float Lifetime = 1.0f;

	// Scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	FVector Scale = FVector(1.0f);

	// Color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	FLinearColor Color = FLinearColor::White;

	// Enable collision?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bEnableCollision = false;

	// Loop?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bLoop = false;

	// Auto-activate on spawn?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bAutoActivate = true;
};

// ============================================================================
// Decal effect configuration
// ============================================================================
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FDecalEffectConfig
{
	GENERATED_BODY()

	// Decal material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	// Size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	FVector Size = FVector(100.0f);

	// Lifetime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	float Lifetime = 5.0f;

	// Fade out time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	float FadeOutTime = 1.0f;
};

// ============================================================================
// Per-wheel effect state for tracking active effects per tire
// ============================================================================
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FWheelEffectState
{
	GENERATED_BODY()

	// Active tire smoke component (UFXSystemComponent base for Niagara/Cascade compatibility)
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	TObjectPtr<UFXSystemComponent> TireSmokeComponent;

	// Active drift smoke component
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	TObjectPtr<UFXSystemComponent> DriftSmokeComponent;

	// Active dust component
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	TObjectPtr<UFXSystemComponent> DustComponent;

	// Last decal spawn location (for spacing)
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	FVector LastDecalLocation = FVector::ZeroVector;

	// Time since last decal was spawned
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	float TimeSinceLastDecal = 0.0f;

	// Current wheel surface type
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	ETireSurfaceType SurfaceType = ETireSurfaceType::Tarmac;

	// Is the wheel currently on the ground?
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Effects")
	bool bIsGrounded = false;
};

// ============================================================================
// Rain state
// ============================================================================
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FRainState
{
	GENERATED_BODY()

	// Current rain intensity (0 = none, 1 = heavy)
	UPROPERTY(BlueprintReadOnly, Category = "Rain")
	float Intensity = 0.0f;

	// Target rain intensity (for interpolation)
	UPROPERTY(BlueprintReadOnly, Category = "Rain")
	float TargetIntensity = 0.0f;

	// Is it currently raining?
	UPROPERTY(BlueprintReadOnly, Category = "Rain")
	bool bIsRaining = false;

	// Active rain ambient component (screen rain)
	UPROPERTY(BlueprintReadOnly, Category = "Rain")
	TObjectPtr<UFXSystemComponent> RainAmbientComponent;

	// Active ground splash component
	UPROPERTY(BlueprintReadOnly, Category = "Rain")
	TObjectPtr<UFXSystemComponent> RainSplashComponent;
};

// ============================================================================
// Particle System Manager
// Manages all visual effects: tire smoke, collision sparks, drift marks, etc.
// ============================================================================
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UParticleSystemManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UParticleSystemManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === Effect Control ===

	// Spawn a one-shot particle effect at a location
	UFUNCTION(BlueprintCallable, Category = "Particles")
	UFXSystemComponent* SpawnEffect(EParticleEffect EffectType, const FVector& Location, const FRotator& Rotation);

	// Stop and deactivate a particle effect
	UFUNCTION(BlueprintCallable, Category = "Particles")
	void StopEffect(UFXSystemComponent* Effect);

	// Spawn a decal at a location
	UFUNCTION(BlueprintCallable, Category = "Particles")
	void SpawnDecal(const FDecalEffectConfig& Config, const FVector& Location, const FRotator& Rotation);

	// === Vehicle Effects (per-wheel) ===

	// Update tire smoke for all wheels based on vehicle state
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void UpdateTireEffects(float DeltaTime, const TArray<FTireState>& TireStates, float VehicleSpeedKmh);

	// Trigger collision sparks at impact point
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void TriggerCollisionSparks(const FVector& Location, const FVector& Normal, float ImpactForce);

	// === Environment Effects ===

	// Update rain effect (interpolates toward target intensity)
	UFUNCTION(BlueprintCallable, Category = "Particles|Environment")
	void UpdateRainEffect(float DeltaTime, float TargetIntensity);

	// Update dust based on speed and surface
	UFUNCTION(BlueprintCallable, Category = "Particles|Environment")
	void UpdateDustEffect(float Speed, bool bOnDirt);

	// === Configuration ===

	// Set particle effect config
	UFUNCTION(BlueprintCallable, Category = "Particles|Config")
	void SetEffectConfig(EParticleEffect EffectType, const FParticleEffectConfig& Config);

	// Get particle effect config
	UFUNCTION(BlueprintCallable, Category = "Particles|Config")
	const FParticleEffectConfig& GetEffectConfig(EParticleEffect EffectType) const;

	// Set surface-specific particle parameters
	UFUNCTION(BlueprintCallable, Category = "Particles|Config")
	void SetSurfaceParams(ETireSurfaceType Surface, const FSurfaceParticleParams& Params);

	// Set rain enabled/disabled
	UFUNCTION(BlueprintCallable, Category = "Particles|Config")
	void SetRainEnabled(bool bEnabled);

protected:
	// === Effect Configs ===

	// Particle effect configs per type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Config")
	TMap<EParticleEffect, FParticleEffectConfig> EffectConfigs;

	// Decal configs per type name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Config")
	TMap<FString, FDecalEffectConfig> DecalConfigs;

	// Surface-specific particle parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Config")
	TMap<ETireSurfaceType, FSurfaceParticleParams> SurfaceParams;

	// === Active Effects ===

	// Active particle effects pool (UFXSystemComponent base for Niagara/Cascade compatibility)
	UPROPERTY(BlueprintReadOnly, Category = "Particles|Active")
	TArray<TObjectPtr<UFXSystemComponent>> ActiveEffects;

	// Active decals
	UPROPERTY(BlueprintReadOnly, Category = "Particles|Active")
	TArray<TObjectPtr<UDecalComponent>> ActiveDecals;

	// Per-wheel effect states (index 0-3: FL, FR, RL, RR)
	UPROPERTY(BlueprintReadOnly, Category = "Particles|Vehicle")
	TArray<FWheelEffectState> WheelEffects;

	// Rain state
	UPROPERTY(BlueprintReadOnly, Category = "Particles|Environment")
	FRainState RainState;

	// === Limits ===

	// Maximum active particle effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Config")
	int32 MaxActiveEffects = 50;

	// Maximum active decals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Config")
	int32 MaxActiveDecals = 100;

	// === Thresholds ===

	// Minimum slip ratio to produce tire smoke
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TireSmokeSlipThreshold = 0.15f;

	// Minimum slip angle (degrees) to produce drift smoke
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float DriftAngleThreshold = 8.0f;

	// Minimum speed (km/h) for dust effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "0.0"))
	float DustSpeedThreshold = 10.0f;

	// Minimum speed (km/h) for rain ground splashes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "0.0"))
	float RainSplashSpeedThreshold = 5.0f;

	// Distance between tire mark decals (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "10.0"))
	float TireMarkSpacing = 30.0f;

	// Rain intensity interpolation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Thresholds", meta = (ClampMin = "0.1"))
	float RainInterpSpeed = 0.5f;

private:
	// === Initialization ===

	// Initialize default effect configs
	void InitializeDefaultConfigs();

	// Initialize default surface parameters
	void InitializeDefaultSurfaceParams();

	// Initialize per-wheel effect states
	void InitializeWheelEffects();

	// === Per-Wheel Updates ===

	// Update tire smoke for a single wheel
	void UpdateWheelTireSmoke(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh);

	// Update drift smoke for a single wheel
	void UpdateWheelDriftSmoke(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh);

	// Update tire marks (decals) for a single wheel
	void UpdateWheelTireMarks(int32 WheelIndex, const FTireState& TireState, float DeltaTime);

	// Update dust for a single wheel
	void UpdateWheelDust(int32 WheelIndex, const FTireState& TireState, float VehicleSpeedKmh);

	// === Component Management ===

	// Spawn or reuse a looping particle component attached to the owner
	UFXSystemComponent* GetOrCreateLoopingEffect(EParticleEffect EffectType, USceneComponent* AttachTo);

	// Spawn a one-shot particle component
	UFXSystemComponent* SpawnOneShotEffect(EParticleEffect EffectType, const FVector& Location, const FRotator& Rotation);

	// Deactivate and pool a particle component
	void DeactivateEffect(UFXSystemComponent* Effect);

	// Set a float parameter on either Niagara or Cascade component
	void SetEffectFloatParam(UFXSystemComponent* Effect, const FName& ParamName, float Value);

	// Set a vector parameter on either Niagara or Cascade component
	void SetEffectVectorParam(UFXSystemComponent* Effect, const FName& ParamName, const FVector& Value);

	// Check if an effect component is active
	bool IsEffectActive(UFXSystemComponent* Effect) const;

	// === Cleanup ===

	// Clean up finished particle effects from the active pool
	void CleanupEffects();

	// Clean up expired decals
	void CleanupDecals();

	// === Helpers ===

	// Get effect config or a default if not found
	const FParticleEffectConfig& GetConfigOrDefault(EParticleEffect EffectType) const;

	// Get surface particle params or default
	const FSurfaceParticleParams& GetSurfaceParamsOrDefault(ETireSurfaceType Surface) const;

	// Get the wheel world location from the owner vehicle
	FVector GetWheelLocation(int32 WheelIndex) const;

	// Get the wheel world rotation (ground contact normal)
	FRotator GetWheelRotation(int32 WheelIndex) const;

	// Get the number of wheels from the vehicle movement component
	int32 GetNumWheels() const;

	// Cached owner actor
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	// Cached vehicle movement component
	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

	// Pending collision sparks to spawn (queued to avoid mid-physics spawns)
	struct FPendingCollisionSparks
	{
		FVector Location;
		FVector Normal;
		float ImpactForce;
	};
	TArray<FPendingCollisionSparks> PendingCollisionSparks;
};
