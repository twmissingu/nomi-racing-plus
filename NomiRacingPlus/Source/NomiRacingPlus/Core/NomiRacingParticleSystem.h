// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NomiRacingParticleSystem.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * Particle quality levels
 */
UENUM(BlueprintType)
enum class EParticleQuality : uint8
{
	Low       UMETA(DisplayName = "Low"),
	Medium    UMETA(DisplayName = "Medium"),
	High      UMETA(DisplayName = "High")
};

/**
 * Particle system manager for vehicle effects (tire smoke, drift smoke, collision sparks)
 * Uses Niagara for all particle rendering
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UNomiRacingParticleSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UNomiRacingParticleSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Spawn a burst of tire smoke at the given location (e.g. tire contact point)
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void SpawnTireSmoke(FVector Location, float Intensity);

	// Spawn a burst of sparks at the collision point oriented along the impact normal
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void SpawnCollisionSparks(FVector Location, FVector Normal, float Intensity);

	// Start or stop continuous drift smoke; Intensity scales particle count and size
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void UpdateDriftSmoke(bool bActive, float Intensity);

	// Spawn exhaust boost effect at exhaust location (triggered by high throttle at speed)
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void SpawnExhaustEffect(FVector Location, float Throttle, float SpeedKmh);

	// Start or stop continuous speed trail / airflow particles at high speed
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void UpdateSpeedTrail(bool bActive, float SpeedKmh);

	// Spawn water spray at tire contact point (triggered by wet surfaces)
	UFUNCTION(BlueprintCallable, Category = "Particles|Vehicle")
	void SpawnWaterSpray(FVector Location, float Intensity);

	// Adjust particle counts and sizes for the given quality level (0=Low, 1=Medium, 2=High)
	UFUNCTION(BlueprintCallable, Category = "Particles|Quality")
	void SetParticleQuality(int32 Level);

protected:
	// ------------------------------------------------------------------
	// Editable Niagara system assets (assign in Blueprint or editor)
	// ------------------------------------------------------------------

	// Niagara system used for single-frame tire smoke bursts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> TireSmokeSystemAsset;

	// Niagara system used for single-frame collision spark bursts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> CollisionSparksSystemAsset;

	// Niagara system used for continuous drift smoke
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> DriftSmokeSystemAsset;

	// Niagara system for exhaust boost effect (flame/heat burst at high throttle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> ExhaustEffectSystemAsset;

	// Niagara system for high-speed airflow / speed trail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> SpeedTrailSystemAsset;

	// Niagara system for water spray at tire contact points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles|Assets")
	TObjectPtr<UNiagaraSystem> WaterSpraySystemAsset;

	// ------------------------------------------------------------------
	// Managed Niagara components (created at runtime)
	// ------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles|Components")
	TObjectPtr<UNiagaraComponent> TireSmokeSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles|Components")
	TObjectPtr<UNiagaraComponent> CollisionSparksSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles|Components")
	TObjectPtr<UNiagaraComponent> DriftSmokeSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles|Components")
	TObjectPtr<UNiagaraComponent> SpeedTrailSystem;

	// ------------------------------------------------------------------
	// Quality settings
	// ------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles|Quality")
	EParticleQuality CurrentQuality;

private:
	// Cached owning actor root component (set once in BeginPlay)
	TWeakObjectPtr<USceneComponent> CachedRootComponent;

	// Create and register a Niagara component attached to the owner
	UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* System, const FName& ComponentName);

	// Apply quality scaling to a Niagara component's spawn count parameter
	void ApplyQualityToComponent(UNiagaraComponent* Component, float BaseSpawnCount);

	// Quality multipliers
	float GetQualitySpawnMultiplier() const;
	float GetQualitySizeMultiplier() const;
};
