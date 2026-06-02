// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/NIOVehicleBase.h"
#include "NIO_ET7.generated.h"

/**
 * NIO ET7 - Luxury Sedan
 * - 480kW dual motor AWD
 * - 0-100 km/h: 3.8 seconds
 * - Top speed: 250 km/h
 * - Premium comfort-focused suspension
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANIO_ET7 : public ANIOVehicleBase
{
	GENERATED_BODY()

public:
	ANIO_ET7();
};
