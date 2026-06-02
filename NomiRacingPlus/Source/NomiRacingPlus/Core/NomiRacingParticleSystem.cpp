// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiRacingParticleSystem.h"
#include "NomiRacingPlus.h"

UNomiRacingParticleSystem::UNomiRacingParticleSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UNomiRacingParticleSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UNomiRacingParticleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
