// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NomiRacingParticleSystem.generated.h"

/**
 * Particle system manager for vehicle effects (tire smoke, drift smoke, sparks, etc.)
 * Niagara integration will be added when the Niagara module is available.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UNomiRacingParticleSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UNomiRacingParticleSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
