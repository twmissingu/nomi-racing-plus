// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NIOVehicleBase.h"
#include "Xiaomi_SU7Ultra.generated.h"

/**
 * Xiaomi SU7 Ultra - Super Sedan
 * - 1138kW (1548 HP) dual motor AWD
 * - 0-100 km/h: 1.98 seconds
 * - Top speed: 350 km/h
 * - Nürburgring record: 6:46.874
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API AXiaomi_SU7Ultra : public ANIOVehicleBase
{
	GENERATED_BODY()

public:
	AXiaomi_SU7Ultra(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
