// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NIOVehicleBase.h"
#include "NIO_ET5.generated.h"

/**
 * NIO ET5 - Compact Electric Sedan
 * - 360kW dual motor AWD
 * - 0-100 km/h: 4.0 seconds
 * - Top speed: 200 km/h
 * - Sporty handling with compact dimensions
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANIO_ET5 : public ANIOVehicleBase
{
	GENERATED_BODY()

public:
	ANIO_ET5(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
